#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "feedback.h"
#include "audio_mix.h"

struct AppTimer { int valid; };
static struct AppTimer timer;
static void (*pump)(void *);
static SpeakerFinishedCallback finish;
static void *finish_context;
static uint32_t now,queued,written,total_written,limit=UINT32_MAX;
static unsigned opens,closes,stops,pulses;
static uint32_t trailing_silence;
static uint32_t last_duration;
static bool muted,quiet,fail_open,defer_finish,pending_finish;
static SpeakerFinishReason pending_reason;
static SpeakerStatus status;
static QbGameView view;
QbGameView game_view(void) { return view; }
uint16_t time_ms(time_t *seconds,uint16_t *ms) { *seconds=now/1000; *ms=now%1000; return *ms; }
AppTimer *app_timer_register(uint32_t ms,void (*callback)(void *),void *ctx) {
  (void)ctx; assert(ms==15); pump=callback; return &timer;
}
bool app_timer_cancel(AppTimer *t) { assert(t==&timer); pump=NULL; return true; }
static void notify(SpeakerFinishReason reason) {
  if(defer_finish) { pending_finish=true; pending_reason=reason; }
  else if(finish) finish(reason,finish_context);
}
void speaker_stop(void) { stops++; status=SpeakerStatusIdle; queued=0; notify(SpeakerFinishReasonStopped); }
void speaker_set_finish_callback(SpeakerFinishedCallback callback,void *ctx) { finish=callback; finish_context=ctx; }
bool speaker_is_muted(void) { return muted; }
bool speaker_stream_open(SpeakerPcmFormat format,uint8_t volume) {
  assert(format==SpeakerPcmFormat_8kHz_16bit && volume<=100);
  opens++; if(fail_open) return false; trailing_silence=0; status=SpeakerStatusPlaying; return true;
}
uint32_t speaker_stream_write(const void *data,uint32_t n) {
  (void)data;
  assert(status==SpeakerStatusPlaying);
  if(n>limit) n=limit;
  if(n>2048-queued) n=2048-queued;
  n&=~1U;
  const int16_t *samples=data;
  for(uint32_t i=0;i<n/2;i++) trailing_silence=samples[i]?0:trailing_silence+2;
  queued+=n; written+=n; total_written+=n; return n;
}
void speaker_stream_close(void) {
  assert(trailing_silence>=4*QB_AUDIO_BLOCK_BYTES);
  closes++; status=SpeakerStatusDraining;
}
SpeakerStatus speaker_get_status(void) { return status; }
void speaker_set_volume(uint8_t v) { assert(v<=100); }
ResHandle resource_get_handle(uint32_t id) { assert(id>=1 && id<=8); return (int)id; }
size_t resource_size(ResHandle h) { return h==RESOURCE_ID_AUDIO_MUSIC?16000:1600; }
size_t resource_load_byte_range(ResHandle h,uint32_t offset,uint8_t *out,size_t n) {
  assert(offset+n<=resource_size(h)); memset(out,h*8,n); return n;
}
bool quiet_time_is_active(void) { return quiet; }
void vibes_cancel(void) {}
void vibes_enqueue_custom_pattern(VibePattern p) {
  assert(p.num_segments>0 && p.num_segments<=5);
  assert(p.num_segments==1);
  last_duration=p.durations[0];
  uint32_t ms=0; for(unsigned i=0;i<p.num_segments;i++) ms+=p.durations[i];
  assert(ms<=260); pulses++;
}

static void advance(unsigned ms) {
  for(unsigned t=0;t<ms;t+=15) {
    now+=15;
    queued=queued>240?queued-240:0;
    if(status==SpeakerStatusDraining && queued==0) { status=SpeakerStatusIdle; notify(SpeakerFinishReasonDone); }
    written=0;
    assert(pump); pump(NULL);
    assert(written<=3*QB_AUDIO_BLOCK_BYTES && queued<=5*QB_AUDIO_BLOCK_BYTES/2);
  }
}

int main(void) {
  view=(QbGameView){.screen=GAME_SCREEN_TITLE,.music_enabled=true,.effects_enabled=true,.haptics_enabled=true,.volume=25};
  feedback_init();
  advance(90); assert(opens==0);
  view.screen=GAME_SCREEN_COUNTDOWN; view.countdown=3;
  feedback_update(view,true); assert(pulses==1);
  advance(1030); assert(opens==1 && status==SpeakerStatusPlaying);
  feedback_update(view,true); assert(pulses==1);
  view.countdown=2; feedback_update(view,true); advance(1000);
  view.countdown=1; feedback_update(view,true); advance(1000);
  assert(opens==1 && closes==0 && status==SpeakerStatusPlaying);
  view.screen=GAME_SCREEN_PLAYING; view.moving=true;
  feedback_update(view,true); advance(1000);
  assert(status==SpeakerStatusPlaying && total_written>16000 && opens==1);
  view.music_enabled=false; feedback_update(view,true); advance(600);
  assert(opens==1 && closes==0 && status==SpeakerStatusPlaying);
  view.effects_enabled=false; feedback_update(view,true); advance(400);
  assert(closes==1 && status==SpeakerStatusIdle);
  view.music_enabled=view.effects_enabled=true; feedback_update(view,true); advance(90);
  assert(status==SpeakerStatusPlaying);
  unsigned before=pulses;
  for(int i=0;i<30;i++) feedback_update(view,true);
  assert(pulses==before);
  limit=17; advance(90); limit=UINT32_MAX; advance(300);
  defer_finish=true;
  view.screen=GAME_SCREEN_TITLE; feedback_update(view,true); advance(400);
  assert(pending_finish && status==SpeakerStatusIdle);
  unsigned old_opens=opens;
  view.screen=GAME_SCREEN_PLAYING; feedback_update(view,true); advance(30);
  assert(opens==old_opens);
  defer_finish=false; pending_finish=false; finish(pending_reason,finish_context);
  advance(90); assert(status==SpeakerStatusPlaying && opens>old_opens);
  defer_finish=true;
  feedback_update(view,false); assert(status==SpeakerStatusIdle && queued==0 && pending_finish);
  old_opens=opens; feedback_update(view,true); advance(30); assert(opens==old_opens);
  defer_finish=false; pending_finish=false; finish(pending_reason,finish_context);
  advance(90); assert(status==SpeakerStatusPlaying && opens>old_opens);
  feedback_update(view,false);
  muted=true; feedback_update(view,true); old_opens=opens;
  advance(300); assert(opens==old_opens);
  muted=false; feedback_update(view,true); advance(90); assert(status==SpeakerStatusPlaying);
  status=SpeakerStatusIdle; queued=0; finish(SpeakerFinishReasonPreempted,finish_context);
  old_opens=opens; advance(300); assert(opens==old_opens);
  advance(900); assert(opens>old_opens);
  quiet=true; before=pulses; view.screen=GAME_SCREEN_DEATH;
  feedback_update(view,true); assert(pulses==before);
  advance(1000); assert(status==SpeakerStatusPlaying);
  view.screen=GAME_SCREEN_TITLE; feedback_update(view,true);
  advance(400); assert(status==SpeakerStatusIdle);
  quiet=false; view.screen=GAME_SCREEN_PLAYING;
  fail_open=true; feedback_update(view,true); advance(90);
  old_opens=opens; advance(300); assert(opens==old_opens);
  fail_open=false; advance(1000); assert(status==SpeakerStatusPlaying);
  before=pulses;
  view.moving=!view.moving; feedback_update(view,true); assert(pulses==before);
  view.screen=GAME_SCREEN_DEATH; feedback_update(view,true);
  assert(pulses==before+1 && last_duration==110);
  view.screen=GAME_SCREEN_GAME_OVER; feedback_update(view,true);
  assert(pulses==before+1);
  feedback_deinit(); assert(!pump && !finish && status==SpeakerStatusIdle && stops>0);
  puts("feedback ok: partial writes, queue/latency bounds, mute/focus/quiet-time, preemption, retry, cleanup");
}
