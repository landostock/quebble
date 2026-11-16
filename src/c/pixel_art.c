#include "pixel_art.h"

#include <string.h>

static const uint8_t *prv_glyph(char c) {
  static const uint8_t glyphs[36][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {15,16,16,16,16,16,15}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {15,16,16,23,17,17,15}, {17,17,17,31,17,17,17},
    {31,4,4,4,4,4,31}, {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,27,17}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
  };
  if(c>='A' && c<='Z') return glyphs[c-'A'];
  if(c>='0' && c<='9') return glyphs[26+c-'0'];
  return NULL;
}

void pixel_text_visit(const char *text,int center_x,int top,int scale,uint16_t tick,
                      QbLogoVisitor visitor,void *context) {
  const int count=(int)strlen(text);
  if(!count || scale<1) return;
  const int left=center_x-(count*6-1)*scale/2;
  for(int letter=0;letter<count;letter++) {
    const uint8_t *glyph=prv_glyph(text[letter]);
    if(!glyph) continue;
    for(int row=0;row<7;row++) for(int col=0;col<5;col++) {
      if(!(glyph[row] & (16U>>col))) continue;
      const unsigned seed=letter*35+row*17+col*11;
      const int age=(int)tick-(int)(seed%7);
      if(age<0) continue;
      const int remaining=age<9?9-age:0;
      const int x=left+(letter*6+col)*scale;
      const int y=top+row*scale;
      visitor(x+((int)(seed*17%41)-20)*remaining*remaining/81,
              y+((int)(seed*23%29)-14)*remaining*remaining/81,remaining==0,context);
    }
  }
}

void pixel_logo_visit(uint16_t tick, QbLogoVisitor visitor, void *context) {
  static const uint8_t letters[7][7] = {
    {14,17,17,17,21,18,13},
    {17,17,17,17,17,17,14},
    {31,16,16,30,16,16,31},
    {30,17,17,30,17,17,30},
    {30,17,17,30,17,17,30},
    {16,16,16,16,16,16,31},
    {31,16,16,30,16,16,31}
  };
  for (int letter = 0; letter < 7; letter++) {
    for (int row = 0; row < 7; row++) {
      for (int col = 0; col < 5; col++) {
        if (!(letters[letter][row] & (16U >> col))) continue;
        const unsigned seed = letter * 35 + row * 17 + col * 11;
        const int age = (int)tick - (int)(seed % 40);
        if (age < 0) continue;
        const int target_x = 14 + letter * 24 + col * 4 + 6-row;
        const int target_y = 68 + row * 4;
        const int remaining = age < 14 ? 14 - age : 0;
        const int x = target_x + ((int)(4 + seed * 17 % 188) - target_x) * remaining * remaining / 196;
        const int y = target_y + ((int)(18 + seed * 23 % 95) - target_y) * remaining * remaining / 196;
        visitor(x, y, remaining == 0, context);
      }
    }
  }
}
