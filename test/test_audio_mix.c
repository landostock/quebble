#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "audio_mix.h"

static uint32_t sizes[QB_AUDIO_CLIP_COUNT]={7,50,90,200,300,400,500,600};
static uint32_t read_clip(QbAudioClip clip,uint32_t offset,int16_t *out,uint32_t count,void *context) {
  (void)context;
  assert(offset+count<=sizes[clip]);
  for(uint32_t i=0;i<count;i++) out[i]=clip==QB_AUDIO_MUSIC?((int)(offset+i)*20-60)*256:25600;
  return count;
}

int main(void) {
  QbAudioMix a,b;
  audio_mix_init(&a,sizes);
  assert(!audio_mix_active(&a));
  a.music=true;
  b=a;
  int16_t whole[1024],pieces[1024];
  audio_mix_render(&a,whole,1024,read_clip,NULL);
  for(int i=0;i<1024;i+=16) audio_mix_render(&b,pieces+i,16,read_clip,NULL);
  assert(memcmp(whole,pieces,sizeof(whole))==0);
  assert(a.music_pos==1024%7 && a.music_gain==40);
  assert(audio_mix_effect(&a,QB_AUDIO_HIT,5));
  assert(!audio_mix_effect(&a,QB_AUDIO_UI,1));
  assert(a.fx==QB_AUDIO_HIT);
  b=a;
  audio_mix_render(&a,whole,1024,read_clip,NULL);
  for(int i=0;i<1024;i+=16) audio_mix_render(&b,pieces+i,16,read_clip,NULL);
  assert(memcmp(whole,pieces,sizeof(whole))==0);
  assert(a.fx==QB_AUDIO_CLIP_COUNT && a.music_gain==40);
  a.music=false;
  audio_mix_render(&a,whole,1024,read_clip,NULL);
  assert(!audio_mix_active(&a));
  for(int i=256;i<1024;i++) assert(whole[i]==0);
  assert(!audio_mix_effect(&a,QB_AUDIO_MUSIC,1));
  assert(!audio_mix_effect(&a,QB_AUDIO_CLIP_COUNT,1));
  assert(audio_mix_effect(&a,QB_AUDIO_UI,1));
  audio_mix_render(&a,whole,50,read_clip,NULL);
  for(int i=0;i<50;i++) assert(whole[i]==12800);
  puts("audio mix ok: looping, chunk independence, ducking, priorities, fades, bounds");
}
