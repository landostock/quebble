#include <assert.h>
#include <stdio.h>
/* Include the implementation to exercise private movement without a test-only watch API. */
#include "../src/c/game.c"

static void test_connect_to_existing_area(QbCellState fill) {
  game_init();
  prv_new_game();
  s_game.screen = GAME_SCREEN_PLAYING;
  s_game.enemies.rift_count = 1;
  const int bottom=QB_GRID_H*3/4;
  for (int y = bottom; y < QB_GRID_H - 1; y++)
    for (int x = 1; x < QB_GRID_W - 1; x++) grid_test_set(x, y, fill);
  grid_test_set(25, bottom-1, CELL_BORDER);
  grid_test_set(29, bottom-2, CELL_BORDER);
  s_game.player = (QbCell){25, bottom};
  s_game.dir = QB_DIR_UP;
  s_game.moving = true;
  prv_move_player();
  assert(!s_game.drawing);
  game_set_direction(QB_DIR_UP);
  for (int i = 0; i < 6; i++) prv_move_player();
  assert(s_game.drawing);
  game_turn_right(false);
  for (int i = 0; i < 8; i++) prv_move_player();
  game_turn_right(false);
  for (int i = 0; i < 7 && s_game.drawing; i++) prv_move_player();
  assert(!s_game.drawing);
  assert(s_game.player.x == 33 && s_game.player.y == bottom);
  assert(grid_get(25, bottom-1) == CELL_BORDER);
  assert(grid_get(33, bottom-1) == CELL_BORDER);
  assert(grid_get(33, bottom) == fill);
  assert(grid_get(29, bottom-2) == CELL_BORDER);
  assert(s_game.claim_cells > 0);
  assert(grid_is_player_path(s_game.player.x, s_game.player.y));
  assert(game_view().screen == GAME_SCREEN_CLAIM_PAUSE);
}

static void test_last_empty_neighbor_becomes_trail(void) {
  game_init();
  prv_new_game();
  s_game.enemies.rift_count = 0;
  s_game.screen = GAME_SCREEN_PLAYING;
  s_game.drawing = true;
  s_game.moving = true;
  s_game.dir = QB_DIR_DOWN;
  const int x=QB_GRID_W/2,y=QB_GRID_H/2;
  s_game.player = (QbCell){x, y-1};
  s_game.trail_len = 4;
  for (int row = y-4; row < y; row++) grid_set_trail(x, row, false);
  for(int row=y-1;row<=y+1;row++) for(int col=x-1;col<=x+1;col++)
    if(col!=x || row>=y) grid_test_set(col,row,CELL_CLAIM_FAST);
  grid_test_set(x,y,CELL_BORDER);
  assert(!grid_is_player_path(x, y));
  assert(prv_can_step(QB_DIR_DOWN));
  prv_move_player();
  assert(!s_game.drawing);
  assert(grid_is_player_path(s_game.player.x,s_game.player.y));
  assert(s_game.screen != GAME_SCREEN_DEATH);
}

static void test_buried_marker_three_rectangles(void) {
  game_init();
  prv_new_game();
  s_game.enemies.rift_count=0;
  s_game.enemies.runner_count=0;
  s_game.screen=GAME_SCREEN_PLAYING;
  const int left=QB_GRID_W/2-1,seam=QB_GRID_W*2/3,top=QB_GRID_H/2,bottom=QB_GRID_H*3/4;
  for(int y=bottom;y<QB_GRID_H-1;y++) for(int x=left+1;x<QB_GRID_W-1;x++) grid_test_set(x,y,CELL_CLAIM_FAST);
  for(int y=top;y<bottom;y++) for(int x=seam;x<QB_GRID_W-1;x++) grid_test_set(x,y,CELL_CLAIM_FAST);
  for(int y=top-1;y<bottom;y++) for(int x=left;x<=seam;x++) grid_test_set(x,y,CELL_CLAIM_FAST);
  for(int y=top;y<bottom;y++) grid_test_set(seam,y,CELL_BORDER);
  const QbCell buried={seam,(top+bottom)/2};
  s_game.player=buried;
  s_game.dir=QB_DIR_UP;
  s_game.moving=false;
  assert(!grid_is_player_path(buried.x,buried.y));
  game_stop_go();
  for(int turn=0;turn<4 && !s_game.moving;turn++) game_turn_left(false);
  assert(s_game.moving);
  assert(grid_is_player_path(s_game.player.x,s_game.player.y));
  assert(grid_get(buried.x,buried.y)==CELL_BORDER);
  QbCell before=s_game.player;
  prv_move_player();
  assert(s_game.player.x!=before.x || s_game.player.y!=before.y);
  assert(!s_game.drawing && grid_is_player_path(s_game.player.x,s_game.player.y));

  s_game.player=buried;
  s_game.drawing=false;
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE;
  s_game.screen_ticks=1;
  game_update();
  assert(s_game.screen==GAME_SCREEN_PLAYING);
  assert(grid_is_player_path(s_game.player.x,s_game.player.y));
  for(int turn=0;turn<4 && !s_game.moving;turn++) game_turn_left(false);
  assert(s_game.moving);

  s_game.player=buried;
  s_game.screen=GAME_SCREEN_PLAYING;
  s_game.drawing=true;
  s_game.trail_len=4;
  s_game.enemies.rift_count=1;
  QbRift *r=&s_game.enemies.rifts[0];
  r->x_fp=r->y_fp=5*QB_FP_ONE+8;
  for(int i=0;i<QB_RIFT_LINES;i++) r->strokes[i]=(QbStroke){r->x_fp-8,r->y_fp,r->x_fp+8,r->y_fp};
  prv_close_trail();
  assert(s_game.screen==GAME_SCREEN_CLAIM_PAUSE);
  assert(grid_is_player_path(s_game.player.x,s_game.player.y));
  assert(grid_get(buried.x,buried.y)==CELL_BORDER);
}

static void test_help_and_light_lifecycle(void) {
  game_init();
  assert(!game_backlight_required());
  game_touch_tap(100,194);
  assert(game_view().screen==GAME_SCREEN_DIFFICULTY && !game_backlight_required());
  game_stop_go();
  assert(game_view().screen==GAME_SCREEN_COUNTDOWN);
  game_back_to_title();
  game_turn_left(false);
  assert(game_view().screen==GAME_SCREEN_HELP && !game_backlight_required());
  for(int page=1;page<QB_GUIDE_PAGES;page++) {
    game_stop_go();
    assert(game_view().screen==GAME_SCREEN_HELP && game_view().help_page==page);
  }
  game_stop_go();
  assert(game_view().screen==GAME_SCREEN_DIFFICULTY);
  game_stop_go();
  assert(game_view().screen==GAME_SCREEN_COUNTDOWN && game_backlight_required());
  const QbGameScreen active[]={GAME_SCREEN_PLAYING,GAME_SCREEN_CLAIM_PAUSE,GAME_SCREEN_DEATH,GAME_SCREEN_LEVEL_CLEAR};
  for(unsigned i=0;i<ARRAY_LENGTH(active);i++) { s_game.screen=active[i]; assert(game_backlight_required()); }
  game_back_to_title();
  assert(!game_backlight_required());
  game_touch_tap(20,210);
  assert(game_view().screen==GAME_SCREEN_HELP);
  game_touch_tap(20,210);
  assert(game_view().screen==GAME_SCREEN_TITLE);
  game_turn_left(false);
  for(int page=0;page<QB_GUIDE_PAGES;page++) game_touch_tap(170,214);
  assert(game_view().screen==GAME_SCREEN_DIFFICULTY);
  game_touch_tap(100,120);
  assert(game_view().screen==GAME_SCREEN_COUNTDOWN && game_backlight_required());
  s_game.screen=GAME_SCREEN_PLAYING;
  game_toggle_slow();
  game_turn_left(false);
  assert(s_game.slow_mode);
  game_toggle_slow();
  assert(!s_game.slow_mode);
  s_game.enemies.super_ticks_remaining=1;
  assert(game_view().super_seconds==1);
  s_game.enemies.super_ticks_remaining=30;
  assert(game_view().super_seconds==1);
  s_game.enemies.super_ticks_remaining=31;
  assert(game_view().super_seconds==2);
  s_game.screen=GAME_SCREEN_GAME_OVER;
  assert(game_backlight_required());
  game_touch_tap(100,210);
  assert(game_view().screen==GAME_SCREEN_TITLE && !game_backlight_required());
}

static void test_final_hit_and_results(void) {
  game_init();
  prv_new_game();
  s_game.screen=GAME_SCREEN_PLAYING;
  s_game.lives=1;
  s_game.score=1200;
  s_game.menu_ticks=120;
  prv_die();
  assert(s_game.screen==GAME_SCREEN_DEATH && s_game.lives==0);
  assert(game_view().transition_ticks==0 && game_backlight_required());
  for(int tick=1;tick<QB_DEATH_CARD_TICKS;tick++) {
    game_update();
    assert(s_game.screen==GAME_SCREEN_DEATH);
    assert(game_view().transition_ticks==tick && game_backlight_required());
  }
  game_update();
  assert(s_game.screen==GAME_SCREEN_GAME_OVER && game_view().menu_ticks==0);
  for(int tick=0;tick<900;tick++) {
    game_update();
    assert(s_game.screen==GAME_SCREEN_GAME_OVER && game_backlight_required());
    assert(game_view().score==1200);
  }
  game_stop_go();
  assert(s_game.screen==GAME_SCREEN_COUNTDOWN && s_game.lives==3 && s_game.score==0);
}

static void test_level_progression(void) {
  game_init();
  prv_new_game();
  for(int level=1;level<=UINT8_MAX;level++) {
    s_game.level=(uint8_t)level;
    prv_begin_level();
    assert(s_game.target_percent==75);
    assert(s_game.enemies.rift_count==(level>=3?2:1));
    assert(s_game.enemies.aggression==((level==1 || level==3)?0:level<5?1:level<7?2:3));
  }
  prv_level_clear(false);
  s_game.screen_ticks=1;
  game_update();
  assert(s_game.level==UINT8_MAX && s_game.target_percent==75);
}

static void test_countdown_and_pause(void) {
  game_init();
  game_stop_go();
  game_stop_go();
  const QbEnemies before = s_game.enemies;
  for (int i = 0; i < 3 * (1000 / QB_TIMER_MS); i++) {
    assert(game_view().transition_ticks==i%(1000/QB_TIMER_MS));
    assert(game_view().countdown==3-i/(1000/QB_TIMER_MS));
    game_update();
    assert(memcmp(&before, &s_game.enemies, sizeof(before)) == 0);
  }
  assert(game_view().screen == GAME_SCREEN_PLAYING);
  assert(game_view().moving && s_game.dir == QB_DIR_UP);
  s_game.screen = GAME_SCREEN_CLAIM_PAUSE;
  s_game.screen_ticks = 1000 / QB_TIMER_MS;
  s_game.moving = false;
  const QbCell player = s_game.player;
  for (int i = 0; i < 1000 / QB_TIMER_MS; i++) {
    game_update();
    assert(memcmp(&before, &s_game.enemies, sizeof(before)) == 0);
    assert(memcmp(&player, &s_game.player, sizeof(player)) == 0);
  }
  assert(game_view().screen == GAME_SCREEN_PLAYING);
}

static void test_sound_controls(void) {
  game_init();
  assert(game_view().music_enabled && game_view().effects_enabled && game_view().haptics_enabled);
  assert(game_view().volume==50);
  game_turn_right(false);
  assert(game_view().screen==GAME_SCREEN_SOUND && !game_backlight_required());
  game_stop_go(); assert(!game_view().music_enabled);
  game_turn_right(false); game_stop_go(); assert(!game_view().effects_enabled);
  game_turn_right(false); game_stop_go(); assert(!game_view().haptics_enabled);
  game_turn_right(false); game_stop_go(); assert(game_view().volume==75);
  game_stop_go(); assert(game_view().volume==100);
  game_stop_go(); assert(game_view().volume==0);
  game_stop_go(); assert(game_view().volume==25);
  game_turn_right(false); game_stop_go();
  assert(game_view().screen==GAME_SCREEN_CREDITS && !game_backlight_required());
  game_back_to_title(); assert(game_view().screen==GAME_SCREEN_SOUND);
  game_back_to_title(); assert(game_view().screen==GAME_SCREEN_TITLE);
  game_touch_tap(170,211); assert(game_view().screen==GAME_SCREEN_SOUND);
  game_touch_tap(175,195); assert(game_view().screen==GAME_SCREEN_CREDITS);
  game_touch_tap(100,211); assert(game_view().screen==GAME_SCREEN_SOUND);
  game_back_to_title(); game_stop_go();
  assert(game_view().screen==GAME_SCREEN_DIFFICULTY);
  game_stop_go();
  assert(game_view().screen==GAME_SCREEN_COUNTDOWN);
  assert(!game_view().music_enabled && !game_view().effects_enabled && !game_view().haptics_enabled);
}

static void test_difficulties(void) {
  const int targets[]={65,75,80},lives[]={4,3,2},seconds[]={45,30,22};
  const int double_level[]={5,3,1},four_level[]={7,5,3},super_level[]={9,7,5};
  for(int mode=0;mode<QB_DIFFICULTY_COUNT;mode++) {
    game_init();
    s_game.screen=GAME_SCREEN_DIFFICULTY;
    prv_select_difficulty((QbDifficulty)mode);
    game_stop_go();
    assert(s_game.lives==lives[mode] && s_game.target_percent==targets[mode]);
    for(int level=1;level<=12;level++) {
      s_game.level=(uint8_t)level;
      prv_begin_level();
      assert(s_game.enemies.rift_count==(level>=double_level[mode]?2:1));
      assert(s_game.enemies.runner_count==(level>=four_level[mode]?4:2));
      assert(s_game.enemies.runners_super==(level>=super_level[mode]));
      assert(s_game.enemies.reinforcement_ticks==seconds[mode]*(1000/QB_TIMER_MS));
      if(!s_game.enemies.runners_super)
        assert(s_game.enemies.super_ticks_remaining==seconds[mode]*(1000/QB_TIMER_MS));
    }
  }
  game_init(); game_stop_go();
  game_turn_left(false); assert(s_game.difficulty==QB_EASY);
  game_turn_left(false); assert(s_game.difficulty==QB_EXPERT);
  game_turn_right(false); assert(s_game.difficulty==QB_EASY);
  game_touch_tap(100,120);
  assert(s_game.difficulty==QB_STANDARD && s_game.screen==GAME_SCREEN_COUNTDOWN);
}

static void test_guide_and_relative_buttons(void) {
  game_init(); game_turn_left(false);
  assert(game_view().help_page==0);
  game_turn_left(false); assert(game_view().help_page==0);
  game_turn_right(false); assert(game_view().help_page==1);
  game_set_direction(QB_DIR_LEFT); assert(game_view().help_page==2);
  game_set_direction(QB_DIR_LEFT); assert(game_view().help_page==QB_GUIDE_PAGES-1);
  game_set_direction(QB_DIR_RIGHT); assert(game_view().help_page==1);
  game_set_direction(QB_DIR_DOWN); assert(game_view().help_page==0);
  game_toggle_slow(); assert(game_view().help_page==1 && !s_game.slow_mode);
  game_back_to_title(); game_turn_left(false);
  assert(game_view().help_page==0 && !game_backlight_required());
  game_touch_tap(100,100); assert(game_view().help_page==0);
  game_back_to_title(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  s_game.drawing=true; s_game.player=(QbCell){32,30};
  const QbDir facing[]={QB_DIR_UP,QB_DIR_RIGHT,QB_DIR_DOWN,QB_DIR_LEFT};
  const QbDir left[]={QB_DIR_LEFT,QB_DIR_UP,QB_DIR_RIGHT,QB_DIR_DOWN};
  const QbDir right[]={QB_DIR_RIGHT,QB_DIR_DOWN,QB_DIR_LEFT,QB_DIR_UP};
  for(int i=0;i<4;i++) {
    s_game.arrival_dir=QB_DIR_NONE;
    s_game.dir=facing[i]; game_turn_left(false); assert(s_game.dir==left[i]);
    s_game.arrival_dir=QB_DIR_NONE;
    s_game.dir=facing[i]; game_turn_right(false); assert(s_game.dir==right[i]);
  }
  s_game.drawing=false; s_game.player=(QbCell){0,20};
  for(int x=1;x<10;x++) grid_test_set(x,20,CELL_BORDER);
  s_game.dir=QB_DIR_LEFT;
  s_game.arrival_dir=QB_DIR_LEFT;
  game_turn_left(false); assert(s_game.dir==QB_DIR_DOWN);
  s_game.dir=QB_DIR_LEFT;
  game_turn_right(false); assert(s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(!s_game.moving && s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(s_game.moving && s_game.dir==QB_DIR_UP);
  prv_move_player(); assert(s_game.player.x==0 && s_game.player.y==19 && !s_game.drawing);
}

static void test_absolute_controls_and_coasting(void) {
  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  game_set_direction(QB_DIR_RIGHT);
  assert(s_game.dir==QB_DIR_RIGHT && s_game.moving && s_game.edge_follow);
  game_set_direction(QB_DIR_RIGHT);
  assert(s_game.dir==QB_DIR_RIGHT);
  game_set_direction(QB_DIR_DOWN); /* Outside the board: do not stop or reverse. */
  assert(s_game.dir==QB_DIR_RIGHT && s_game.moving);
  s_game.player=(QbCell){QB_GRID_W-1,QB_GRID_H-1};
  s_game.arrival_dir=QB_DIR_RIGHT;
  s_game.edge_choice=QB_DIR_NONE;
  prv_move_player();
  assert(s_game.dir==QB_DIR_UP && s_game.player.y==QB_GRID_H-2);
  s_game.player=(QbCell){QB_GRID_W/2,0};
  s_game.arrival_dir=s_game.dir=QB_DIR_UP; s_game.moving=true;
  s_game.edge_choice=QB_DIR_NONE;
  prv_move_player(); /* Ambiguous left/right: await the player's choice. */
  assert(!s_game.moving && s_game.player.x==QB_GRID_W/2);
  assert(s_game.dir==QB_DIR_UP && game_view().choosing_path);
  game_set_direction(QB_DIR_LEFT); assert(s_game.moving && s_game.dir==QB_DIR_LEFT);
  prv_move_player(); assert(s_game.player.x==QB_GRID_W/2-1);
  game_set_direction(QB_DIR_DOWN); assert(!s_game.edge_follow);
  prv_move_player(); assert(s_game.drawing);
  game_set_direction(QB_DIR_DOWN); prv_move_player();
  game_set_direction(QB_DIR_UP); /* Do not backtrack the exposed trail. */
  assert(s_game.dir==QB_DIR_DOWN && s_game.moving);
  game_set_direction(QB_DIR_RIGHT);
  assert(s_game.dir==QB_DIR_RIGHT && s_game.moving);

  grid_init_level();
  s_game.player=(QbCell){QB_GRID_W/2,0}; s_game.drawing=false;
  s_game.dir=QB_DIR_UP; s_game.screen=GAME_SCREEN_CLAIM_PAUSE;
  s_game.screen_ticks=1; s_game.pending_action=PENDING_NONE;
  s_game.arrival_dir=QB_DIR_UP; s_game.edge_choice=QB_DIR_NONE;
  game_update(); assert(!s_game.moving);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_set_direction(QB_DIR_RIGHT);
  assert(s_game.pending_action==PENDING_RIGHT);
  game_update(); assert(s_game.moving && s_game.dir==QB_DIR_RIGHT);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  s_game.dir=QB_DIR_UP;
  s_game.edge_choice=QB_DIR_NONE;
  game_tap(); game_update(); /* No safe way back on this artificial border fixture. */
  assert(!s_game.moving);
  game_turn_right(false); assert(s_game.moving && s_game.dir==QB_DIR_RIGHT);
}

int main(void) {
  test_connect_to_existing_area(CELL_CLAIM_FAST);
  test_connect_to_existing_area(CELL_CLAIM_SLOW);
  test_last_empty_neighbor_becomes_trail();
  test_countdown_and_pause();
  test_buried_marker_three_rectangles();
  test_help_and_light_lifecycle();
  test_final_hit_and_results();
  test_level_progression();
  test_sound_controls();
  test_difficulties();
  test_absolute_controls_and_coasting();
  test_guide_and_relative_buttons();
  puts("game ok: connections, countdown/freeze, final hit/results/retry/backlight, level progression");
  return 0;
}
