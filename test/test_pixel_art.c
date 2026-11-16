#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pixel_art.h"

typedef struct { int count,landed; unsigned char pixels[QB_SCREEN_H][QB_SCREEN_W]; } Image;

static void collect(int x,int y,bool settled,void *context) {
  Image *image=context;
  assert(x>=0 && x+6<=QB_SCREEN_W && y>=0 && y+7<=119);
  image->count++;
  image->landed+=settled;
  image->pixels[y][x]=1;
}

typedef struct { Image image; int scale; } TextImage;

static void collect_text(int x,int y,bool settled,void *context) {
  TextImage *text=context;
  assert(x>=0 && x+text->scale+1<QB_SCREEN_W);
  assert(y>=QB_HUD_H && y+text->scale+2<QB_SCREEN_H);
  text->image.count++;
  text->image.landed+=settled;
  text->image.pixels[y][x]=1;
}

static void test_status_words(void) {
  const char *words[]={"HIT","CLEAR","GAME OVER","3","2","1"};
  const int scales[]={5,4,3,8,8,8};
  for(int i=0;i<6;i++) {
    TextImage full={.scale=scales[i]},stable={.scale=scales[i]},first={.scale=scales[i]};
    int top=i==2?56:105;
    pixel_text_visit(words[i],100,top,scales[i],30,collect_text,&full);
    pixel_text_visit(words[i],100,top,scales[i],120,collect_text,&stable);
    pixel_text_visit(words[i],100,top,scales[i],0,collect_text,&first);
    assert(full.image.count>=10 && full.image.landed==full.image.count);
    assert(first.image.count<full.image.count);
    assert(memcmp(full.image.pixels,stable.image.pixels,sizeof(full.image.pixels))==0);
    for(int t=0;t<30;t++) {
      TextImage frame={.scale=scales[i]};
      pixel_text_visit(words[i],100,top,scales[i],t,collect_text,&frame);
    }
  }
}

int main(void) {
  Image first={0},full={0},stable={0};
  pixel_logo_visit(0,collect,&first);
  pixel_logo_visit(70,collect,&full);
  pixel_logo_visit(120,collect,&stable);
  assert(full.count>90 && first.count<full.count);
  assert(full.landed==full.count);
  assert(memcmp(full.pixels,stable.pixels,sizeof(full.pixels))==0);
  for(int t=0;t<70;t++) { Image image={0}; pixel_logo_visit(t,collect,&image); }
  test_status_words();
  puts("pixel art ok: logo, transition words and digits, assembly/bounds/stable final glyphs");
  return 0;
}
