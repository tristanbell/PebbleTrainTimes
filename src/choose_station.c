#include <pebble.h>
#include "choose_station.h"
#include "quick_journey.h"
#include "keys.h"	
	
#define NUM_MENU_SECTIONS 1

#define NUM_STATIONS 10

static struct {
    Window *gs_window;
    MenuLayer *menu_layer;
	TextLayer *text_layer;
	bool initialized;
} ui;

char station_list[NUM_STATIONS][35];
static uint8_t current_station = 0;

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
	  return current_station;
    default:
      return 0;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	if (cell_index->section == 0) {
		menu_cell_basic_draw(ctx, cell_layer, station_list[cell_index->row], NULL, NULL);
    }
}

static void set_station_distance(int distance) {
	Tuplet distance_tuple = TupletInteger(GET_STATIONS_KEY_SET_DISTANCE, distance);

  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);

	if (iter == NULL) {
    	return;
    }

  	dict_write_tuplet(iter, &distance_tuple);
  	dict_write_end(iter);

  	app_message_outbox_send();
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	int distance = cell_index->row;
	set_station_distance(distance);
	
	// Deregister callbacks so app messages are sent to quick_journey instead
	//app_message_deregister_callbacks();
}

static void fetch_msg(void) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "fetching first message...");
	text_layer_set_text(ui.text_layer, "Loading...");
	
  	Tuplet fetch_tuple = TupletInteger(GET_STATIONS_KEY_FETCH, 1);

  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);

	if (iter == NULL) {
    	return;
    }

  	dict_write_tuplet(iter, &fetch_tuple);
  	dict_write_end(iter);

  	app_message_outbox_send();
}
	
static void out_sent_handler(DictionaryIterator *sent, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sent.");
	
	Tuple *tuple = dict_find(sent, GET_STATIONS_KEY_SET_DISTANCE);
	if (tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures...");
		quick_journey_show();
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sending failed.");
	
	text_layer_set_text(ui.text_layer, "Error: initialization failed.");
	layer_set_hidden(text_layer_get_layer(ui.text_layer), false);
}


static void in_received_handler(DictionaryIterator *received, void *context)
{
	// Perhaps move this bit?
	if (!ui.initialized) {
		ui.initialized = true;
		layer_set_hidden(text_layer_get_layer(ui.text_layer), true);
	}
	
	Tuple *station = dict_find(received, GET_STATIONS_KEY_STATION);

  	if (station) {
		if (current_station < NUM_STATIONS) {
			strncpy(station_list[current_station++], station->value->cstring, sizeof(station_list[0]));
		}
	}
	
    menu_layer_reload_data(ui.menu_layer);
	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
}


static void in_dropped_handler(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message dropped.");
}

static void app_message_init(void)
{	
	app_message_register_inbox_received(in_received_handler);
   	app_message_register_inbox_dropped(in_dropped_handler);
   	app_message_register_outbox_sent(out_sent_handler);
  	app_message_register_outbox_failed(out_failed_handler);
	
	fetch_msg();
}

static void window_load(Window *window)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WINDOW LOAD!");
	
     //   departures = departure_list_create(num_departures);
	Layer *window_layer = window_get_root_layer(window);
 	GRect bounds = layer_get_frame(window_layer);
	
	ui.menu_layer = menu_layer_create(bounds);
	
	menu_layer_set_callbacks(ui.menu_layer, NULL, (MenuLayerCallbacks) {
    	.get_num_sections = menu_get_num_sections_callback,
    	.get_num_rows = menu_get_num_rows_callback,
    	.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback
    });
	menu_layer_set_click_config_onto_window(ui.menu_layer, window);
	
	ui.text_layer = text_layer_create(bounds);
	text_layer_set_background_color(ui.text_layer, GColorBlack);
	text_layer_set_text_color(ui.text_layer, GColorWhite);
	text_layer_set_font(ui.text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(ui.text_layer, GTextAlignmentCenter);

	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));
	layer_add_child(window_layer, text_layer_get_layer(ui.text_layer));
	
    app_message_init();
}

static void window_unload(Window *window)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WINDOW UNLOAD!");
	
	menu_layer_destroy(ui.menu_layer);
	text_layer_destroy(ui.text_layer);
	current_station = 0;
	ui.initialized = false;
	
	app_message_deregister_callbacks();
}

void choose_station_init(void){
	ui.gs_window = window_create();
	window_set_window_handlers(ui.gs_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	
}

void choose_station_deinit(void){
	window_destroy(ui.gs_window);
}

void choose_station_show(void){
	window_stack_push(ui.gs_window, true);
}