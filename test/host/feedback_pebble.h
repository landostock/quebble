#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#define ARRAY_LENGTH(a) (sizeof(a)/sizeof((a)[0]))
typedef struct { const uint32_t *durations; unsigned num_segments; } VibePattern;
typedef struct AppTimer AppTimer;
typedef int ResHandle;
typedef enum { SpeakerFinishReasonDone,SpeakerFinishReasonStopped,SpeakerFinishReasonPreempted,SpeakerFinishReasonError } SpeakerFinishReason;
typedef enum { SpeakerStatusIdle,SpeakerStatusPlaying,SpeakerStatusDraining } SpeakerStatus;
typedef enum { SpeakerPcmFormat_8kHz_16bit } SpeakerPcmFormat;
typedef void (*SpeakerFinishedCallback)(SpeakerFinishReason,void *);
uint16_t time_ms(time_t *,uint16_t *);
AppTimer *app_timer_register(uint32_t,void (*)(void *),void *);
bool app_timer_cancel(AppTimer *);
void speaker_stop(void);
void speaker_set_finish_callback(SpeakerFinishedCallback,void *);
bool speaker_is_muted(void);
bool speaker_stream_open(SpeakerPcmFormat,uint8_t);
uint32_t speaker_stream_write(const void *,uint32_t);
void speaker_stream_close(void);
SpeakerStatus speaker_get_status(void);
void speaker_set_volume(uint8_t);
ResHandle resource_get_handle(uint32_t);
size_t resource_size(ResHandle);
size_t resource_load_byte_range(ResHandle,uint32_t,uint8_t *,size_t);
bool quiet_time_is_active(void);
void vibes_cancel(void);
void vibes_enqueue_custom_pattern(VibePattern);
enum { RESOURCE_ID_AUDIO_MUSIC=1,RESOURCE_ID_AUDIO_UI,RESOURCE_ID_AUDIO_TICK,
  RESOURCE_ID_AUDIO_START,RESOURCE_ID_AUDIO_CLAIM,RESOURCE_ID_AUDIO_HIT,
  RESOURCE_ID_AUDIO_CLEAR,RESOURCE_ID_AUDIO_OVER };
