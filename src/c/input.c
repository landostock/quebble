#include "input.h"

static QbInputHandlers s_handlers;
static bool s_touch_enabled;

static void prv_stop_go(void) {
  if (s_handlers.stop_go) {
    s_handlers.stop_go();
  }
}

static void prv_turn_left(bool slow) {
  if (s_handlers.turn_left) {
    s_handlers.turn_left(slow);
  }
}

static void prv_turn_right(bool slow) {
  if (s_handlers.turn_right) {
    s_handlers.turn_right(slow);
  }
}

static void prv_toggle_slow(void) {
  if (s_handlers.toggle_slow) {
    s_handlers.toggle_slow();
  }
}

static void prv_tap(int x, int y) {
  if (s_handlers.tap) {
    s_handlers.tap(x, y);
  }
}

static void prv_back(void) {
  if (s_handlers.back) {
    s_handlers.back();
  }
}

static void prv_exit(void) {
  if (s_handlers.exit) {
    s_handlers.exit();
  }
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  prv_stop_go();
}

static void prv_select_long(ClickRecognizerRef recognizer, void *context) {
  prv_toggle_slow();
}

static void prv_up_click(ClickRecognizerRef recognizer, void *context) {
  prv_turn_left(false);
}

static void prv_up_long(ClickRecognizerRef recognizer, void *context) {
  prv_turn_left(true);
}

static void prv_down_click(ClickRecognizerRef recognizer, void *context) {
  prv_turn_right(false);
}

static void prv_down_long(ClickRecognizerRef recognizer, void *context) {
  prv_turn_right(true);
}

static void prv_back_click(ClickRecognizerRef recognizer, void *context) {
  prv_back();
}

static void prv_back_long(ClickRecognizerRef recognizer, void *context) {
  prv_exit();
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, 300, prv_select_long, NULL);

  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click);
  window_long_click_subscribe(BUTTON_ID_UP, 300, prv_up_long, NULL);

  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click);
  window_long_click_subscribe(BUTTON_ID_DOWN, 300, prv_down_long, NULL);

  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click);
  window_long_click_subscribe(BUTTON_ID_BACK, 650, prv_back_long, NULL);
}

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_FLINT)
static void prv_pan(const Recognizer *recognizer,RecognizerEvent event) {
  if(event!=RecognizerEvent_Started || !s_handlers.direction) return;
  const GPoint delta=pan_recognizer_get_total_delta(recognizer);
  QbDir direction;
  if(qb_abs_i(delta.x)>qb_abs_i(delta.y)) direction=delta.x>0?QB_DIR_RIGHT:QB_DIR_LEFT;
  else direction=delta.y>0?QB_DIR_DOWN:QB_DIR_UP;
  s_handlers.direction(direction);
}

static void prv_touch_tap(const Recognizer *recognizer,RecognizerEvent event) {
  if(event!=RecognizerEvent_Completed) return;
  GPoint point=tap_recognizer_get_tap_point(recognizer);
  prv_tap(point.x,point.y);
}
#endif

void input_init(Window *window, const QbInputHandlers *handlers) {
  s_handlers = *handlers;
  window_set_click_config_provider(window, prv_click_config_provider);

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_FLINT)
  if (touch_service_is_enabled()) {
    Recognizer *horizontal=pan_recognizer_create(prv_pan,NULL,PanAxis_Horizontal);
    Recognizer *vertical=pan_recognizer_create(prv_pan,NULL,PanAxis_Vertical);
    Recognizer *tap=tap_recognizer_create(prv_touch_tap,NULL);
    if(horizontal && vertical && tap) {
      /* SDK axis lock and exclusive recognition prevent a flick becoming a tap.
         Act once at Started: no velocity minimum and no need to keep dragging. */
      window_set_touch_bridge_disabled(window,true);
      window_attach_recognizer(window,horizontal);
      window_attach_recognizer(window,vertical);
      window_attach_recognizer(window,tap);
      app_touch_navigation_enable(true);
      s_touch_enabled=true;
    } else {
      if(horizontal) recognizer_destroy(horizontal);
      if(vertical) recognizer_destroy(vertical);
      if(tap) recognizer_destroy(tap);
    }
  }
#endif
}

void input_deinit(void) {
  if(s_touch_enabled) {
    app_touch_navigation_enable(false);
    s_touch_enabled=false;
  }
  /* Attached recognizers belong to the window and are destroyed on unload. */
}
