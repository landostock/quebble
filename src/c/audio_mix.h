#ifndef AUDIO_MIX_H
#define AUDIO_MIX_H

#include <stdbool.h>
#include <stdint.h>

#define QB_AUDIO_BLOCK 256
#define QB_AUDIO_SAMPLE_BYTES 2
#define QB_AUDIO_BLOCK_BYTES (QB_AUDIO_BLOCK * QB_AUDIO_SAMPLE_BYTES)
typedef enum {
  QB_AUDIO_MUSIC, QB_AUDIO_UI, QB_AUDIO_TICK, QB_AUDIO_START, QB_AUDIO_CLAIM,
  QB_AUDIO_HIT, QB_AUDIO_CLEAR, QB_AUDIO_OVER, QB_AUDIO_CLIP_COUNT
} QbAudioClip;

/* Resource offsets and lengths are in samples; speaker writes are in bytes. */
typedef uint32_t (*QbAudioRead)(QbAudioClip clip,uint32_t offset,int16_t *out,uint32_t count,void *context);
typedef struct {
  uint32_t sizes[QB_AUDIO_CLIP_COUNT];
  uint32_t music_pos,fx_pos;
  QbAudioClip fx;
  uint8_t priority,music_gain,gain_tick;
  bool music;
  int16_t background[QB_AUDIO_BLOCK],effect[QB_AUDIO_BLOCK];
} QbAudioMix;

void audio_mix_init(QbAudioMix *mix,const uint32_t sizes[QB_AUDIO_CLIP_COUNT]);
bool audio_mix_effect(QbAudioMix *mix,QbAudioClip clip,uint8_t priority);
void audio_mix_clear_effect(QbAudioMix *mix);
bool audio_mix_active(const QbAudioMix *mix);
void audio_mix_render(QbAudioMix *mix,int16_t *out,uint32_t count,QbAudioRead read,void *context);

#endif
