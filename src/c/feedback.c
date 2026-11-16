#include "feedback.h"
#include "audio_mix.h"
#include <string.h>

static QbAudioMix s_mix;
static ResHandle s_resources[QB_AUDIO_CLIP_COUNT];
static AppTimer *s_timer;
static QbGameView s_view,s_previous;
static bool s_focused=true,s_owned,s_draining,s_waiting_finish;
static uint32_t s_last_ms,s_retry_ms,s_queued;
static int16_t s_pending[QB_AUDIO_BLOCK];
static uint32_t s_pending_size,s_pending_offset;
static unsigned s_tail_blocks;

static uint32_t prv_ms(void) {
  time_t seconds;
  uint16_t ms;
  time_ms(&seconds,&ms);
  return (uint32_t)((uint64_t)seconds*1000+ms);
}

static uint32_t prv_read(QbAudioClip clip,uint32_t offset,int16_t *out,uint32_t count,void *context) {
  (void)context;
  return (uint32_t)resource_load_byte_range(s_resources[clip],offset*QB_AUDIO_SAMPLE_BYTES,
    (uint8_t *)out,count*QB_AUDIO_SAMPLE_BYTES)/QB_AUDIO_SAMPLE_BYTES;
}

static bool prv_keep_round_stream(void) {
  if(!s_view.music_enabled && !s_view.effects_enabled) return false;
  return s_view.screen==GAME_SCREEN_COUNTDOWN || s_view.screen==GAME_SCREEN_LEVEL ||
    s_view.screen==GAME_SCREEN_PLAYING || s_view.screen==GAME_SCREEN_CLAIM_PAUSE ||
    s_view.screen==GAME_SCREEN_DEATH || s_view.screen==GAME_SCREEN_LEVEL_CLEAR;
}

static void prv_stop_stream(void) {
  if(s_owned) {
    s_waiting_finish=true;
    s_owned=false;
    speaker_stop();
  }
  s_owned=s_draining=false;
  s_pending_size=s_pending_offset=s_queued=0;
  s_tail_blocks=0;
}

static void prv_finished(SpeakerFinishReason reason,void *context) {
  (void)context;
  bool requested=s_waiting_finish;
  s_waiting_finish=false;
  s_owned=s_draining=false;
  s_pending_size=s_pending_offset=s_queued=0;
  s_tail_blocks=0;
  if(!requested && reason!=SpeakerFinishReasonDone) {
    s_retry_ms=prv_ms()+1000;
    audio_mix_clear_effect(&s_mix);
    s_mix.music_gain=0;
  }
}

static void prv_pump(void *context) {
  (void)context;
  uint32_t now=prv_ms(),elapsed=now-s_last_ms;
  s_last_ms=now;
  if(elapsed>1000) elapsed=1000;
  uint32_t consumed=elapsed*8*QB_AUDIO_SAMPLE_BYTES;
  s_queued=consumed<s_queued?s_queued-consumed:0;
  bool audible=s_focused && s_view.volume && !speaker_is_muted();
  if(!audible) {
    prv_stop_stream();
    audio_mix_clear_effect(&s_mix);
    s_mix.music_gain=0;
  } else {
    /* A completion event belongs to the old request. Wait for it before opening
       another stream, even if firmware already reports Idle. */
    if(!s_owned && !s_waiting_finish && audio_mix_active(&s_mix) && (int32_t)(now-s_retry_ms)>=0 &&
       speaker_get_status()==SpeakerStatusIdle) {
      s_owned=speaker_stream_open(SpeakerPcmFormat_8kHz_16bit,s_view.volume);
      if(!s_owned) {
        s_retry_ms=now+1000;
        audio_mix_clear_effect(&s_mix);
      }
    }
    /* Bound both queued latency and per-callback work; never wait on the speaker. */
    for(int refill=0;s_owned && !s_draining && s_queued<3*QB_AUDIO_BLOCK_BYTES/2 && refill<3;refill++) {
      if(s_pending_offset==s_pending_size) {
        if(audio_mix_active(&s_mix) || prv_keep_round_stream()) s_tail_blocks=0;
        else if(s_tail_blocks++>=4) {
          /* Drain at least 128 ms of digital silence before releasing the DAC. */
          s_draining=true;
          speaker_stream_close();
          break;
        }
        audio_mix_render(&s_mix,s_pending,QB_AUDIO_BLOCK,prv_read,NULL);
        s_pending_size=sizeof(s_pending);
        s_pending_offset=0;
      }
      uint32_t written=speaker_stream_write((uint8_t *)s_pending+s_pending_offset,
                                             s_pending_size-s_pending_offset);
      if(!written) break;
      s_pending_offset+=written;
      s_queued+=written;
    }
  }
  s_timer=app_timer_register(15,prv_pump,NULL);
}

static void prv_event(QbAudioClip clip,uint8_t priority,const uint32_t *segments,unsigned count) {
  if(s_focused && s_view.effects_enabled && s_view.volume && !speaker_is_muted() &&
     (int32_t)(prv_ms()-s_retry_ms)>=0 && (s_owned || speaker_get_status()==SpeakerStatusIdle))
    audio_mix_effect(&s_mix,clip,priority);
  if(s_focused && s_view.haptics_enabled && !quiet_time_is_active() && count) {
    vibes_cancel();
    vibes_enqueue_custom_pattern((VibePattern){.durations=segments,.num_segments=count});
  }
}

void feedback_update(QbGameView view,bool focused) {
  static const uint32_t tick[]={10},start[]={28},claim[]={32},hit[]={110};
  static const uint32_t clear[]={65};
  s_view=view;
  s_focused=focused;
  s_mix.music=focused && view.music_enabled &&
    (view.screen==GAME_SCREEN_PLAYING || view.screen==GAME_SCREEN_CLAIM_PAUSE);
  if(!view.effects_enabled) audio_mix_clear_effect(&s_mix);
  if(!view.haptics_enabled && s_previous.haptics_enabled) vibes_cancel();
  if(!focused) {
    prv_stop_stream();
    audio_mix_clear_effect(&s_mix);
    s_mix.music_gain=0;
    s_previous=view;
    return;
  }
  if(s_owned && view.volume!=s_previous.volume) speaker_set_volume(view.volume);
  if(view.screen==GAME_SCREEN_COUNTDOWN &&
     (s_previous.screen!=view.screen || s_previous.countdown!=view.countdown))
    prv_event(QB_AUDIO_TICK,2,tick,ARRAY_LENGTH(tick));
  else if(view.screen!=s_previous.screen) {
    switch(view.screen) {
      case GAME_SCREEN_PLAYING:
        if(s_previous.screen==GAME_SCREEN_COUNTDOWN) prv_event(QB_AUDIO_START,3,start,ARRAY_LENGTH(start));
        break;
      case GAME_SCREEN_CLAIM_PAUSE: prv_event(QB_AUDIO_CLAIM,3,claim,ARRAY_LENGTH(claim)); break;
      case GAME_SCREEN_DEATH: prv_event(QB_AUDIO_HIT,5,hit,ARRAY_LENGTH(hit)); break;
      case GAME_SCREEN_LEVEL_CLEAR: prv_event(QB_AUDIO_CLEAR,4,clear,ARRAY_LENGTH(clear)); break;
      case GAME_SCREEN_GAME_OVER: prv_event(QB_AUDIO_OVER,6,NULL,0); break;
      default: audio_mix_clear_effect(&s_mix); break;
    }
  } else if(view.screen==GAME_SCREEN_PLAYING &&
            (view.moving!=s_previous.moving || view.slow_mode!=s_previous.slow_mode))
    prv_event(QB_AUDIO_UI,1,NULL,0);
  else if(view.screen==GAME_SCREEN_PLAYING &&
          ((view.super_seconds<=5 && s_previous.super_seconds>5) ||
           (view.runners_super && !s_previous.runners_super)))
    prv_event(QB_AUDIO_TICK,2,NULL,0);
  else if(view.screen==GAME_SCREEN_SOUND &&
          (view.music_enabled!=s_previous.music_enabled || view.effects_enabled!=s_previous.effects_enabled ||
           view.volume!=s_previous.volume || view.haptics_enabled!=s_previous.haptics_enabled))
    prv_event(QB_AUDIO_UI,1,NULL,0);
  s_previous=view;
}

void feedback_init(void) {
  const uint32_t ids[QB_AUDIO_CLIP_COUNT]={RESOURCE_ID_AUDIO_MUSIC,RESOURCE_ID_AUDIO_UI,
    RESOURCE_ID_AUDIO_TICK,RESOURCE_ID_AUDIO_START,RESOURCE_ID_AUDIO_CLAIM,
    RESOURCE_ID_AUDIO_HIT,RESOURCE_ID_AUDIO_CLEAR,RESOURCE_ID_AUDIO_OVER};
  uint32_t sizes[QB_AUDIO_CLIP_COUNT];
  for(int i=0;i<QB_AUDIO_CLIP_COUNT;i++) {
    s_resources[i]=resource_get_handle(ids[i]);
    sizes[i]=(uint32_t)resource_size(s_resources[i])/QB_AUDIO_SAMPLE_BYTES;
  }
  audio_mix_init(&s_mix,sizes);
  s_view=s_previous=game_view();
  s_last_ms=prv_ms();
  s_retry_ms=s_last_ms;
  speaker_set_finish_callback(prv_finished,NULL);
  s_timer=app_timer_register(15,prv_pump,NULL);
}

void feedback_deinit(void) {
  if(s_timer) { app_timer_cancel(s_timer); s_timer=NULL; }
  prv_stop_stream();
  speaker_set_finish_callback(NULL,NULL);
  vibes_cancel();
}
