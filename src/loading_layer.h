#ifndef LOADING_LAYER_H_
#define LOADING_LAYER_H_
	
#include <pebble.h>

#define MAX_LOADING_TICKS 3
	
typedef struct LoadingLayer {
	TextLayer *text_layer;
	bool hidden;
	char loading_text[10];
	uint8_t loading_progress;
} LoadingLayer;

LoadingLayer *loading_layer_create();
void loading_layer_destroy(LoadingLayer *layer);

void loading_layer_set_hidden(LoadingLayer *layer, bool hidden);
void loading_layer_tick_handler(LoadingLayer *layer, struct tm *tick_time, TimeUnits units_changed);
void loading_layer_set_text(LoadingLayer *layer, const char *text);

#endif