#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "enemies.h"

static void check_empty(int x, int y, void *context) {
  (void)context;
  assert(x >= 0 && x < QB_BOARD_W);
  assert(y >= 0 && y < QB_GRID_H * QB_CELL_PX);
  assert(grid_get(x / QB_CELL_PX, y / QB_CELL_PX) == CELL_EMPTY);
}

static void check_shape(const QbEnemies *e) {
  for (unsigned i = 0; i < e->rift_count; i++) {
    assert(grid_get(e->rifts[i].x_fp / 16, e->rifts[i].y_fp / 16) == CELL_EMPTY);
    for (int line = 0; line < QB_RIFT_LINES; line++)
      enemies_visit_rift_line(&e->rifts[i], line, check_empty, NULL);
  }
}

static void test_no_tracking(void) {
  grid_init_level();
  QbEnemies a, b;
  enemies_init_seed(&a, 3, 7321);
  enemies_init_seed(&b, 3, 7322);
  assert(memcmp(a.rifts, b.rifts, sizeof(a.rifts)) != 0);
  enemies_init_seed(&b, 3, 7321);
  for (int frame = 0; frame < 5000; frame++) {
    bool hit_a, hit_b;
    enemies_update(&a, (QbCell){1, 1}, true, &hit_a);
    enemies_update(&b, (QbCell){98, 101}, false, &hit_b);
    assert(!hit_a && !hit_b);
    assert(memcmp(&a, &b, sizeof(a)) == 0);
  }
}

static void test_aggressive_stages(void) {
  grid_init_level();
  const int stages[]={2,4,5,7};
  for(unsigned stage=0;stage<sizeof(stages)/sizeof(stages[0]);stage++) {
    QbEnemies a,b;
    enemies_init_seed(&a,stages[stage],7321);
    b=a;
    int different=0;
    for(int frame=0;frame<3000;frame++) {
      const QbRift before[2]={a.rifts[0],a.rifts[1]};
      bool hit_a,hit_b;
      enemies_update(&a,(QbCell){QB_GRID_W-2,QB_GRID_H-2},true,&hit_a);
      enemies_update(&b,(QbCell){1,1},true,&hit_b);
      assert(!hit_a && !hit_b);
      different+=memcmp(a.rifts,b.rifts,sizeof(a.rifts))!=0;
      check_shape(&a);
      check_shape(&b);
      for(int i=0;i<a.rift_count;i++) for(int endpoint=0;endpoint<2;endpoint++) {
        QbPixel from=enemies_get_rift_visual_point(&before[i],0,endpoint);
        QbPixel to=enemies_get_rift_visual_point(&a.rifts[i],0,endpoint);
        assert(qb_abs_i(to.x-from.x)<=QB_RIFT_MAX_STEP_PX);
        assert(qb_abs_i(to.y-from.y)<=QB_RIFT_MAX_STEP_PX);
      }
    }
    assert(different>100);
  }
}

static int checks;

static void hold_envelopes(QbRift *r, uint8_t motion, uint8_t turn) {
  r->motion_intensity = r->target_motion_intensity = motion;
  r->turn_intensity = r->target_turn_intensity = turn;
  r->motion_envelope_timer = r->turn_envelope_timer = 200;
}

static void test_walls(int fixture) {
  for (uint32_t seed = 1; seed <= 12; seed++) {
    grid_init_level();
    if (fixture) {
      for (int y = 1; y < QB_GRID_H - 1; y++) {
        for (int x = 1; x < QB_GRID_W - 1; x++) {
          if (x < QB_GRID_W/6 || (y > QB_GRID_H*3/4 && x < QB_GRID_W*3/4) ||
              (x > QB_GRID_W*4/5 && y < QB_GRID_H/4))
            grid_test_set(x, y, CELL_CLAIM_FAST);
        }
      }
    }
    QbEnemies e;
    enemies_init_seed(&e, 1, seed * 9187);
    int still = 0, worst_still = 0, corner = 0, worst_corner = 0;
    int min_x = 10000, max_x = 0, min_y = 10000, max_y = 0;
    for (int frame = 0; frame < 6000; frame++) {
      const QbRift before = e.rifts[0];
      bool hit;
      enemies_update(&e, (QbCell){50, 102}, false, &hit);
      assert(!hit);
      check_shape(&e);
      const QbRift *r = &e.rifts[0];
      assert(qb_abs_i(r->x_fp - before.x_fp) <= QB_RIFT_MAX_STEP_PX*QB_FP_ONE/QB_CELL_PX);
      assert(qb_abs_i(r->y_fp - before.y_fp) <= QB_RIFT_MAX_STEP_PX*QB_FP_ONE/QB_CELL_PX);
      for (int age = 1; age < QB_RIFT_LINES; age++)
        assert(memcmp(&r->strokes[age], &before.strokes[age - 1], sizeof(QbStroke)) == 0);
      for (int endpoint = 0; endpoint < 2; endpoint++) {
        QbPixel a = enemies_get_rift_visual_point(&before, 0, endpoint);
        QbPixel b = enemies_get_rift_visual_point(r, 0, endpoint);
        assert(qb_abs_i(a.x - b.x) <= 4);
        assert(qb_abs_i(a.y - b.y) <= 4);
      }
      if (r->x_fp == before.x_fp && r->y_fp == before.y_fp) still++;
      else still = 0;
      if (still > worst_still) worst_still = still;
      const int cx = r->x_fp / 16, cy = r->y_fp / 16;
      if ((cx < QB_CORNER_CELLS || cx >= QB_GRID_W-QB_CORNER_CELLS) &&
          (cy < QB_CORNER_CELLS || cy >= QB_GRID_H-QB_CORNER_CELLS)) corner++;
      else corner = 0;
      if (corner > worst_corner) worst_corner = corner;
      if (r->x_fp < min_x) min_x = r->x_fp;
      if (r->x_fp > max_x) max_x = r->x_fp;
      if (r->y_fp < min_y) min_y = r->y_fp;
      if (r->y_fp > max_y) max_y = r->y_fp;
      checks++;
    }
    assert(worst_still < 30);
    assert(worst_corner < 120);
    assert(max_x - min_x > 40 * 8);
    assert(max_y - min_y > 40 * 8);
  }
}

static void test_swept_trail(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 17);
  QbRift *r = &e.rifts[0];
  r->x_fp = 641;
  r->y_fp = 720;
  r->angle = 1024;
  r->half_length = r->target_length = 80;
  r->vx = r->target_vx = 23;
  r->vy = r->target_vy = 0;
  r->angular_velocity = r->target_spin = 0;
  r->pivot = r->target_pivot = 0;
  r->length_timer = 200;
  r->spin_timer = 20;
  r->turn_timer = 20;
  hold_envelopes(r, 128, 128);
  for (int i = 0; i < QB_RIFT_LINES; i++) r->strokes[i] = (QbStroke){641, 640, 641, 800};
  for (int y = 40; y <= 50; y++) grid_set_trail(41, y, false);
  assert(!enemies_trail_collision(&e));
  bool hit;
  enemies_update(&e, (QbCell){41, 50}, true, &hit);
  assert(hit);
  assert(r->x_fp == 641);
}

static void test_rotation_freedom(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 765);
  QbRift *r = &e.rifts[0];
  r->x_fp = 800;
  r->y_fp = 800;
  r->angle = 0;
  r->half_length = r->target_length = 96;
  r->vx = r->vy = r->target_vx = r->target_vy = 0;
  r->angular_velocity = r->target_spin = 96;
  r->pivot = r->target_pivot = 0;
  r->length_timer = 200;
  r->turn_timer = r->spin_timer = 200;
  hold_envelopes(r, 128, 128);
  for (int i = 0; i < QB_RIFT_LINES; i++) r->strokes[i] = (QbStroke){704, 800, 896, 800};
  int rotation = 0;
  for (int frame = 0; frame < 60; frame++) {
    const int angle = r->angle;
    bool hit;
    enemies_update(&e, (QbCell){1, 1}, false, &hit);
    assert(!hit);
    assert(r->x_fp == 800 && r->y_fp == 800);
    check_shape(&e);
    const int delta = (r->angle - angle + 4096) & 4095;
    assert(delta == 96);
    rotation += delta;
    if (frame >= QB_RIFT_LINES) {
      const QbStroke oldest = r->strokes[QB_RIFT_LINES - 1];
      const QbStroke newest = r->strokes[0];
      const int ax = newest.bx - newest.ax, ay = newest.by - newest.ay;
      const int bx = oldest.bx - oldest.ax, by = oldest.by - oldest.ay;
      /* More than 45 degrees of visible fan spread, not just a narrow wobble. */
      assert(qb_abs_i(ax * by - ay * bx) > qb_abs_i(ax * bx + ay * by));
    }
  }
  assert(rotation > 4096);

  /* Fast rotation plus translation must be slowed to fit, never cancelled. */
  r->vx = r->target_vx = 23;
  r->vy = r->target_vy = 23;
  r->target_spin = r->angular_velocity = 112;
  bool hit;
  enemies_update(&e, (QbCell){1, 1}, false, &hit);
  assert(!hit && r->angular_velocity > 0);
  check_shape(&e);
}

static void test_irregular_motion(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 915613);
  int quiet = 0, strong = 0, fast_tight = 0, local_fan = 0;
  int narrow = 0, wide = 0, near_limit = 0;
  int min_length = 1000, max_length = 0;
  for (int frame = 0; frame < 12000; frame++) {
    const QbRift before = e.rifts[0];
    bool hit;
    enemies_update(&e, (QbCell){1, 1}, false, &hit);
    assert(!hit);
    const QbRift *r = &e.rifts[0];
    const int turn = qb_abs_i(r->angular_velocity);
    const int drift = qb_abs_i(r->vx) + qb_abs_i(r->vy);
    quiet += turn <= 12;
    strong += turn >= 60;
    fast_tight += turn <= 12 && drift >= 16;
    local_fan += turn >= 60 && drift <= 8;
    if (r->half_length < min_length) min_length = r->half_length;
    if (r->half_length > max_length) max_length = r->half_length;
    const QbStroke a = r->strokes[0], b = r->strokes[QB_RIFT_LINES - 1];
    const int ax = a.bx - a.ax, ay = a.by - a.ay;
    const int bx = b.bx - b.ax, by = b.by - b.ay;
    const int cross = qb_abs_i(ax * by - ay * bx), dot = qb_abs_i(ax * bx + ay * by);
    narrow += cross * 4 < dot;
    wide += cross > dot;
    const QbStroke p = before.strokes[0];
    const int limit=QB_RIFT_MAX_STEP_PX*QB_FP_ONE/QB_CELL_PX*7/8;
    near_limit += qb_abs_i(a.ax-p.ax) >= limit || qb_abs_i(a.ay-p.ay) >= limit ||
                  qb_abs_i(a.bx-p.bx) >= limit || qb_abs_i(a.by-p.by) >= limit;
  }
  printf("intensity: quiet=%d strong=%d fast-tight=%d local-fan=%d narrow=%d wide=%d limit=%d length=%d..%d\n",
    quiet, strong, fast_tight, local_fan, narrow, wide, near_limit, min_length, max_length);
  assert(quiet > strong && strong > 100);
  assert(fast_tight > 100 && local_fan > 50);
  assert(narrow > wide && wide > 50);
  assert(near_limit < 12000 / 4);
  assert(min_length <= qb_pixel_to_fp(QB_RIFT_MIN_LENGTH_PX+4)/2 &&
         max_length >= qb_pixel_to_fp(QB_RIFT_MAX_LENGTH_PX-6)/2);
}

static void test_fast_translation_without_large_fan(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 159);
  QbRift *r = &e.rifts[0];
  r->x_fp = r->y_fp = 512;
  r->angle = 0;
  r->half_length = r->target_length = 96;
  r->vx = r->target_vx = 16;
  r->vy = r->target_vy = 0;
  r->angular_velocity = 8;
  r->target_spin = 128;
  r->pivot = r->target_pivot = 0;
  r->turn_timer = r->spin_timer = r->length_timer = 200;
  hold_envelopes(r, 128, 8);
  for (int i = 0; i < QB_RIFT_LINES; i++) r->strokes[i] = (QbStroke){416,512,608,512};
  for (int frame = 0; frame < 12; frame++) {
    bool hit;
    const int x = r->x_fp;
    enemies_update(&e, (QbCell){1,1}, false, &hit);
    assert(!hit && r->x_fp == x + 16 && r->angular_velocity == 8);
    check_shape(&e);
  }
}

static void mark_and_check(int x, int y, void *context) {
  QbEnemies *e = context;
  grid_test_set(x / QB_CELL_PX, y / QB_CELL_PX, CELL_TRAIL_FAST);
  assert(enemies_trail_collision(e));
  bool hit;
  enemies_update(e, (QbCell){1, 1}, true, &hit);
  assert(hit);
  grid_test_set(x / QB_CELL_PX, y / QB_CELL_PX, CELL_EMPTY);
}

static void test_every_pixel(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e, 1, 13476);
  for (int age = 0; age < QB_RIFT_LINES; age++)
    enemies_visit_rift_line(&e.rifts[0], age, mark_and_check, &e);
  QbPixel p = enemies_get_rift_visual_point(&e.rifts[0], QB_RIFT_LINES - 1, 1);
  grid_test_set(p.x / QB_CELL_PX, p.y / QB_CELL_PX, CELL_TRAIL_SLOW);
  QbPixel wall = enemies_get_rift_visual_point(&e.rifts[0], 0, 0);
  grid_test_set(wall.x / QB_CELL_PX, wall.y / QB_CELL_PX, CELL_BORDER);
  assert(enemies_trail_collision(&e));
}

static void test_thick_stroke_contact(void) {
  grid_init_level();
  QbEnemies e;
  enemies_init_seed(&e,1,127);
  e.rift_count=1;
  const int y=11*QB_CELL_PX-1;
  const int fp_y=(y*QB_FP_ONE+QB_CELL_PX-1)/QB_CELL_PX;
  for(int i=0;i<QB_RIFT_LINES;i++) e.rifts[0].strokes[i]=(QbStroke){10*QB_FP_ONE,fp_y,15*QB_FP_ONE,fp_y};
  assert(!enemies_trail_collision(&e));
  grid_set_trail(12,11,false);
  assert(enemies_trail_collision(&e));
}

int main(void) {
  test_no_tracking();
  test_aggressive_stages();
  test_walls(0);
  test_walls(1);
  test_every_pixel();
  test_swept_trail();
  test_rotation_freedom();
  test_irregular_motion();
  test_fast_translation_without_large_fan();
  test_thick_stroke_contact();
  printf("motion ok: %d wall frames + 12000 aggressive frames, exact pixels, bounded steps, random-stage independence\n", checks);
  return 0;
}
