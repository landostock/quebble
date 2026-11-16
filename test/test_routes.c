#include <assert.h>
#include <stdio.h>
#include "../src/c/game.c"

static int dx(QbDir d) { return d==QB_DIR_RIGHT?1:d==QB_DIR_LEFT?-1:0; }
static int dy(QbDir d) { return d==QB_DIR_DOWN?1:d==QB_DIR_UP?-1:0; }
static unsigned bit(QbDir d) { return 1U<<(d-1); }

static void fixture(unsigned mask,QbDir arrival,bool touch) {
  game_init(); prv_new_game(); grid_test_clear_all(CELL_EMPTY);
  s_game.screen=GAME_SCREEN_PLAYING;
  s_game.player=(QbCell){32,30}; s_game.dir=s_game.arrival_dir=arrival;
  s_game.edge_follow=s_game.moving=true; s_game.touch_controls=touch;
  s_game.enemies.rift_count=s_game.enemies.runner_count=0;
  grid_test_set(32,30,CELL_BORDER);
  for(QbDir d=QB_DIR_UP;d<=QB_DIR_LEFT;d++)
    if(mask&bit(d)) grid_test_set(32+dx(d),30+dy(d),CELL_BORDER);
}

static void exhaustive_touch_choices(void) {
  const QbDir order[]={QB_DIR_UP,QB_DIR_LEFT,QB_DIR_RIGHT,QB_DIR_DOWN};
  for(QbDir arrival=QB_DIR_UP;arrival<=QB_DIR_LEFT;arrival++) {
    const QbDir back=prv_reverse(arrival);
    for(unsigned mask=1;mask<16;mask++) {
      if(!(mask&bit(back))) continue;
      QbDir expected[3]; unsigned count=0;
      for(unsigned i=0;i<4;i++)
        if(order[i]!=back && (mask&bit(order[i]))) expected[count++]=order[i];
      fixture(mask,arrival,true); prv_continue_on_edge();
      assert(s_game.moving==(count==1));
      assert(game_view().choosing_path==(count>1));
      assert(game_view().edge_dead_end==(count==0));
      if(count==1) assert(s_game.dir==expected[0]);
      else assert(s_game.dir==arrival);
      for(unsigned i=0;i<count;i++) {
        game_set_direction(expected[i]);
        assert(s_game.dir==expected[i] && s_game.moving && s_game.touch_controls);
      }
      QbDir selected=s_game.dir;
      game_set_direction(back); assert(s_game.dir==selected);
      game_tap(); assert(s_game.dir==back && s_game.moving);
      prv_move_player();
      assert(s_game.player.x==32+dx(back) && s_game.player.y==30+dy(back));
      assert(!s_game.drawing && grid_is_player_path(s_game.player.x,s_game.player.y));
    }
  }
}

static void exhaustive_button_paths(void) {
  for(QbDir arrival=QB_DIR_UP;arrival<=QB_DIR_LEFT;arrival++) {
    for(unsigned mask=1;mask<16;mask++) {
      if(!(mask&bit(prv_reverse(arrival)))) continue;
      fixture(mask,arrival,false); prv_continue_on_edge();
      unsigned onward=0;
      for(QbDir d=QB_DIR_UP;d<=QB_DIR_LEFT;d++)
        if(d!=prv_reverse(arrival) && (mask&bit(d))) onward++;
      assert(s_game.moving==((mask&bit(arrival))!=0 || onward==1));
      if(mask&bit(arrival)) assert(s_game.dir==arrival);
      assert(!game_view().choosing_path && !game_view().edge_dead_end);
    }
    fixture(15,arrival,false); game_turn_left(false);
    assert(s_game.dir==prv_turn_left(arrival) && s_game.moving && !s_game.touch_controls);
    fixture(15,arrival,false); game_turn_right(false);
    assert(s_game.dir==prv_turn_right(arrival) && s_game.moving);
    QbDir heading=s_game.dir;
    game_stop_go(); assert(!s_game.moving && s_game.dir==heading);
    game_stop_go(); assert(s_game.moving && s_game.dir==heading);
  }
}

static void unambiguous_paths(void) {
  const QbCell corners[]={{0,0},{QB_GRID_W-1,0},{QB_GRID_W-1,QB_GRID_H-1},{0,QB_GRID_H-1}};
  const QbDir approach[][2]={{QB_DIR_UP,QB_DIR_LEFT},{QB_DIR_RIGHT,QB_DIR_UP},
                           {QB_DIR_DOWN,QB_DIR_RIGHT},{QB_DIR_LEFT,QB_DIR_DOWN}};
  const QbDir exit[][2]={{QB_DIR_RIGHT,QB_DIR_DOWN},{QB_DIR_DOWN,QB_DIR_LEFT},
                       {QB_DIR_LEFT,QB_DIR_UP},{QB_DIR_UP,QB_DIR_RIGHT}};
  for(unsigned i=0;i<4;i++) for(unsigned j=0;j<2;j++) {
    game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
    s_game.enemies.rift_count=s_game.enemies.runner_count=0;
    s_game.player=(QbCell){corners[i].x-dx(approach[i][j]),corners[i].y-dy(approach[i][j])};
    s_game.dir=s_game.arrival_dir=approach[i][j];
    s_game.edge_follow=s_game.moving=true;
    prv_move_player();
    assert(s_game.player.x==corners[i].x && s_game.player.y==corners[i].y);
    assert(s_game.moving && s_game.dir==exit[i][j]);
    prv_move_player();
    assert(s_game.player.x==corners[i].x+dx(exit[i][j]));
    assert(s_game.player.y==corners[i].y+dy(exit[i][j]));
  }
  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_DOWN),QB_DIR_UP,false);
  prv_continue_on_edge(); assert(s_game.moving && s_game.dir==QB_DIR_LEFT);
  s_game.dir=QB_DIR_UP; s_game.moving=false;
  game_stop_go(); assert(s_game.moving && s_game.dir==QB_DIR_UP && !s_game.edge_follow);
  s_game.dir=QB_DIR_UP;
  game_turn_left(false); assert(s_game.dir==QB_DIR_LEFT && s_game.moving);
  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_DOWN),QB_DIR_UP,true);
  prv_continue_on_edge(); assert(s_game.moving && s_game.dir==QB_DIR_LEFT);
}

static void wall_and_source_switch(void) {
  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  s_game.enemies.rift_count=s_game.enemies.runner_count=0;
  for(int x=1;x<12;x++) grid_test_set(x,20,CELL_BORDER);
  s_game.player=(QbCell){1,20}; s_game.dir=s_game.arrival_dir=QB_DIR_LEFT;
  s_game.edge_follow=s_game.moving=true;
  prv_move_player();
  assert(s_game.player.x==0 && !s_game.moving && !game_view().choosing_path);
  game_turn_left(false); assert(s_game.dir==QB_DIR_DOWN);
  game_stop_go(); assert(!s_game.moving && s_game.dir==QB_DIR_DOWN);
  game_stop_go(); assert(s_game.moving && s_game.dir==QB_DIR_DOWN);
  s_game.dir=QB_DIR_LEFT; s_game.moving=false;
  game_stop_go(); assert(s_game.moving && s_game.dir==QB_DIR_RIGHT);
  s_game.dir=QB_DIR_LEFT;
  game_turn_right(false); assert(s_game.dir==QB_DIR_UP);
  game_touch_tap(100,100); assert(s_game.touch_controls && s_game.dir==QB_DIR_RIGHT);
  game_set_direction(QB_DIR_UP); assert(s_game.touch_controls && s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(!s_game.touch_controls && !s_game.moving);
  game_stop_go(); assert(s_game.dir==QB_DIR_UP && s_game.moving);
}

static void cuts_and_freeze(void) {
  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_RIGHT),QB_DIR_RIGHT,false);
  game_turn_left(false); assert(!s_game.edge_follow && s_game.dir==QB_DIR_UP);
  prv_move_player(); assert(s_game.drawing && s_game.player.y==29);
  game_toggle_slow(); assert(game_view().slow_mode && !game_view().cut_slow);
  game_set_direction(QB_DIR_DOWN); assert(s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(!s_game.moving && s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(s_game.moving && s_game.dir==QB_DIR_UP);
  game_turn_right(false); assert(s_game.dir==QB_DIR_RIGHT);
  prv_move_player(); assert(s_game.player.x==33 && s_game.player.y==29);
  game_tap(); assert(!s_game.moving && s_game.touch_controls);
  game_tap(); assert(s_game.moving && s_game.dir==QB_DIR_RIGHT);

  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_RIGHT),QB_DIR_RIGHT,false);
  game_turn_left(true); assert(s_game.slow_mode && s_game.dir==QB_DIR_UP);
  prv_move_player(); assert(s_game.drawing && game_view().cut_slow);

  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_RIGHT)|bit(QB_DIR_DOWN),QB_DIR_UP,false);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=2;
  game_turn_right(false); assert(s_game.pending_action==PENDING_RIGHT_TURN);
  QbCell start=s_game.player;
  game_update(); assert(s_game.player.x==start.x && s_game.screen==GAME_SCREEN_CLAIM_PAUSE);
  game_update(); assert(s_game.dir==QB_DIR_RIGHT && s_game.moving);
  assert(s_game.arrival_dir==QB_DIR_UP && s_game.pending_action==PENDING_NONE);
  prv_move_player(); assert(s_game.player.x==33);

  fixture(15,QB_DIR_UP,false);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_stop_go(); game_update(); assert(s_game.moving && s_game.dir==QB_DIR_UP);
  game_stop_go(); assert(!s_game.moving && s_game.dir==QB_DIR_UP);

  fixture(15,QB_DIR_UP,true);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_set_direction(QB_DIR_RIGHT); assert(s_game.pending_action==PENDING_RIGHT);
  game_set_direction(QB_DIR_DOWN); assert(s_game.pending_action==PENDING_RIGHT);
  game_update(); assert(s_game.touch_controls && s_game.dir==QB_DIR_RIGHT && s_game.moving);

  /* Resolve a buffered relative turn at input, before any automatic corner. */
  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  s_game.player=(QbCell){0,0}; s_game.dir=s_game.arrival_dir=QB_DIR_UP;
  game_turn_right(false); game_update();
  assert(s_game.dir==QB_DIR_RIGHT && s_game.moving && !s_game.drawing);
}

static void captured_line_is_a_safe_touch_return(void) {
  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  s_game.enemies.runner_count=0; s_game.enemies.rift_count=1;
  QbRift *r=&s_game.enemies.rifts[0]; r->x_fp=r->y_fp=5*QB_FP_ONE+8;
  for(int i=0;i<QB_RIFT_LINES;i++) r->strokes[i]=(QbStroke){r->x_fp-8,r->y_fp,r->x_fp+8,r->y_fp};
  const int x=QB_GRID_W/2;
  s_game.player=(QbCell){x,0}; s_game.dir=s_game.arrival_dir=QB_DIR_UP;
  s_game.drawing=true; s_game.trail_len=QB_GRID_H-2;
  for(int y=1;y<QB_GRID_H-1;y++) grid_set_trail(x,y,false);
  prv_close_trail();
  assert(s_game.screen==GAME_SCREEN_CLAIM_PAUSE && s_game.claim_cells>0);
  const QbCell start=s_game.player; game_tap();
  for(int i=0;i<1000/QB_TIMER_MS;i++) {
    assert(s_game.player.x==start.x && s_game.player.y==start.y); game_update();
  }
  assert(s_game.screen==GAME_SCREEN_PLAYING && s_game.dir==QB_DIR_DOWN && s_game.moving);
  prv_move_player(); assert(s_game.player.x==x && s_game.player.y==1 && !s_game.drawing);
  assert(grid_get(x,1)==CELL_BORDER);
}

static void control_source_on_start(void) {
  game_init(); game_stop_go(); game_stop_go(); assert(!s_game.touch_controls);
  game_back_to_title(); game_touch_tap(100,150); game_touch_tap(100,120);
  assert(s_game.touch_controls && s_game.screen==GAME_SCREEN_COUNTDOWN);
  prv_begin_level(); assert(s_game.touch_controls);
  s_game.screen=GAME_SCREEN_GAME_OVER; game_stop_go(); assert(!s_game.touch_controls);
  s_game.screen=GAME_SCREEN_GAME_OVER; game_tap(); assert(s_game.touch_controls);
}

static void blocked_turns_and_resume(void) {
  for(QbDir facing=QB_DIR_UP;facing<=QB_DIR_LEFT;facing++) {
    game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
    const QbCell positions[]={{0,0},{0,20},{20,0},{QB_GRID_W-1,20},{20,QB_GRID_H-1}};
    s_game.player=positions[facing];
    s_game.dir=s_game.arrival_dir=facing;
    s_game.moving=s_game.edge_follow=true;
    /* Turning out of the frame still rotates; a second turn reaches the return road. */
    game_turn_left(false);
    assert(s_game.dir==prv_turn_left(facing) && !s_game.moving);
    game_turn_left(false);
    assert(s_game.dir==prv_reverse(facing) && s_game.moving);
    game_turn_right(false); game_turn_right(false);
    assert(s_game.dir==facing && s_game.moving);
    s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
    game_turn_left(false); game_turn_left(false);
    assert(s_game.pending_heading==prv_reverse(facing));
    game_update();
    assert(s_game.dir==prv_reverse(facing) && s_game.moving);
  }

  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  s_game.player=(QbCell){0,20}; s_game.dir=s_game.arrival_dir=QB_DIR_LEFT;
  s_game.edge_follow=true; s_game.moving=false;
  for(int x=1;x<10;x++) grid_test_set(x,20,CELL_BORDER);
  game_stop_go(); assert(s_game.dir==QB_DIR_RIGHT && s_game.moving);
  prv_move_player(); assert(s_game.player.x==1 && s_game.player.y==20);

  fixture(15,QB_DIR_UP,false); s_game.drawing=true;
  grid_test_set(32,31,CELL_TRAIL_FAST);
  game_turn_left(false); game_turn_left(false);
  assert(s_game.dir==QB_DIR_DOWN && !s_game.moving);
  game_stop_go(); assert(s_game.dir==QB_DIR_UP && s_game.moving);
  game_turn_right(false); game_turn_right(false);
  assert(s_game.dir==QB_DIR_DOWN && !s_game.moving);

  fixture(15,QB_DIR_UP,false);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_turn_left(false); game_turn_left(false);
  assert(s_game.pending_heading==QB_DIR_DOWN);
  game_update(); assert(s_game.dir==QB_DIR_DOWN && s_game.moving);
  fixture(15,QB_DIR_UP,false);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_stop_go(); game_stop_go(); game_update(); assert(!s_game.moving);
}

static void screenshot_buried_line_and_straight_cut(void) {
  game_init(); prv_new_game(); s_game.screen=GAME_SCREEN_PLAYING;
  s_game.enemies.rift_count=s_game.enemies.runner_count=0;
  for(int y=1;y<QB_GRID_H-1;y++) for(int x=47;x<QB_GRID_W-1;x++)
    grid_test_set(x,y,CELL_CLAIM_FAST);
  for(int y=34;y<=43;y++) for(int x=40;x<47;x++)
    grid_test_set(x,y,CELL_CLAIM_FAST);
  for(int y=15;y<=43;y++) grid_test_set(47,y,CELL_BORDER);
  s_game.player=(QbCell){47,34};
  s_game.dir=s_game.arrival_dir=QB_DIR_DOWN;
  s_game.edge_follow=true; s_game.moving=false;
  assert(grid_is_player_path(47,34) && !grid_is_player_path(47,35));
  game_stop_go(); assert(s_game.dir==QB_DIR_UP && s_game.moving);
  prv_move_player(); assert(s_game.player.x==47 && s_game.player.y==33);

  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_RIGHT),QB_DIR_UP,false);
  s_game.moving=false;
  game_stop_go(); assert(s_game.dir==QB_DIR_UP && s_game.moving && !s_game.edge_follow);
  prv_move_player(); assert(s_game.drawing && s_game.player.y==29);

  fixture(bit(QB_DIR_LEFT)|bit(QB_DIR_RIGHT),QB_DIR_UP,false);
  s_game.screen=GAME_SCREEN_CLAIM_PAUSE; s_game.screen_ticks=1;
  game_stop_go(); game_update();
  assert(s_game.dir==QB_DIR_UP && s_game.moving && !s_game.edge_follow);
}

int main(void) {
  exhaustive_touch_choices(); exhaustive_button_paths(); unambiguous_paths();
  wall_and_source_switch(); cuts_and_freeze(); captured_line_is_a_safe_touch_return(); control_source_on_start();
  blocked_turns_and_resume();
  screenshot_buried_line_and_straight_cut();
  puts("routes ok: unconditional quarter turns, straight/back resume, all unique paths; touch unchanged");
}
