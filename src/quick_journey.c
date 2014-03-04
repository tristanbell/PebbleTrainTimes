#include <stdio.h>

#include <pebble.h>
#include "quick_journey.h"
#include "departure_data.h"
#include "keys.h"
#include "loading_layer.h"
	
#define NUM_MENU_SECTIONS 1

//static uint16_t num_menu_items = 0;
#define NUM_DEPARTURES 25

static struct {
    Window *qj_window;
    MenuLayer *menu_layer;
	LoadingLayer *loading_layer;
	bool initialized;
} ui;

static DepartureDataList* departures;
//static DepartureData departures[NUM_DEPARTURES];
static int current_dep = 0;
char station[35];

//TODO: make this an array so double digit numbers can be created by index 
GBitmap *one_bmp, *two_bmp, *three_bmp, *four_bmp, *five_bmp;

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	if (section_index == 0 && station[0]) {
		menu_cell_basic_header_draw(ctx, cell_layer, station);
	}
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
        return departure_data_list_get_size(departures);
    default:
      return 0;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
      DepartureData dep_data = departure_data_list_get(departures, cell_index->row);

	  if (!dep_data.destination) return;
	  
	  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Dest: %s, Dep: %s", dep_data.destination, dep_data.expected_departure);
	  
	  switch (dep_data.platform) {
		  case 1:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, one_bmp);
		  	break;
		  case 2:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, two_bmp);
		  	break;
		  case 3:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, three_bmp);
		  	break;
		  case 4:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, four_bmp);
		  	break;
		  case 5:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, five_bmp);
		  	break;
		  
		  default:
		  	menu_cell_basic_draw(ctx, cell_layer, dep_data.destination, dep_data.expected_departure, NULL);
		  	break;
	  }
  }
}

static void fetch_msg(void) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "fetching first message...");
	loading_layer_set_text(ui.loading_layer, "Loading");
	
  	Tuplet fetch_tuple = TupletInteger(DEPARTURE_KEY_FETCH, 1);

  	DictionaryIterator *iter;
  	AppMessageResult result = app_message_outbox_begin(&iter);

	if (iter == NULL) {		
		if (result == APP_MSG_INVALID_ARGS)
			APP_LOG(APP_LOG_LEVEL_DEBUG, "App message invalid args");
		if (result == APP_MSG_BUSY)
			APP_LOG(APP_LOG_LEVEL_DEBUG, "App message busy");
		if (result == APP_MSG_NOT_CONNECTED)
			APP_LOG(APP_LOG_LEVEL_DEBUG, "App message not connected");
		if (result == APP_MSG_CALLBACK_NOT_REGISTERED)
			APP_LOG(APP_LOG_LEVEL_DEBUG, "App message callback not registered");
    	return;
    }

  	dict_write_tuplet(iter, &fetch_tuple);
  	dict_write_end(iter);

  	app_message_outbox_send();
}
	
static void out_sent_handler(DictionaryIterator *sent, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sent.");
	/*
	Tuple *tuple = dict_read_first(sent);
	while (tuple) {
  		APP_LOG(APP_LOG_LEVEL_DEBUG, "\t%d\n", (int) (tuple->key));
  	    
		tuple = dict_read_next(sent);
    }
	*/
}


static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sending failed.");
	
	loading_layer_set_text(ui.loading_layer, "Error: initialization failed.");
	loading_layer_set_hidden(ui.loading_layer, false);
}


static void in_received_handler(DictionaryIterator *received, void *context)
{	
	// Perhaps move this bit?
	if (!ui.initialized) {
		ui.initialized = true;
		loading_layer_set_hidden(ui.loading_layer, true);
	}
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "received.");
	
	if (current_dep++ >= NUM_DEPARTURES) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Departure limit reached.");
		return;
	}
	
	Tuple *from = dict_find(received, DEPARTURE_KEY_FROM);
  	Tuple *dest = dict_find(received, DEPARTURE_KEY_DESTINATION);
	Tuple *exp_departure = dict_find(received, DEPARTURE_KEY_EXP_DEPARTURE);
  	Tuple *platform = dict_find(received, DEPARTURE_KEY_PLATFORM);

	DepartureData data;

  	if (from) {
		char *from_station = from->value->cstring;
		if (!station[0] || strcmp(station, from_station) != 0) {
    //		strcpy(data.from, from->value->cstring);
			strncpy(station, from_station, sizeof(station));
		}
	}
	if (dest) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "\tDest: %s", dest->value->cstring);
    	strncpy(data.destination, dest->value->cstring, sizeof(data.destination));
    }
	if (exp_departure) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "\tDeparture: %s", exp_departure->value->cstring);
    	strncpy(data.expected_departure, exp_departure->value->cstring, sizeof(data.expected_departure));
    }
	if (platform) {
    	data.platform = atoi(platform->value->cstring);
    }
	
	if (dest && exp_departure) {
		departure_data_list_add(&departures, data);
		//departure_list_add(&departures, data);
		//departures[current_dep++] = data;
	
		menu_layer_reload_data(ui.menu_layer);
		layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
	} else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Message not recognised.");
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message dropped.");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (ui.initialized) return;
	
	loading_layer_tick_handler(ui.loading_layer, tick_time, units_changed);
}

static void app_message_init(void)
{	
	app_message_register_inbox_received(in_received_handler);
   	app_message_register_inbox_dropped(in_dropped_handler);
   	app_message_register_outbox_sent(out_sent_handler);
  	app_message_register_outbox_failed(out_failed_handler);
	
	fetch_msg();
}

// TODO: set a tick handler for loading progress

static void window_load(Window *window)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WINDOW LOAD!");
	
     //   departures = departure_list_create(num_departures);
	Layer *window_layer = window_get_root_layer(window);
 	GRect bounds = layer_get_frame(window_layer);
	
	one_bmp = gbitmap_create_with_resource(RESOURCE_ID_PLATFORM_1);
	two_bmp = gbitmap_create_with_resource(RESOURCE_ID_PLATFORM_2);
	three_bmp = gbitmap_create_with_resource(RESOURCE_ID_PLATFORM_3);
	four_bmp = gbitmap_create_with_resource(RESOURCE_ID_PLATFORM_4);
	five_bmp = gbitmap_create_with_resource(RESOURCE_ID_PLATFORM_5);
	
	ui.menu_layer = menu_layer_create(bounds);
	
	menu_layer_set_callbacks(ui.menu_layer, NULL, (MenuLayerCallbacks) {
    	.get_num_sections = menu_get_num_sections_callback,
    	.get_num_rows = menu_get_num_rows_callback,
		.draw_header = menu_draw_header_callback,
		.get_header_height = menu_get_header_height_callback,
    	.draw_row = menu_draw_row_callback
    });
	menu_layer_set_click_config_onto_window(ui.menu_layer, window);
	
	ui.loading_layer = loading_layer_create(bounds);

	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));
	layer_add_child(window_layer, text_layer_get_layer(ui.loading_layer->text_layer));
	
    app_message_init();
	
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void window_unload(Window *window)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WINDOW UNLOAD!");
	
	menu_layer_destroy(ui.menu_layer);
    departure_data_list_destroy(departures);
	departures = NULL;
	loading_layer_destroy(ui.loading_layer);
	ui.initialized = false;
	current_dep = 0;
	
	gbitmap_destroy(one_bmp);
	gbitmap_destroy(two_bmp);
	gbitmap_destroy(three_bmp);
	gbitmap_destroy(four_bmp);
	gbitmap_destroy(five_bmp);
	
	app_message_deregister_callbacks();
	tick_timer_service_unsubscribe();
}

void quick_journey_init(void)
{
	ui.qj_window = window_create();
	window_set_window_handlers(ui.qj_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
}

void quick_journey_deinit(void)
{
	window_destroy(ui.qj_window);
}

void quick_journey_show(void)
{
	window_stack_push(ui.qj_window, true);
}