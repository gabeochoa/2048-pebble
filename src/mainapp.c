#include <pebble.h>
#include <time.h>

static Window *window;
static TextLayer *text_layer;
Layer *number_layer;
Layer *grid_layer;

#define DEBUG 1
//Global Variables
static uint16_t grid[4][4];
static struct tm *curtime;

typedef struct point
{
    uint8_t a;
    uint8_t b;
}point;

//Declarations
static void click_config_provider(void *context);
//int random(int max);
point get_empty_spot(int dir);
bool combine_blocks(int lastdir);
bool check_cmb(int i,int j,int lastdir);
void draw_grid(Layer *me, GContext* ctx);
void mvblocks(int dir);
bool mv_up();
bool mv_left();
bool mv_right();
bool mv_down();
void spawn_block(int dir);
bool get_empty();

void draw_grid(Layer *layer, GContext* ctx)
{
	(void)layer;
	
	GRect bound = layer_get_bounds(layer);
	int spx = bound.size.w/4;
	int spy = bound.size.h/4;
 	graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bound, 5, GCornerNone);
  for(int i=0; i<4; i++)
	for(int j=0; j<4; j++)
	{	
 		graphics_context_set_fill_color(ctx, GColorWhite);
		GRect rec = ((GRect)
								{
								.origin = {5+i*spx,5+j*spy},
								.size = {spx-10,spy-10}
								});
		graphics_fill_rect(ctx, rec, 10, GCornerNone);
		
		//Drawing numbers for grid.
		if(grid[i][j] == 0)
			continue;
		char str[4];
		snprintf(str, 4, "%d", grid[i][j]);
		graphics_context_set_text_color(ctx, GColorBlack);
		GFont* font;
		font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
		if(grid[i][j] > 99)
		font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
		if(grid[i][j] > 999)
		font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		
		graphics_draw_text(ctx, 
											 str, 
											 font, 
											 rec, 
											 GTextOverflowModeFill, 
											 GTextAlignmentCenter, 
											 NULL);
	}
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  grid_layer = layer_create((GRect){ .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } }); 
	layer_set_update_proc(grid_layer, draw_grid);
	layer_add_child(window_layer, grid_layer);
	
	#if DEBUG
	text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(grid_layer,text_layer_get_layer(text_layer));
	#endif
}

static void window_unload(Window *window) {
  #if DEBUG
	text_layer_destroy(text_layer);
	#endif
	layer_destroy(grid_layer);
	layer_destroy(number_layer);
}
static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  
  for(int i=0; i<4; i++)
  for(int j=0; j<4; j++)
    grid[i][j] = 0;
  
  int s1a = rand()%3;
  int s1b = rand()%3;
  int s2a = rand()%3;
  int s2b = rand()%3;
  int r1 = rand()%4;
  int r2 = rand()%4;
  grid[s1a][s1b] = ((r1&0x1)==0x1)? r1 : 2;
  grid[s2a][s2b] = ((r2&0x1)==0x1)? r2 : 4;
  
  window_stack_push(window, animated);
}
static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Move up");
	mvblocks(0);
	
	//#if DEBUG
	text_layer_set_text(text_layer, "Up");
	//#endif
}
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Move right");
	mvblocks(1);
	//#if DEBUG
	text_layer_set_text(text_layer, "Select");
	//#endif
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Move down");
	mvblocks(2);
	//#if DEBUG
	text_layer_set_text(text_layer, "Down");
	//#endif
}
static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Move left");
	mvblocks(3);
	//#if DEBUG
	text_layer_set_text(text_layer, "Back");
	//#endif
}
static void long_back_click(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "LongBackDown");
}
static void long_back_click_release(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "LongBackUp");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_long_click_subscribe(BUTTON_ID_BACK, 700,long_back_click,long_back_click_release);
}
/*
int random(int max)
{
	time_t ct = time(NULL);
  curtime = localtime(&ct);
	long seed = (curtime->tm_min)*(curtime->tm_mon)*(curtime->tm_hour)*(curtime->tm_sec);
	seed = (((seed * 214013L + 2531011L) >> 16) & 32767);
	return ((seed % max) + 1);
}
*/

bool mv_up()
{
	for(int a = 0; a<10; a++)
	for(int i=0; i<4; i++)
  for(int j=1; j<4; j++)
    if(grid[i][j-1] == 0)
    {
      grid[i][j-1] = grid[i][j]; 
			grid[i][j] = 0;
    }
	return combine_blocks(0);
}
bool mv_left()
{
	for(int a = 0; a<10; a++)
  for(int j=0; j<4; j++)
	for(int i=1; i<4; i++)
    if(grid[i-1][j] == 0)
    {
      grid[i-1][j] = grid[i][j]; 
			grid[i][j] = 0;
    }
	return combine_blocks(3);
}
bool mv_right()
{
	bool ret = false;
  for(int a = 0; a<10; a++)
	for(int j=3; j>=0; j--)
	for(int i=0; i<3; i++)
	{
		if(grid[i+1][j] == 0)
    {
      grid[i+1][j] = grid[i][j]; 
			grid[i][j] = 0;
    }
		else if(check_cmb(i,j,1))
    {
    	grid[i+1][j] *= 2;
      grid[i][j] = 0; 
			ret = true;
    }
	}
	return ret;
}

bool mv_down()
{
	//bool ret = false;
	for(int a = 0; a<10; a++)
  for(int i=0; i<4; i++)
	for(int j=0; j<3; j++)
		if(grid[i][j+1] == 0)
    {
      grid[i][j+1] = grid[i][j]; 
			grid[i][j] = 0;
    }
	  else if(check_cmb(i,j,2))
    {
    	grid[i][j+1] *= 2;
      grid[i][j] = 0; 
		//	ret = true;
    } 
	return true;
}

void mvblocks(int dir)
{
	bool ret = false;
	switch(dir)
	{
		case 0://up:
					ret = ret || mv_up();
    break;
    case 1://right:
					ret = ret || mv_right();
    break;
    case 2://down:
					ret = ret || mv_down();
    break;
    case 3://left:
					ret = ret || mv_left();
    break;
	}
	
	if(ret && get_empty())
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Spawn");
		spawn_block(dir);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Finish Spawn");
	}
}

bool get_empty()
{
	for(int i=0; i<4; i++)
	for(int j=0; j<4; j++)
		if(grid[i][j] == 0)
		return true;
	return false;
}
void spawn_block(int dir)
{
	if(!get_empty())
		return;
	
	point p;
	p = get_empty_spot(dir);
	grid[p.a][p.b] = 2;
}
point get_empty_spot(int dir)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "GetEmptySpot");
    point p;
    int x=0,y=0;
		int r = rand()%4;
	
	switch(dir)
	{
		case 0://up:
					do{
							x = r; y=3;
							r = rand()%4;
					}while(grid[x][y]!=0);
					/*
					if(grid[0][3] == 0){x = 0; y = 3;}
					else if(grid[1][3] == 0){x = 1; y = 3;}
					else if(grid[2][3] == 0){x = 2; y = 3;}
					else if(grid[3][3] == 0){x = 3; y = 3;}	
					*/
    break;
    case 1://right:
					do{
							x = 0; y=r;
							r = rand()%4;
					}while(grid[x][y]!=0);
    break;
    case 2://down:
					do{
							x = r; y=0;
							r = rand()%4;
					}while(grid[x][y]!=0);
    break;
    case 3://left:
					do{
							x = 3; y=r;
							r = rand()%4;
					}while(grid[x][y]!=0);
    break;
	}
    p.a = x;
    p.b = y;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished GetEmptySpot");
    return p;
}

bool combine_blocks(int lastdir)
{
	bool ret = false;
  switch(lastdir)
  {
    case 0://up:
          for(int i=1; i<4; i++)
          for(int j=0; j<4; j++)
            if(check_cmb(i,j,lastdir))
            {
                grid[i][j-1] *= 2;
                grid[i][j] = 0; 
								ret = true;
            } 
    break;
    case 1://right:
          for(int i=0; i<3; i++)
          for(int j=0; j<4; j++)
            if(check_cmb(i,j,lastdir))
            {
                grid[i+1][j] *= 2;
                grid[i][j] = 0;  
								ret = true;
            } 
    break;
    case 2://down:
          for(int i=0; i<4; i++)
          for(int j=0; j<3; j++)
            if(check_cmb(i,j,lastdir))
            {
                grid[i][j+1] *= 2;
                grid[i][j] = 0; 
								ret = true;
            } 
    break;
    case 3://left:
          for(int i=1; i<4; i++)
          for(int j=0; j<4; j++)
            if(check_cmb(i,j,lastdir))
            {
                grid[i-1][j] *= 2;
                grid[i][j] = 0;
								ret = true;
            } 
    break;
  }
	return ret;
}

bool check_cmb(int i,int j,int lastdir)
{
	if(grid[i][j] == 0)
	return false;
	
   switch(lastdir)
  {
    case 0://up:
    if(j>0 && grid[i][j-1] == grid[i][j])
      return true;
    break;
    case 1://right:
    if(i<3 && grid[i+1][j] == grid[i][j])
      return true;
    break;
    case 2://down:
    if(j<3 && grid[i][j+1] == grid[i][j])
      return true;
    break;
    case 3://left:
    if(i>0 && grid[i-1][j] == grid[i][j])
      return true;
    break;
   }
  return false;
}
