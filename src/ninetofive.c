//Dat nine to five progress bar watch face
//Ryan Scott
//Copyright 2013
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x1F, 0x04, 0x9A, 0x54, 0xE2, 0x19, 0x44, 0xEA, 0x82, 0x2A, 0x98, 0x1D, 0x98, 0xB8, 0xAF, 0xF4 }
PBL_APP_INFO(MY_UUID,
             "NineToFive", "Ryan Scott",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define INT_DIGITS 11
Window window;
TextLayer text_time_layer;
TextLayer text_percent_layer;
TextLayer text_date_layer;
Layer progress_bar_border_layer;
Layer progress_bar_fill_layer;
float percent_fill = 0.0;
int start_time = 8;
int end_time = 17;
static char* percent_text;

char *itoa(i)
     int i;
{
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  return p;
}

void draw_progress_bar_border(Layer *layer, GContext *ctx) {
  uint8_t rad = 4; 
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(10, 0, 144-20, 15), rad, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(12, 2, 144-24, 11), rad, GCornersAll);
}

void draw_progress_bar_fill(Layer *layer, GContext *ctx) {
  uint8_t rad = 4; 
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer->bounds, rad, GCornersLeft);
}

void calculate_percent_complete(PblTm atime) {
  if (atime.tm_hour < 8)
  {
    percent_fill = 0.0;
  }
  else if (atime.tm_hour >= 8 && atime.tm_hour < 17)
  {
    percent_fill = (atime.tm_hour - 8.0) / 9.0;
    percent_fill += (atime.tm_min / 60.0) * (1.0/9.0);
  }
  else if (atime.tm_hour >= 17)
  {
    percent_fill = 1.0;
  }
  else {
    percent_fill = 1.0;
  }
  int percent_to_draw = percent_fill * 100;
  percent_text[0] = '\0';
  percent_text[1] = '\0';
  percent_text[2] = '\0';
  percent_text[3] = '\0';
  percent_text[4] = '\0';
  percent_text = itoa(percent_to_draw);
  text_layer_set_text(&text_percent_layer, strncat(percent_text, "%", 1));
  int16_t pixel_fill = percent_fill * 120;
  layer_set_frame(&progress_bar_fill_layer, GRect (12, 32, pixel_fill, 11));
  layer_mark_dirty(&progress_bar_fill_layer);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)t;
  (void)ctx;

  static char timeText[] = "00:00"; // Needs to be static because it's used by the system later.
  static char dateText[] = "Xxxxxxxxx 00";

  PblTm currentTime;

  get_time(&currentTime);
  string_format_time(timeText, sizeof(timeText), "%R", &currentTime);
  text_layer_set_text(&text_time_layer, timeText);

  string_format_time(dateText, sizeof(dateText), "%b %e", &currentTime);
  text_layer_set_text(&text_date_layer, dateText);


  calculate_percent_complete(currentTime);
}


void handle_init(AppContextRef ctx) {

  window_init(&window, "NineToFive");
  window_stack_push(&window, true /* Animated */);

  window_set_background_color(&window, GColorBlack);
  text_layer_init(&text_time_layer, GRect(0, 70, 144, 168-70));
  text_layer_set_text_color(&text_time_layer, GColorWhite);
  text_layer_set_background_color(&text_time_layer, GColorClear);
  text_layer_set_font(&text_time_layer, fonts_get_system_font(FONT_KEY_GOTHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(&text_time_layer, GTextAlignmentCenter);

  text_layer_init(&text_percent_layer, GRect(0, 2, 144, 50));
  text_layer_set_text_color(&text_percent_layer, GColorWhite);
  text_layer_set_background_color(&text_percent_layer, GColorClear);
  text_layer_set_font(&text_percent_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(&text_percent_layer, GTextAlignmentCenter);

  text_layer_init(&text_date_layer, GRect(0, 115, 144, 58));
  text_layer_set_text_color(&text_date_layer, GColorWhite);
  text_layer_set_background_color(&text_date_layer, GColorClear);
  text_layer_set_font(&text_date_layer, fonts_get_system_font(FONT_KEY_GOTHAM_42_LIGHT));
  text_layer_set_text_alignment(&text_date_layer, GTextAlignmentCenter);


  //make the progress bar border layer
  layer_init(&progress_bar_border_layer, GRect(0, 30, 144, 15));
  progress_bar_border_layer.update_proc = &draw_progress_bar_border;
  layer_init(&progress_bar_fill_layer, GRect(12, 32, 120, 11));
  progress_bar_fill_layer.update_proc = &draw_progress_bar_fill;



  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  handle_minute_tick(ctx, NULL);

  layer_add_child(&window.layer, &text_time_layer.layer);
  layer_add_child(&window.layer, &text_percent_layer.layer);
  layer_add_child(&window.layer, &text_date_layer.layer);
  layer_add_child(&window.layer, &progress_bar_border_layer);
  layer_add_child(&window.layer, &progress_bar_fill_layer);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
