#ifndef QUEBBLE_INPUT_PEBBLE_H
#define QUEBBLE_INPUT_PEBBLE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { int16_t x,y; } GPoint;
typedef struct Window Window;
typedef void *ClickRecognizerRef;
typedef void (*ClickHandler)(ClickRecognizerRef,void *);
enum { BUTTON_ID_SELECT,BUTTON_ID_UP,BUTTON_ID_DOWN,BUTTON_ID_BACK };
typedef enum { RecognizerEvent_Started,RecognizerEvent_Updated,RecognizerEvent_Completed,RecognizerEvent_Cancelled } RecognizerEvent;
typedef enum { PanAxis_Horizontal,PanAxis_Vertical } PanAxis;
typedef struct Recognizer Recognizer;
typedef void (*RecognizerEventCb)(const Recognizer *,RecognizerEvent);
Recognizer *pan_recognizer_create(RecognizerEventCb cb,void *ctx,PanAxis axis);
Recognizer *tap_recognizer_create(RecognizerEventCb cb,void *ctx);
GPoint pan_recognizer_get_total_delta(const Recognizer *r);
GPoint tap_recognizer_get_tap_point(const Recognizer *r);
void recognizer_destroy(Recognizer *r);
bool touch_service_is_enabled(void);
void app_touch_navigation_enable(bool enabled);
void window_set_click_config_provider(Window *w,void (*cb)(void *));
void window_set_touch_bridge_disabled(Window *w,bool disabled);
void window_attach_recognizer(Window *w,Recognizer *r);
void window_single_click_subscribe(int button,ClickHandler cb);
void window_long_click_subscribe(int button,unsigned ms,ClickHandler down,ClickHandler up);
#endif
