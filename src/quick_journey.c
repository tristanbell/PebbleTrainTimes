#include <stdio.h>

#include <pebble.h>
#include "quick_journey.h"
#include "departure_data.h"
	
#define NUM_MENU_SECTIONS 1

//static uint16_t num_menu_items = 0;
#define NUM_DEPARTURES 25// Default value

enum {
  DEPARTURE_KEY_FROM = 0x0,
  DEPARTURE_KEY_DESTINATION = 0x1,
  DEPARTURE_KEY_EXP_DEPARTURE = 0x2,
  DEPARTURE_KEY_PLATFORM = 0x3,
  DEPARTURE_KEY_FETCH = 0x4,	
};

static struct QuickUI {
     Window *qj_window;
     MenuLayer *menu_layer;
} ui;

static DepartureDataList* departures;
//static DepartureData departures[NUM_DEPARTURES];
static int current_dep = 0;
char station[35];

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
        //return departures.size;
        //return current_dep;

    default:
      return 0;
  }
}

void on_animation_stopped(Animation *anim, bool finished, void *context) {
	// Free animation memory
	property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay) {
	// Declare animation
	PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
	
	// Set characteristics
	animation_set_duration((Animation*) anim, duration);
	animation_set_delay((Animation*) anim, delay);
	
	// Set stopped handler
	AnimationHandlers handlers = {
		.stopped = (AnimationStoppedHandler) on_animation_stopped
	};
	animation_set_handlers((Animation*) anim, handlers, NULL);
	
	// Start animation
	animation_schedule((Animation*) anim);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
      DepartureData data = departure_data_list_get(departures, cell_index->row);
      //DepartureData data = departures[cell_index->row];
	  if (!data.destination) return;
	  
	  switch (data.platform) {
		  case 1:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, one_bmp);
		  	break;
		  case 2:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, two_bmp);
		  	break;
		  case 3:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, three_bmp);
		  	break;
		  case 4:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, four_bmp);
		  	break;
		  case 5:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, five_bmp);
		  	break;
		  
		  default:
		  	menu_cell_basic_draw(ctx, cell_layer, data.destination, data.expected_departure, NULL);
		  	break;
	  }
  }
}

static void fetch_msg(void) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "fetching first message...");
	
  Tuplet fetch_tuple = TupletInteger(DEPARTURE_KEY_FETCH, 1);
  Tuplet from_tuple = TupletCString(DEPARTURE_KEY_FROM, "hi");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &fetch_tuple);
  dict_write_tuplet(iter, &from_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}
	
static void out_sent_handler(DictionaryIterator *sent, void *context)
{
   // outgoing message was delivered
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sent.");
 }


static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
  // outgoing message failed
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sending failed.");
}


static void in_received_handler(DictionaryIterator *received, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "received.");
	
	if (current_dep >= NUM_DEPARTURES) return;
	
	Tuple *from = dict_find(received, DEPARTURE_KEY_FROM);
  	Tuple *dest = dict_find(received, DEPARTURE_KEY_DESTINATION);
	Tuple *exp_departure = dict_find(received, DEPARTURE_KEY_EXP_DEPARTURE);
  	Tuple *platform = dict_find(received, DEPARTURE_KEY_PLATFORM);

	DepartureData data;

  	if (from && !station[0]) {
    //	strcpy(data.from, from->value->cstring);
		strncpy(station, from->value->cstring, sizeof(station));
	}

	if (dest) {
    	strcpy(data.destination, dest->value->cstring);
    }
	if (exp_departure) {
    	strcpy(data.expected_departure, exp_departure->value->cstring);
    }
	if (platform) {
    	data.platform= atoi(platform->value->cstring);
    }
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Destination received: %s", data.destination);
	
	departure_data_list_add(&departures, data);
    //  departure_list_add(&departures, data);
	//departures[current_dep++] = data;
	
    menu_layer_reload_data(ui.menu_layer);
	layer_mark_dirty(menu_layer_get_layer(ui.menu_layer));
}


static void in_dropped_handler(AppMessageResult reason, void *context)
{
  // incoming message dropped
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message dropped.");
}

static void app_message_init(void)
{	
	app_message_register_inbox_received(in_received_handler);
   	app_message_register_inbox_dropped(in_dropped_handler);
   	app_message_register_outbox_sent(out_sent_handler);
  	app_message_register_outbox_failed(out_failed_handler);

   	const uint32_t inbound_size = 256;
   	const uint32_t outbound_size = 64;
   	app_message_open(inbound_size, outbound_size);
	
	fetch_msg();
}

static void window_load(Window *window)
{
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

	layer_add_child(window_layer, menu_layer_get_layer(ui.menu_layer));
	
    app_message_init();
}

static void window_unload(Window *window)
{
	menu_layer_destroy(ui.menu_layer);
 //   departure_data_list_destroy(departures);
 //       departure_list_destroy(departures);
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