#define QUEBBLE_PERSIST_TEST
#include <assert.h>
#include <stdio.h>
#include "../src/c/game.c"

static int32_t values[6];
static bool present[6];
bool persist_exists(int key) { assert(key>=1001 && key<=1005); return present[key-1000]; }
int32_t persist_read_int(int key) { assert(persist_exists(key)); return values[key-1000]; }
void persist_write_int(int key,int32_t v) { assert(key>=1001 && key<=1005); present[key-1000]=true; values[key-1000]=v; }
int main(void) {
  game_init(); assert(s_game.difficulty==QB_STANDARD && s_game.volume==50);
  prv_new_game(); s_game.score=400; prv_save_highscore(); game_back_to_title();
  game_stop_go(); game_turn_left(false); /* Select Easy without starting a run. */
  game_deinit();
  assert(!present[4] && values[1]==400);
  game_init(); assert(s_game.difficulty==QB_EASY && s_game.highscore==0);
  prv_new_game(); s_game.score=120; game_back_to_title();
  game_stop_go(); game_turn_right(false);
  assert(s_game.difficulty==QB_STANDARD && s_game.highscore==400);
  game_turn_right(false); assert(s_game.highscore==0);
  prv_new_game(); s_game.score=60; game_back_to_title();
  assert(values[5]==60 && values[4]==120 && values[1]==400);
  s_game.screen=GAME_SCREEN_SOUND; s_game.sound_row=3;
  game_stop_go(); assert(s_game.volume==75);
  game_stop_go(); assert(s_game.volume==100);
  game_init(); assert(s_game.volume==100);
  s_game.screen=GAME_SCREEN_SOUND; s_game.sound_row=0; game_stop_go();
  game_init(); assert(!s_game.music_enabled && s_game.effects_enabled);
  values[2]=UINT32_MAX; values[3]=99;
  game_init(); assert(s_game.volume==50 && s_game.difficulty==QB_STANDARD);
  puts("preferences ok: separate mode scores, no cross-mode score leak, sound persistence, validation");
}
