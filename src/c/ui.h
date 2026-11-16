#ifndef UI_H
#define UI_H

#include <pebble.h>

Layer *ui_layer_create(GRect bounds);
void ui_layer_destroy(Layer *layer);
void ui_mark_dirty(Layer *layer);

#endif
