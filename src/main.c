#include <pebble.h>
  
Window *s_my_window;
static char s_hour_buffer[8];
static char s_minute_buffer[8];
Layer *s_line_layer;
TextLayer *s_hour_layer, *s_minute_layer,*batt_text;
GFont s_time_font;
GColor Line;

#ifdef PBL_BW
InverterLayer *s_inv_layer;
#endif
#define KEY_INVERT 0
  

//Updates Time and applies it to a text layer from the buffer
static void update_time () {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  if(clock_is_24h_style() == true){
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%H", tick_time); 
  } else {
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
  }
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_minute_layer, s_minute_buffer);

}

static void bluetooth_handler (bool connected){

  
    if(connected) {
    #ifdef PBL_COLOR
    text_layer_set_text_color(s_minute_layer , GColorSpringBud);
    text_layer_set_text_color(s_hour_layer , GColorGreen);
    #else
    vibes_short_pulse();
    #endif
      
  } else{
    
    #ifdef PBL_COLOR
    text_layer_set_text_color(s_hour_layer, GColorVividCerulean);
    text_layer_set_text_color(s_minute_layer, GColorPictonBlue);
    
    #else
    vibes_double_pulse();
    
    #endif
  }
  

}

static void s_layer_update_proc(Layer *l, GContext *ctx){
 
Line = GColorWhite;
graphics_context_set_fill_color(ctx, Line);
graphics_fill_rect(ctx, GRect(0, 79, 144, 2), 0, GCornerNone);
  
}
  


//Updates update_time function
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

}

 #ifdef PBL_PLATFORM_BASALT
  static void handle_battery(BatteryChargeState charge_state) {
    switch(charge_state.charge_percent){
    case 100  :
       text_layer_set_text_color(s_hour_layer, GColorGreen);
       text_layer_set_text_color(s_minute_layer, GColorGreen);
    case 90  :
       text_layer_set_text_color(s_hour_layer, GColorSpringBud);
       text_layer_set_text_color(s_minute_layer, GColorGreen);
       break; 
    case 80  :
       text_layer_set_text_color(s_hour_layer, GColorSpringBud);
       text_layer_set_text_color(s_minute_layer, GColorSpringBud);
       break
    case 70  :
       text_layer_set_text_color(s_hour_layer, GColorIcterine);
       text_layer_set_text_color(s_minute_layer, GColorSpringBud);
       break; 
    case 60  :
       text_layer_set_text_color(s_hour_layer, GColorIcterine);
       text_layer_set_text_color(s_minute_layer, GColorIcterine);
       break;
    case 50  :
       text_layer_set_text_color(s_hour_layer, GColorRajah);
       text_layer_set_text_color(s_minute_layer, GColorIcterine);
       break; 
    case 40  :
       text_layer_set_text_color(s_hour_layer, GColorRajah);
       text_layer_set_text_color(s_minute_layer, GColorRajah);
       break; 
    case 30  :
       text_layer_set_text_color(s_hour_layer, GColorRed);
       text_layer_set_text_color(s_minute_layer, GColorRajah);
       break; 
    case 20  :
       text_layer_set_text_color(s_hour_layer, GColorRed);
       text_layer_set_text_color(s_minute_layer, GColorRed);
       break;
    case 10  :
       text_layer_set_text_color(s_hour_layer, GColorFolly);
       text_layer_set_text_color(s_minute_layer, GColorFolly);
       break; /* optional */
    default : 
       text_layer_set_text_color(s_hour_layer, GColorGreen);
       text_layer_set_text_color(s_minute_layer, GColorGreen);
  }
}
#endif
 
//Initlaties the window
static void main_window_load(Window *window) {
  
  #ifdef PBL_BW
  bool inverted = persist_read_bool(KEY_INVERT);
  #endif
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

  

    text_layer_set_text_color(s_minute_layer , GColorWhite);//Sets minutes white on OG Pebble
    text_layer_set_text_color(s_hour_layer , GColorWhite);//Set hours white on OG Pebble

   
   //Option-specific setup
     #ifdef PBL_BW
      
  if(inverted == true)
  {
    layer_set_hidden(inverter_layer_get_layer(s_inv_layer), false);
  }
  else
  {
    layer_set_hidden(inverter_layer_get_layer(s_inv_layer), true);

    
  }
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_inv_layer));
  #endif

  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  
  tick_handler(t, MINUTE_UNIT);
  
  
 }



static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(batt_text);
  layer_destroy(s_line_layer);
  #ifdef PBL_BW 
  inverter_layer_destroy(s_inv_layer);
  #endif
}

#ifdef PBL_BW
static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t)
  {
    switch(t->key)
    {
    case KEY_INVERT:
      //It's the KEY_INVERT key
      if(strcmp(t->value->cstring, "on") == 0)
      {
        //Set and save as inverted
        layer_set_hidden(inverter_layer_get_layer(s_inv_layer), false);
        vibes_short_pulse();
 
        persist_write_bool(KEY_INVERT, true);
      }
      else if(strcmp(t->value->cstring, "off") == 0)
      {
        //Set and save as not inverted
          layer_set_hidden(inverter_layer_get_layer(s_inv_layer), true);
          vibes_short_pulse();
        
 
        persist_write_bool(KEY_INVERT, false);
      }
    }
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

  window_stack_push(s_my_window, false);
  
  #ifdef PBL_COLOR
   battery_state_service_subscribe(handle_battery);
  #endif

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
