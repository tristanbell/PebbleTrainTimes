#include <pebble.h>
#include "quick_journey.h"
#include "choose_station.h"
	
#define NUM_MENU_ITEMS 2
#define NUM_SECTIONS 1

Window *my_window;
TextLayer *title_layer;
SimpleMenuLayer *menu;

SimpleMenuSection menu_sections[NUM_SECTIONS];
SimpleMenuItem menu_items[NUM_MENU_ITEMS];

void menu_select_callback(int index, void *ctx)
{
	if (index == 0)
		quick_journey_show();
	else if (index == 1)
		choose_station_show();
}

void title_layer_init(Layer *root_layer, GRect bounds)
{
	title_layer = text_layer_create(bounds);
	text_layer_set_background_color(title_layer, GColorBlack);
	text_layer_set_text_color(title_layer, GColorWhite);
	text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	
	text_layer_set_text(title_layer, "Trains");
	
	layer_add_child(root_layer, (Layer*)title_layer);
}

void menu_init(Layer *root_layer, GRect bounds)
{
	int item_index = 0;
	
	menu_items[item_index++] = (SimpleMenuItem) {
		.title = "Quick Departures",
		.callback = menu_select_callback
	};
	menu_items[item_index++] = (SimpleMenuItem) {
		.title = "Select Station",
		.callback = menu_select_callback
	};
	
	menu_sections[0] = (SimpleMenuSection) {
		.num_items = NUM_MENU_ITEMS,
		.items = menu_items
	};
	
	menu = simple_menu_layer_create(bounds, my_window, menu_sections, NUM_SECTIONS, NULL);
	
	layer_add_child(root_layer, simple_menu_layer_get_layer(menu));
}

void window_load(Window *window)
{
	Layer *root_layer = window_get_root_layer(my_window);
	title_layer_init(root_layer, GRect(0, 0, 144, 60));
	menu_init(root_layer, GRect(0, 63, 144, 105));
}

void window_unload(Window *window)
{
	text_layer_destroy(title_layer);
	simple_menu_layer_destroy(menu);
}

void app_message_init()
{
   	const uint32_t inbound_size = app_message_inbox_size_maximum();
   	const uint32_t outbound_size = app_message_outbox_size_maximum();
   	app_message_open(inbound_size, outbound_size);
}

void handle_init(void)
{
	quick_journey_init();
	choose_station_init();
	app_message_init();
	my_window = window_create();
	window_set_window_handlers(my_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	
	window_stack_push(my_window, true);
}

void handle_deinit(void)
{
	window_destroy(my_window);
	quick_journey_deinit();
	choose_station_deinit();
}

int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
