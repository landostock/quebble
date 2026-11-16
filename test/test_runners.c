#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "enemies.h"

static void tick(QbEnemies *e) {
  bool hit;
  enemies_update(e, (QbCell){QB_GRID_W / 2, QB_GRID_H - 1}, false, &hit);
  assert(!hit);
}

static void test_constant_speed(void) {
  grid_init_level();
  QbEnemies normal, super;
  enemies_init_seed(&normal, 1, 713);
  normal.rift_count = 0;
  normal.super_ticks_remaining = 0;
  super = normal;
  super.runners_super = true;
  int steps = 0;
  for (int frame = 1; frame <= 6000; frame++) {
    QbPixel previous=enemies_get_runner_center(&normal,0);
    QbRunner before[2];
    memcpy(before, normal.runners, sizeof(before));
    tick(&normal);
    tick(&super);
    QbPixel current=enemies_get_runner_center(&normal,0);
    assert(qb_abs_i(current.x-previous.x)+qb_abs_i(current.y-previous.y)<=1);
    for (int i = 0; i < 2; i++) {
      const QbCell p = normal.runners[i].cell;
      const int distance = qb_abs_i(p.x - before[i].cell.x) + qb_abs_i(p.y - before[i].cell.y);
      assert(distance == (frame % QB_RUNNER_TICKS == 0 ? 1 : 0));
      assert(p.x == 0 || p.x == QB_GRID_W - 1 || p.y == 0 || p.y == QB_GRID_H - 1);
      assert(memcmp(&normal.runners[i], &super.runners[i], sizeof(QbRunner)) == 0);
      steps += distance;
    }
  }
  assert(steps == 2*(6000/QB_RUNNER_TICKS));
}

static void test_follows_new_contour(void) {
  grid_init_level();
  const int top_y=QB_GRID_H/2;
  for (int y = top_y; y < QB_GRID_H - 1; y++)
    for (int x = 20; x <= 40; x++) grid_test_set(x, y, CELL_CLAIM_FAST);
  for (int y = top_y+5; y < QB_GRID_H - 1; y++) grid_test_set(30, y, CELL_BORDER);
  int followed = 0, bypassed = 0;
  for (uint32_t seed = 1; seed <= 64; seed++) {
    QbEnemies e;
    enemies_init_seed(&e, 1, seed);
    e.rift_count = 0;
    e.runner_count = 1;
    e.runners[0].cell = (QbCell){20, QB_GRID_H - 1};
    e.runners[0].dir = QB_DIR_RIGHT;
    e.runners[0].hand = -1;
    for (int i = 0; i < QB_RUNNER_TICKS; i++) tick(&e);
    if (e.runners[0].cell.y == QB_GRID_H - 1) { bypassed++; continue; }
    followed++;
    bool left = false, top = false, right = false;
    for (int frame = 0; frame < QB_RUNNER_TICKS*(2*QB_GRID_H+30); frame++) {
      tick(&e);
      const QbCell p = e.runners[0].cell;
      assert(!(p.x > 20 && p.x < 40 && p.y > top_y && p.y < QB_GRID_H - 1));
      left |= p.x == 20 && p.y == top_y+5;
      top |= p.x == 30 && p.y == top_y;
      right |= p.x == 40 && p.y == top_y+5;
      if (left && top && right) break;
    }
    assert(left && top && right);
  }
  assert(followed > 0 && bypassed > followed);
}

static void test_trail_rules(void) {
  grid_init_level();
  for (int y = 1; y <= 8; y++) grid_set_trail(30, y, false);
  QbEnemies normal, super;
  enemies_init_seed(&normal, 1, 761);
  normal.rift_count = 0;
  normal.runner_count = 1;
  normal.runners[0] = (QbRunner){.cell = {30, 0}, .dir = QB_DIR_RIGHT, .hand = 1, .rng = 1};
  super = normal;
  super.runners_super = true;
  for (int i = 0; i < QB_RUNNER_TICKS; i++) { tick(&normal); tick(&super); }
  assert(normal.runners[0].cell.y == 0);
  assert(super.runners[0].cell.x == 30 && super.runners[0].cell.y == 1);
  assert(!enemies_player_collision(&super, (QbCell){30, 8}, true));
  int chased=0;
  while(!enemies_player_collision(&super,(QbCell){30,8},true) && chased<8*QB_RUNNER_TICKS) {
    tick(&super);
    chased++;
  }
  assert(chased>0 && chased<8*QB_RUNNER_TICKS);
}

typedef struct {
  int x,y,count;
  uint64_t mask;
  bool center;
  bool pixels[2*QB_RUNNER_RADIUS+1][2*QB_RUNNER_RADIUS+1];
} Sprite;

static void check_pixel(int x, int y, void *context) {
  Sprite *s = context;
  assert(x + QB_BOARD_X >= 0 && x + QB_BOARD_X < QB_SCREEN_W);
  assert(y + QB_BOARD_Y >= QB_HUD_H && y + QB_BOARD_Y < QB_SCREEN_H);
  const int dx = x - s->x, dy = y - s->y;
  assert(dx >= -QB_RUNNER_RADIUS && dx <= QB_RUNNER_RADIUS &&
         dy >= -QB_RUNNER_RADIUS && dy <= QB_RUNNER_RADIUS);
  s->mask ^= (uint64_t)((dy+QB_RUNNER_RADIUS)*(2*QB_RUNNER_RADIUS+1)+dx+QB_RUNNER_RADIUS);
  s->mask *= 1099511628211ULL;
  s->center |= dx==0 && dy==0;
  s->pixels[dy+QB_RUNNER_RADIUS][dx+QB_RUNNER_RADIUS]=true;
  s->count++;
}

typedef struct {
  Sprite sprite;
  uint8_t inks[2*QB_RUNNER_RADIUS+1][2*QB_RUNNER_RADIUS+1];
  unsigned palette;
} ColoredSprite;

static void check_colored_pixel(int x,int y,uint8_t ink,void *context) {
  ColoredSprite *colored=context;
  assert(ink<=1);
  check_pixel(x,y,&colored->sprite);
  colored->inks[y-colored->sprite.y+QB_RUNNER_RADIUS][x-colored->sprite.x+QB_RUNNER_RADIUS]=ink+1;
  colored->palette|=1U<<ink;
}

static void test_visible_flicker(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 71);
  e.rift_count = 0;
  uint64_t previous = 0;
  uint64_t cycle[19] = {0};
  int changes = 0;
  for (int frame = 0; frame < 1800; frame++) {
    tick(&e);
    for (int i = 0; i < 2; i++) {
      QbPixel center=enemies_get_runner_center(&e,i);
      Sprite sprite = {.x=center.x,.y=center.y,.mask=1469598103934665603ULL};
      enemies_visit_runner_pixels(&e, i, check_pixel, &sprite);
      assert(sprite.count >= 8*4 && sprite.count <= 12*4);
      assert(!sprite.center);
      int visible=0;
      const int side=2*QB_RUNNER_RADIUS+1;
      for(int y=0;y<side;y++) for(int x=0;x<side;x++) {
        if(!sprite.pixels[y][x]) continue;
        visible++;
        bool thick=false;
        for(int oy=-1;oy<=0;oy++) for(int ox=-1;ox<=0;ox++) {
          int left=x+ox,top=y+oy;
          if(left>=0 && top>=0 && left+1<side && top+1<side)
            thick|=sprite.pixels[top][left] && sprite.pixels[top][left+1] &&
                   sprite.pixels[top+1][left] && sprite.pixels[top+1][left+1];
        }
        assert(thick);
      }
      assert(visible>=24);
      ColoredSprite colored={.sprite={.x=center.x,.y=center.y,.mask=1469598103934665603ULL}};
      enemies_visit_runner_colored_pixels(&e,i,check_colored_pixel,&colored);
      assert(colored.palette==3);
      assert(colored.sprite.mask==sprite.mask && colored.sprite.count==sprite.count);
      assert(memcmp(colored.sprite.pixels,sprite.pixels,sizeof(sprite.pixels))==0);
      for(int y=0;y<side-1;y+=2) for(int x=0;x<side-1;x+=2) {
        const uint8_t ink=colored.inks[y][x];
        assert(colored.inks[y][x+1]==ink && colored.inks[y+1][x]==ink && colored.inks[y+1][x+1]==ink);
      }
      for(int y=QB_RUNNER_RADIUS-1;y<=QB_RUNNER_RADIUS;y++)
        for(int x=QB_RUNNER_RADIUS-1;x<=QB_RUNNER_RADIUS;x++) assert(!sprite.pixels[y][x]);
      if (i == 0) {
        changes += sprite.mask != previous;
        previous = sprite.mask;
        if (frame < 19) cycle[frame] = sprite.mask;
        else assert(sprite.mask == cycle[frame % 19]);
      }
    }
  }
  assert(changes > 700);
  int distinct=0;
  for(int i=0;i<19;i++) {
    bool seen=false;
    for(int j=0;j<i;j++) seen|=cycle[i]==cycle[j];
    distinct+=!seen;
  }
  assert(distinct==8);
  assert(enemies_player_collision(&e, e.runners[0].cell, false));
  assert(!enemies_player_collision(&e, (QbCell){QB_GRID_W/2, QB_GRID_H/2}, false));
}

static void test_escalation(void) {
  grid_init_level();
  for(int level=1;level<=8;level++) {
    QbEnemies e;
    enemies_init_seed(&e,level,71);
    e.rift_count=0;
    assert(e.runner_count==(level>=5?4:2));
    assert(e.runners_super==(level>=7));
    if(level>=7) continue;
    e.super_ticks_remaining=1;
    tick(&e);
    assert(e.runner_count==4);
    assert(e.runners_super==(level>=5));
    if(level<5) {
      assert(e.super_ticks_remaining==30*(1000/QB_TIMER_MS));
      e.super_ticks_remaining=1;
      tick(&e);
      assert(e.runners_super && e.super_ticks_remaining==0);
    }
    for(int tick_index=0;tick_index<600;tick_index++) {
      QbCell before[QB_MAX_RUNNERS];
      for(int i=0;i<e.runner_count;i++) before[i]=e.runners[i].cell;
      tick(&e);
      for(int i=0;i<e.runner_count;i++) {
        int distance=qb_abs_i(e.runners[i].cell.x-before[i].x)+qb_abs_i(e.runners[i].cell.y-before[i].y);
        assert(distance==(e.runner_subtick==0?1:0));
      }
    }
  }
}

static void test_interpolation_wrap_and_blocked_path(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e,1,72);
  e.rift_count=0;
  e.super_ticks_remaining=0;
  e.anim=UINT16_MAX-2;
  QbPixel previous=enemies_get_runner_center(&e,0);
  for(int frame=0;frame<100;frame++) {
    tick(&e);
    QbPixel current=enemies_get_runner_center(&e,0);
    assert(qb_abs_i(current.x-previous.x)+qb_abs_i(current.y-previous.y)<=1);
    previous=current;
  }
  for(int y=3;y<=7;y++) for(int x=3;x<=7;x++) grid_test_set(x,y,CELL_CLAIM_FAST);
  e.runner_count=1;
  e.runners[0]=(QbRunner){.cell={5,5},.dir=QB_DIR_RIGHT,.hand=1,.rng=1};
  previous=enemies_get_runner_center(&e,0);
  for(int frame=0;frame<100;frame++) {
    tick(&e);
    QbPixel current=enemies_get_runner_center(&e,0);
    assert(current.x==previous.x && current.y==previous.y);
  }
}

int main(void) {
  test_constant_speed();
  test_follows_new_contour();
  test_trail_rules();
  test_visible_flicker();
  test_escalation();
  test_interpolation_wrap_and_blocked_path();
  puts("runners ok: constant speed, contours, physical pursuit, reinforcements, eight unclipped body phases");
  return 0;
}
