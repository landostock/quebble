#include <stdio.h>
#include <string.h>
#include "enemies.h"

static unsigned char pixels[48][96][3];
typedef struct { int x,y; QbPixel center; } Draw;

static void pixel(int x,int y,uint8_t ink,void *context) {
  Draw *draw=context;
  x+=draw->x-draw->center.x;
  y+=draw->y-draw->center.y;
  if(x<0 || y<0 || x>=96 || y>=48) return;
  pixels[y][x][0]=255;
  pixels[y][x][1]=ink?0:255;
  pixels[y][x][2]=0;
}

int main(int argc,char **argv) {
  if(argc!=2) return 1;
  grid_init_level();
  QbEnemies enemies;
  enemies_init_seed(&enemies,1,71);
  static const uint8_t ticks[8]={0,2,5,7,9,12,14,16};
  memset(pixels,0,sizeof(pixels));
  for(int phase=0;phase<8;phase++) {
    Draw draw={12+(phase%4)*24,12+(phase/4)*24,enemies_get_runner_center(&enemies,0)};
    for(int y=draw.y-1;y<=draw.y+1;y++)
      for(int x=draw.x-12;x<draw.x+12;x++) memset(pixels[y][x],255,3);
    enemies.anim=ticks[phase];
    enemies_visit_runner_colored_pixels(&enemies,0,pixel,&draw);
  }
  FILE *out=fopen(argv[1],"wb");
  if(!out) return 2;
  fprintf(out,"P6\n96 48\n255\n");
  fwrite(pixels,1,sizeof(pixels),out);
  fclose(out);
  return 0;
}
