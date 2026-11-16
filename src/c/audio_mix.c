#include "audio_mix.h"
#include <string.h>

void audio_mix_init(QbAudioMix *mix,const uint32_t sizes[QB_AUDIO_CLIP_COUNT]) {
  memset(mix,0,sizeof(*mix));
  memcpy(mix->sizes,sizes,sizeof(mix->sizes));
  mix->fx=QB_AUDIO_CLIP_COUNT;
}

bool audio_mix_effect(QbAudioMix *mix,QbAudioClip clip,uint8_t priority) {
  if(clip<=QB_AUDIO_MUSIC || clip>=QB_AUDIO_CLIP_COUNT || !mix->sizes[clip]) return false;
  if(mix->fx<QB_AUDIO_CLIP_COUNT && mix->priority>priority) return false;
  mix->fx=clip;
  mix->fx_pos=0;
  mix->priority=priority;
  return true;
}

void audio_mix_clear_effect(QbAudioMix *mix) {
  mix->fx=QB_AUDIO_CLIP_COUNT;
  mix->fx_pos=0;
  mix->priority=0;
}

bool audio_mix_active(const QbAudioMix *mix) {
  return (mix->music && mix->sizes[QB_AUDIO_MUSIC]) || mix->music_gain || mix->fx<QB_AUDIO_CLIP_COUNT;
}

void audio_mix_render(QbAudioMix *mix,int16_t *out,uint32_t count,QbAudioRead read,void *context) {
  int16_t *bg=mix->background,*fx=mix->effect;
  while(count) {
    uint32_t n=count>QB_AUDIO_BLOCK?QB_AUDIO_BLOCK:count;
    memset(bg,0,n*sizeof(*bg));
    memset(fx,0,n*sizeof(*fx));
    if((mix->music || mix->music_gain) && mix->sizes[QB_AUDIO_MUSIC]) {
      uint32_t done=0;
      while(done<n) {
        uint32_t chunk=mix->sizes[QB_AUDIO_MUSIC]-mix->music_pos;
        if(chunk>n-done) chunk=n-done;
        if(read(QB_AUDIO_MUSIC,mix->music_pos,bg+done,chunk,context)!=chunk) break;
        done+=chunk;
        mix->music_pos=(mix->music_pos+chunk)%mix->sizes[QB_AUDIO_MUSIC];
      }
    }
    bool effect=mix->fx<QB_AUDIO_CLIP_COUNT;
    uint32_t effect_count=0;
    if(effect) {
      uint32_t chunk=mix->sizes[mix->fx]-mix->fx_pos;
      if(chunk>n) chunk=n;
      uint32_t got=read(mix->fx,mix->fx_pos,fx,chunk,context);
      effect_count=got;
      mix->fx_pos+=got;
      if(got!=chunk || mix->fx_pos==mix->sizes[mix->fx]) audio_mix_clear_effect(mix);
    }
    for(uint32_t i=0;i<n;i++) {
      const int target=mix->music?(i<effect_count?12:40):0;
      if(++mix->gain_tick==8) {
        mix->gain_tick=0;
        if(mix->music_gain<target) mix->music_gain++;
        if(mix->music_gain>target) {
          int drop=mix->music_gain-target;
          mix->music_gain-=(uint8_t)(drop>4?4:drop);
        }
      }
      int sample=bg[i]*mix->music_gain/100+fx[i]/2;
      if(sample>32767) sample=32767;
      if(sample<-32768) sample=-32768;
      out[i]=(int16_t)sample;
    }
    out+=n;
    count-=n;
  }
}
