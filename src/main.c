#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

//Var for the main watchface
static Window *s_main_window;

//for for the main text
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;

//Font Elements for layers
static GFont s_time_font;
static GFont s_weather_font;

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


//Load the window
static void main_window_load(Window *window){
  
  //Create time TextLayer
  s_time_layer = text_layer_create(GRect(5,52,139,50));
  text_layer_set_background_color(s_time_layer, GColorClear);  
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LATO_BOLD_42));
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  
  //Add Layer
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_time_layer));

  //Create Temp Layer
  s_weather_layer = text_layer_create(GRect(0,130,144,25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "`");
  
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LATO_REGULAR_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  update_time();
}

static void main_window_unload(Window *window){
  
    //Destory Text Layer and font
    fonts_unload_custom_font(s_time_font);
    text_layer_destroy(s_time_layer);
  
    //Destroy Weather font and layer
    text_layer_destroy(s_weather_layer);
    fonts_unload_custom_font(s_weather_font);
}

//Time Updated
static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
  
  if (tick_time -> tm_min % 30 == 0){
    //Begin Dict
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    //Add key values
    dict_write_uint8(iter, 0, 0);
    
    //Send the message
    app_message_outbox_send();
  }
}

//App communication
static void inbox_received_callback(DictionaryIterator *iterator, void *context){
  
  //Store incoming data
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  Tuple *t = dict_read_first(iterator);
  
  //For all items
  while(t != NULL){
    switch(t->key){
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
          break;
    }
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback( AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send Success!");
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
  
    //Register Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
    //open App Message
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
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