#ifndef INPUT_H
#define INPUT_H

#include <pebble.h>

#include "quebble_config.h"

typedef struct {
  void (*stop_go)(void);
  void (*turn_left)(bool slow);
  void (*turn_right)(bool slow);
  void (*toggle_slow)(void);
  void (*tap)(int x, int y);
  void (*direction)(QbDir direction);
  void (*back)(void);
  void (*exit)(void);
} QbInputHandlers;

void input_init(Window *window, const QbInputHandlers *handlers);
void input_deinit(void);

#endif
