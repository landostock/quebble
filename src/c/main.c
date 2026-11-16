#include <pebble.h>

#include "game.h"
#include "input.h"
#include "ui.h"
#include "feedback.h"

static Window *s_window;
static Layer *s_game_layer;
static AppTimer *s_timer;
static bool s_in_focus = true;
static bool s_light_held;

static void prv_sync_backlight(void) {
  const bool keep_on = s_in_focus && s_game_layer && game_backlight_required();
  if (keep_on != s_light_held) {
    light_enable(keep_on);
    s_light_held = keep_on;
  }
}

static void prv_focus_changed(bool in_focus) {
  s_in_focus = in_focus;
  prv_sync_backlight();
  feedback_update(game_view(),in_focus);
}

static void prv_timer(void *context) {
  game_update();
  feedback_update(game_view(),s_in_focus);
  prv_sync_backlight();
  if (s_game_layer) {
    ui_mark_dirty(s_game_layer);
  }
  s_timer = app_timer_register(QB_TIMER_MS, prv_timer, NULL);
}

static void prv_input_stop_go(void) {
  game_stop_go();
}

static void prv_input_tap(int x, int y) {
  game_touch_tap(x, y);
}

static void prv_input_turn_left(bool slow) {
  game_turn_left(slow);
}

static void prv_input_turn_right(bool slow) {
  game_turn_right(slow);
}

static void prv_input_toggle_slow(void) {
  game_toggle_slow();
}

static void prv_input_back(void) {
  game_back_to_title();
}

static void prv_input_exit(void) {
  window_stack_pop_all(true);
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_game_layer = ui_layer_create(layer_get_bounds(root));
  layer_add_child(root, s_game_layer);
}

static void prv_window_unload(Window *window) {
  s_in_focus=false;
  feedback_update(game_view(),false);
  if (s_game_layer) {
    ui_layer_destroy(s_game_layer);
    s_game_layer = NULL;
  }
  prv_sync_backlight();
}

static void prv_init(void) {
  game_init();
  feedback_init();

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });

  const QbInputHandlers handlers = {
    .stop_go = prv_input_stop_go,
    .turn_left = prv_input_turn_left,
    .turn_right = prv_input_turn_right,
    .toggle_slow = prv_input_toggle_slow,
    .tap = prv_input_tap,
    .direction = game_set_direction,
    .back = prv_input_back,
    .exit = prv_input_exit
  };
  input_init(s_window, &handlers);

  window_stack_push(s_window, true);
  app_focus_service_subscribe_handlers((AppFocusHandlers){.did_focus = prv_focus_changed});
  s_timer = app_timer_register(QB_TIMER_MS, prv_timer, NULL);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Quebble ready. Free heap: %u", (unsigned)heap_bytes_free());
}

static void prv_deinit(void) {
  feedback_deinit();
  light_enable(false);
  app_focus_service_unsubscribe();
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  input_deinit();
  game_deinit();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
