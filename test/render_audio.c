#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "audio_mix.h"

static FILE *files[QB_AUDIO_CLIP_COUNT];
static uint32_t read_clip(QbAudioClip clip,uint32_t offset,int16_t *out,uint32_t n,void *context) {
  (void)context;
  assert(fseek(files[clip],offset*QB_AUDIO_SAMPLE_BYTES,SEEK_SET)==0);
  return (uint32_t)fread(out,QB_AUDIO_SAMPLE_BYTES,n,files[clip]);
}

int main(int argc,char **argv) {
  if(argc!=3) return 1;
  const char *names[]={"music","ui","tick","start","claim","hit","clear","over"};
  uint32_t sizes[QB_AUDIO_CLIP_COUNT];
  for(int i=0;i<QB_AUDIO_CLIP_COUNT;i++) {
    char path[1024]; snprintf(path,sizeof(path),"%s/%s.pcm",argv[1],names[i]);
    files[i]=fopen(path,"rb"); if(!files[i]) return 2;
    fseek(files[i],0,SEEK_END); sizes[i]=(uint32_t)ftell(files[i])/QB_AUDIO_SAMPLE_BYTES;
  }
  FILE *out=fopen(argv[2],"wb"); if(!out) return 3;
  QbAudioMix mix; audio_mix_init(&mix,sizes);
  int16_t block[QB_AUDIO_BLOCK];
  for(int sample=0;sample<20*8000;sample+=256) {
    if(sample==0 || sample==8192 || sample==16384) audio_mix_effect(&mix,QB_AUDIO_TICK,2);
    if(sample==24576) { mix.music=true; audio_mix_effect(&mix,QB_AUDIO_START,3); }
    if(sample==65536) audio_mix_effect(&mix,QB_AUDIO_CLAIM,3);
    if(sample==98304) audio_mix_effect(&mix,QB_AUDIO_UI,1);
    if(sample==122880) { mix.music=false; audio_mix_effect(&mix,QB_AUDIO_HIT,5); }
    if(sample==135168) audio_mix_effect(&mix,QB_AUDIO_OVER,6);
    audio_mix_render(&mix,block,QB_AUDIO_BLOCK,read_clip,NULL);
    fwrite(block,1,sizeof(block),out);
  }
  fclose(out);
  for(int i=0;i<QB_AUDIO_CLIP_COUNT;i++) fclose(files[i]);
}
