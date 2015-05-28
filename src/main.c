#include <pebble.h>
#define SETTINGS_KEY 0

Window *s_my_window;
Layer *s_line_layer;
TextLayer *s_hour_layer, *s_minute_layer;
GFont s_time_font;
GColor Line;

#ifdef PBL_BW//Pebble B/W layer only
  InverterLayer *s_inv_layer;
#endif

typedef enum AppKey {
  APP_KEY_INVERT = 0,
  APP_KEY_BT
} AppKey;

typedef struct Settings {
  bool invert;
  bool bt;
} Settings;

//Bluetooth service handler
void bluetooth_handler (bool connected){ 
  //Checks to see if bluetooth is connected and changes the color to either blue or green(Pebble Time ONLY)
    if(connected) {
      #ifdef PBL_COLOR
        text_layer_set_text_color(s_minute_layer , GColorSpringBud);
        text_layer_set_text_color(s_hour_layer , GColorGreen);
      #endif
      
      vibes_long_pulse();
  } 
  else{
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_hour_layer, GColorVividCerulean);
      text_layer_set_text_color(s_minute_layer, GColorPictonBlue);
    #endif
    
    vibes_double_pulse();
  }
}

//Draws the canvas to which everything should be drawn, or painted, depending if you ar an artist or not
void s_layer_update_proc(Layer *l, GContext *ctx){
  Line = GColorWhite;
    
  graphics_context_set_fill_color(ctx, Line);
  graphics_fill_rect(ctx, GRect(0, 79, 144, 1), 0, GCornerNone);
}
  
//Updates Time and applies it to a text layer from the buffer
//Pun intended in function name?
void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_hour_buffer[16];
  static char s_minute_buffer[16];
  if(clock_is_24h_style() == true){
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%H", tick_time); 
  } else {
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
  }
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_minute_layer, s_minute_buffer);

}

//Updates update_time function
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
 
//Initlaties the window
void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  //Window
  window_set_background_color(s_my_window, GColorBlack);
  
  //Time handling
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Bluetooth handling
  bluetooth_connection_service_subscribe(bluetooth_handler);
  
  //Load font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LIGHT_72));
  
  //Layers
  
  //Minutes
  s_minute_layer = text_layer_create(GRect(15, 56, 144, 80));
  text_layer_set_text(s_minute_layer, "");
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_font(s_minute_layer, s_time_font);
  text_layer_set_background_color(s_minute_layer, GColorClear);
  layer_add_child(window_get_root_layer(s_my_window), text_layer_get_layer(s_minute_layer));
  
  //Hours
  s_hour_layer = text_layer_create(GRect(-23, 19, 154, 61));
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_text(s_hour_layer, "");
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_hour_layer, GColorBlack);
  layer_add_child(window_get_root_layer(s_my_window), text_layer_get_layer(s_hour_layer));
  
  //Line
  s_line_layer = layer_create(GRect(0 , 0, 144, 168));
  layer_set_update_proc(s_line_layer, s_layer_update_proc);
  layer_add_child(window_get_root_layer(s_my_window), s_line_layer);
  
  //Inverter Layer, only for pebble OG
  #ifdef PBL_BW
    s_inv_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  #endif
    
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_minute_layer , GColorSpringBud);
    text_layer_set_text_color(s_hour_layer , GColorGreen);
  
  #else
    text_layer_set_text_color(s_minute_layer , GColorWhite); //Sets minutes white on OG Pebble
    text_layer_set_text_color(s_hour_layer , GColorWhite); //Set hours white on OG Pebble
  #endif
   
   //Option-specific setup
  #ifdef PBL_BW
    //! will flip the boolean to its opposite value, effectively doing what you want with less code
    layer_set_hidden(inverter_layer_get_layer(s_inv_layer), !settings.invert);
    layer_add_child(window_layer, inverter_layer_get_layer(s_inv_layer));
  #endif
}


// https://youtu.be/3ceSvpj0Tbk?t=18s
void main_window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  layer_destroy(s_line_layer);
  //Only applies to OG Pebble because inverter layer only applies to OG pebble.
  #ifdef PBL_BW
  inverter_layer_destroy(s_inv_layer);
  #endif
}

//Saves and updates settings
void save_settings(){
    layer_set_hidden(inverter_layer_get_layer(s_inv_layer), settings.invert);
    if(settings.bt || settings.invert){
      vibes_long_pulse();
    }
    else{
      vibes_short_pulse();
    }
    
    int result = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    APP_LOG(APP_LOG_LEVEL_INFO, "%d bytes written to settings.", result);
    //If it's below 0, there's been an error. You can do something with this info if you'd like.
    if(result < 0){
      //...
    }
}

//Settings
#ifdef PBL_BW
void in_recv_handler(DictionaryIterator *iterator, void *context){
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t){
    switch(t->key){
    case KEY_INVERT:
      settings.invert = !(strcmp(t->value->cstring, "on") == 0);
      break;   
     
     case KEY_BT:
      settings.bt = (strcmp(t->value->cstring, "on") == 0);
      break;
    }
    save_settings();
  }
}
#endif
static void init() {
  // Create main Window element and assign to pointer
  s_my_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_my_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //Load settings
  int result = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_INFO, "Read %d bytes from settings.", result);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_my_window, false);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  //Receives the messages from javascript.
  #ifdef PBL_BW
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  #endif
}

static void deinit() {
  // Destroy Window
  window_destroy(s_my_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
