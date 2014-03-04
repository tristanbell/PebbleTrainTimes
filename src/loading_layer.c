#include "loading_layer.h"

LoadingLayer *loading_layer_create(GRect bounds)
{
	LoadingLayer *layer = malloc(sizeof(LoadingLayer));
	
	layer->text_layer = text_layer_create(bounds);
	text_layer_set_background_color(layer->text_layer, GColorBlack);
	text_layer_set_text_color(layer->text_layer, GColorWhite);
	text_layer_set_font(layer->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(layer->text_layer, GTextAlignmentCenter);
	
	layer->loading_progress = 0;
	
	return layer;
}

void loading_layer_destroy(LoadingLayer *layer)
{
	text_layer_destroy(layer->text_layer);
	free(layer);
}

void loading_layer_set_hidden(LoadingLayer *layer, bool hidden)
{
	layer->hidden = hidden;
	layer_set_hidden(text_layer_get_layer(layer->text_layer), hidden);
}

void loading_layer_set_text(LoadingLayer *layer, const char *text)
{
	text_layer_set_text(layer->text_layer, text);
}

void loading_layer_tick_handler(LoadingLayer *layer, struct tm *tick_time, TimeUnits units_changed) {
	if (layer->loading_progress++ > MAX_LOADING_TICKS) {
		layer->loading_progress = 0;
	}
	
	strcpy(layer->loading_text, "Loading");
	
	for (int i = 0; i < layer->loading_progress; i++) {
		strcat(layer->loading_text, ".");
	}
	
	text_layer_set_text(layer->text_layer, layer->loading_text);
}