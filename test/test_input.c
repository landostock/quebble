#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "input.h"

struct Window { bool bridge_disabled; };
struct Recognizer { RecognizerEventCb cb; GPoint delta; bool attached; };
static struct Recognizer recognizers[3];
static unsigned allocated,attached,destroyed,taps,directions;
static bool enabled=true,allocation_failure;
static bool opted_in;
static QbDir last_direction;
static ClickHandler clicks[4],long_clicks[4];
Recognizer *pan_recognizer_create(RecognizerEventCb cb,void *ctx,PanAxis axis) {
  (void)ctx; (void)axis;
  if(allocation_failure && allocated==1) return NULL;
  recognizers[allocated].cb=cb;
  return &recognizers[allocated++];
}
Recognizer *tap_recognizer_create(RecognizerEventCb cb,void *ctx) {
  return pan_recognizer_create(cb,ctx,PanAxis_Horizontal);
}
GPoint pan_recognizer_get_total_delta(const Recognizer *r) { return r->delta; }
GPoint tap_recognizer_get_tap_point(const Recognizer *r) { return r->delta; }
void recognizer_destroy(Recognizer *r) { assert(!r->attached); destroyed++; }
bool touch_service_is_enabled(void) { return enabled; }
void app_touch_navigation_enable(bool value) { opted_in=value; }
void window_set_click_config_provider(Window *w,void (*cb)(void *)) { cb(w); }
void window_set_touch_bridge_disabled(Window *w,bool disabled) { w->bridge_disabled=disabled; }
void window_attach_recognizer(Window *w,Recognizer *r) { (void)w; r->attached=true; attached++; }
void window_single_click_subscribe(int b,ClickHandler cb) { clicks[b]=cb; }
void window_long_click_subscribe(int b,unsigned ms,ClickHandler down,ClickHandler up) {
  (void)ms; (void)up; long_clicks[b]=down;
}
static void tap(int x,int y) { assert(x==75 && y==100); taps++; }
static void direction(QbDir d) { last_direction=d; directions++; }
int main(void) {
  Window w={0};
  QbInputHandlers handlers={.tap=tap,.direction=direction};
  input_init(&w,&handlers);
  assert(w.bridge_disabled && allocated==3 && attached==3 && opted_in);
  for(int i=0;i<4;i++) assert(clicks[i] && long_clicks[i]);
  const GPoint deltas[]={{12,1},{-13,2},{1,-12},{-1,11}};
  const QbDir expected[]={QB_DIR_RIGHT,QB_DIR_LEFT,QB_DIR_UP,QB_DIR_DOWN};
  for(unsigned i=0;i<4;i++) {
    Recognizer *r=&recognizers[i<2?0:1];
    r->delta=deltas[i]; r->cb(r,RecognizerEvent_Started);
    assert(last_direction==expected[i] && directions==i+1 && taps==0);
    r->delta=(GPoint){0,0};
    r->cb(r,RecognizerEvent_Updated); r->cb(r,RecognizerEvent_Completed);
    r->cb(r,RecognizerEvent_Cancelled);
    assert(directions==i+1 && taps==0);
  }
  recognizers[2].delta=(GPoint){75,100};
  recognizers[2].cb(&recognizers[2],RecognizerEvent_Completed);
  assert(taps==1);
  input_deinit(); assert(destroyed==0 && !opted_in);
  memset(recognizers,0,sizeof(recognizers)); /* Simulate window-owned destruction. */
  enabled=false; allocated=attached=0; w.bridge_disabled=false;
  input_init(&w,&handlers); assert(!allocated && !w.bridge_disabled);
  enabled=true; allocation_failure=true;
  input_init(&w,&handlers); assert(!attached && !w.bridge_disabled);
  assert(destroyed==1);
  puts("input ok: native axis recognizers, absolute directions, one impulse per gesture, button fallback");
}
