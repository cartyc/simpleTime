#include <pebble.h>

//Var for the main watchface
static Window *s_main_window;


static GFont *s_time_font;
//for for the main text
static TextLayer *s_time_layer;

static void update_time(){
  //Get a tm Structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  //Create Long lived buffer
  static char buffer[] = "00:00";
  
  //write current hours/minutes
  if(clock_is_24h_style() == true){
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else{
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  text_layer_set_text(s_time_layer, buffer);
}

//Time Updated
static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}
//Load the window
static void main_window_load(Window *window){
  //Create time TextLayer
  s_time_layer = text_layer_create(GRect(0,5,144,50));
  
  text_layer_set_background_color(s_time_layer, GColorClear);  
  text_layer_set_text_color(s_time_layer, GColorBlack);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LATO_BOLD_42));

  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_time_layer));
  
  update_time();
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
}

//This Initializes the watch face
//Everything here has to have an deinit in the deinit()
static void init(){
//Create main Window element
s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  //Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
}

static void deinit(){
  //Close out all
  window_destroy(s_main_window);
}
  
//Run the watchfacee
int main(void){
  //initialize watch face
  init();
  //Run the loop
  app_event_loop();
  //Close out when lop is exited
  deinit();
}