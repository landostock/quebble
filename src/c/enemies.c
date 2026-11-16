#include "enemies.h"

#include <string.h>

/* Positions and lengths use 1/16 cell. Angles use 4096 units per turn. */
#define FP_ONE QB_FP_ONE
#define ANGLE_MASK 4095
#define MAX_ENDPOINT_STEP (QB_RIFT_MAX_STEP_PX * FP_ONE / QB_CELL_PX)

typedef enum { SPACE_OK = 0, SPACE_WALL = 1, SPACE_TRAIL = 2 } Space;

static uint32_t prv_rng_state(uint32_t *state) {
  uint32_t x = *state ? *state : 1;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

static uint32_t prv_rng(QbEnemies *enemies) {
  return prv_rng_state(&enemies->rng);
}

static int prv_random(QbEnemies *enemies, int lo, int hi) {
  return lo + (int)(prv_rng(enemies) % (uint32_t)(hi - lo + 1));
}

static int prv_sin(int angle) {
  static const int16_t quarter[17] = {
    0, 25, 50, 74, 98, 121, 142, 162, 181,
    198, 213, 226, 237, 245, 251, 255, 256
  };
  angle &= ANGLE_MASK;
  const int sign = angle >= 2048 ? -1 : 1;
  int a = angle & 2047;
  if (a > 1024) a = 2048 - a;
  const int i = a / 64;
  const int v = i == 16 ? 256 :
    quarter[i] + (quarter[i + 1] - quarter[i]) * (a % 64) / 64;
  return sign * v;
}

static int prv_approach(int value, int target, int divisor) {
  const int difference = target - value;
  if (!difference) return value;
  int step = difference / divisor;
  if (!step) step = difference > 0 ? 1 : -1;
  return value + step;
}

static QbStroke prv_stroke(int x, int y, int angle, int length) {
  const int dx = prv_sin(angle + 1024) * length / 256;
  const int dy = prv_sin(angle) * length / 256;
  return (QbStroke){x - dx, y - dy, x + dx, y + dy};
}

QbPixel enemies_get_rift_visual_point(const QbRift *rift, uint8_t line, uint8_t point) {
  if (line >= QB_RIFT_LINES) line = QB_RIFT_LINES - 1;
  const QbStroke *s = &rift->strokes[line];
  return (QbPixel){qb_fp_to_pixel(point ? s->bx : s->ax), qb_fp_to_pixel(point ? s->by : s->ay)};
}

/* The exact same integer rasterizer is used by rendering and collisions. */
static void prv_visit_stroke(QbStroke stroke, QbRiftPixelVisitor visitor, void *context) {
  int x = qb_fp_to_pixel(stroke.ax), y = qb_fp_to_pixel(stroke.ay);
  const int end_x = qb_fp_to_pixel(stroke.bx), end_y = qb_fp_to_pixel(stroke.by);
  const int dx = qb_abs_i(end_x - x), dy = -qb_abs_i(end_y - y);
  const int sx = x < end_x ? 1 : -1, sy = y < end_y ? 1 : -1;
  int error = dx + dy;
  for (;;) {
    for (int oy = 0; oy < QB_RIFT_BRUSH_PX; oy++)
      for (int ox = 0; ox < QB_RIFT_BRUSH_PX; ox++) visitor(x + ox, y + oy, context);
    if (x == end_x && y == end_y) break;
    const int twice = error * 2;
    if (twice >= dy) { error += dy; x += sx; }
    if (twice <= dx) { error += dx; y += sy; }
  }
}

void enemies_visit_rift_line(const QbRift *rift, uint8_t line,
                             QbRiftPixelVisitor visitor, void *context) {
  if (line < QB_RIFT_LINES) prv_visit_stroke(rift->strokes[line], visitor, context);
}

static void prv_check_pixel(int x, int y, void *context) {
  Space *space = context;
  if (x < 0 || y < 0 || x >= QB_BOARD_W || y >= QB_BOARD_H) {
    *space |= SPACE_WALL;
    return;
  }
  const QbCellState state = grid_get(x / QB_CELL_PX, y / QB_CELL_PX);
  if (grid_is_trail_state(state)) *space |= SPACE_TRAIL;
  else if (state != CELL_EMPTY) *space |= SPACE_WALL;
}

static Space prv_space(QbStroke stroke) {
  Space space = SPACE_OK;
  prv_visit_stroke(stroke, prv_check_pixel, &space);
  return space;
}

bool enemies_trail_collision(const QbEnemies *enemies) {
  for (uint8_t i = 0; i < enemies->rift_count; i++) {
    for (uint8_t line = 0; line < QB_RIFT_LINES; line++) {
      if (prv_space(enemies->rifts[i].strokes[line]) & SPACE_TRAIL) return true;
    }
  }
  return false;
}

static QbStroke prv_lerp_stroke(QbStroke a, QbStroke b, int n, int d) {
  return (QbStroke){a.ax + (b.ax - a.ax) * n / d, a.ay + (b.ay - a.ay) * n / d,
                    a.bx + (b.bx - a.bx) * n / d, a.by + (b.by - a.by) * n / d};
}

static int prv_endpoint_distance(QbStroke a, QbStroke b) {
  int maximum = qb_abs_i(a.ax - b.ax);
  const int values[] = {a.ay - b.ay, a.bx - b.bx, a.by - b.by};
  for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
    if (qb_abs_i(values[i]) > maximum) maximum = qb_abs_i(values[i]);
  }
  return maximum;
}

static Space prv_swept_space(QbStroke from, QbStroke to) {
  const int steps = prv_endpoint_distance(from, to) / (FP_ONE / (2 * QB_CELL_PX)) + 1;
  Space space = SPACE_OK;
  for (int i = 1; i <= steps; i++) {
    const Space sample = prv_space(prv_lerp_stroke(from, to, i, steps));
    space |= sample;
  }
  return space;
}

static void prv_choose_motion(QbEnemies *enemies, QbRift *rift) {
  const int heading = prv_random(enemies, 0, ANGLE_MASK);
  rift->target_vx = prv_sin(heading + 1024) * MAX_ENDPOINT_STEP / 256;
  rift->target_vy = prv_sin(heading) * MAX_ENDPOINT_STEP / 256;
  rift->turn_timer = prv_random(enemies, 6, 28);
}

static void prv_choose_spin(QbEnemies *enemies, QbRift *rift) {
  rift->target_spin = prv_random(enemies, -144, 144);
  rift->spin_timer = prv_random(enemies, 6, 30);
  const int anchor = prv_random(enemies, 0, 2);
  rift->target_pivot = anchor == 0 ? -128 : anchor == 1 ? 128 : prv_random(enemies, -64, 64);
}

static uint8_t prv_choose_intensity(QbEnemies *enemies) {
  const int weight = prv_random(enemies, 0, 99);
  if (weight < 35) return prv_random(enemies, 8, 32);
  if (weight < 80) return prv_random(enemies, 40, 88);
  return prv_random(enemies, 100, 128);
}

static void prv_update_envelopes(QbEnemies *enemies, QbRift *rift) {
  if (!rift->motion_envelope_timer) {
    rift->target_motion_intensity = prv_choose_intensity(enemies);
    rift->motion_envelope_timer = prv_random(enemies, 16, 55);
  } else rift->motion_envelope_timer--;
  if (!rift->turn_envelope_timer) {
    rift->target_turn_intensity = prv_choose_intensity(enemies);
    rift->turn_envelope_timer = prv_random(enemies, 12, 45);
  } else rift->turn_envelope_timer--;
  rift->motion_intensity = prv_approach(rift->motion_intensity, rift->target_motion_intensity, 8);
  rift->turn_intensity = prv_approach(rift->turn_intensity, rift->target_turn_intensity, 8);
}

static QbStroke prv_pose(const QbRift *rift, int vx, int vy, int spin, int length) {
  const QbStroke rotated = prv_stroke(rift->x_fp, rift->y_fp, rift->angle + spin, length);
  /* Shift the rotation centre towards either endpoint: a fan, not a propeller. */
  const int dx = (rotated.bx - rift->strokes[0].bx) * rift->pivot / 128;
  const int dy = (rotated.by - rift->strokes[0].by) * rift->pivot / 128;
  return (QbStroke){rotated.ax + vx - dx, rotated.ay + vy - dy,
                    rotated.bx + vx - dx, rotated.by + vy - dy};
}

static QbStroke prv_bounded_pose(const QbRift *rift, int *vx, int *vy,
                                int *spin, int *length) {
  for (;;) {
    const QbStroke next = prv_pose(rift, *vx, *vy, *spin, *length);
    if (prv_endpoint_distance(rift->strokes[0], next) <= MAX_ENDPOINT_STEP) return next;
    /* Preserve the turn instead of rejecting it when translation uses the budget. */
    *vx = *vx * 7 / 8;
    *vy = *vy * 7 / 8;
    *spin = *spin * 7 / 8;
    *length = rift->half_length + (*length - rift->half_length) * 7 / 8;
  }
}

static void prv_init_rift(QbEnemies *enemies, QbRift *rift, int x, int y) {
  rift->x_fp = x * FP_ONE + 8;
  rift->y_fp = y * FP_ONE + 8;
  rift->angle = prv_random(enemies, 0, ANGLE_MASK);
  rift->half_length = qb_pixel_to_fp(42) / 2;
  prv_choose_motion(enemies, rift);
  prv_choose_spin(enemies, rift);
  prv_update_envelopes(enemies, rift);
  rift->motion_intensity = rift->target_motion_intensity;
  rift->turn_intensity = rift->target_turn_intensity;
  rift->target_length = rift->half_length;
  rift->length_timer = prv_random(enemies, 5, 20);
  rift->pivot = rift->target_pivot;
  rift->vx = rift->target_vx * rift->motion_intensity / 128;
  rift->vy = rift->target_vy * rift->motion_intensity / 128;
  rift->angular_velocity = rift->target_spin * rift->turn_intensity / 128;
  for (uint8_t i = 0; i < QB_RIFT_LINES; i++) {
    rift->strokes[i] = prv_stroke(rift->x_fp, rift->y_fp, rift->angle, rift->half_length);
  }
}

static void prv_add_runner_pair(QbEnemies *enemies) {
  if(enemies->runner_count+2>QB_MAX_RUNNERS) return;
  for(int side=0;side<2;side++) {
    const uint8_t index=enemies->runner_count++;
    enemies->runners[index]=(QbRunner){
      .cell={QB_GRID_W/2+(side?2:-2),0},.dir=side?QB_DIR_RIGHT:QB_DIR_LEFT,
      .hand=side?1:-1,.rng=prv_rng(enemies)
    };
  }
}

void enemies_init_seed(QbEnemies *enemies, uint8_t level, uint32_t seed) {
  memset(enemies, 0, sizeof(*enemies));
  enemies->rift_count = level >= 3 ? 2 : 1;
  enemies->aggression=level==2 || level==4?1:level>=7?3:level>=5?2:0;
  enemies->super_ticks_remaining = 30 * (1000 / QB_TIMER_MS);
  enemies->reinforcement_ticks = enemies->super_ticks_remaining;
  enemies->rng = seed ? seed : 1;
  prv_init_rift(enemies, &enemies->rifts[0], QB_GRID_W / 2, QB_GRID_H / 2);
  prv_init_rift(enemies, &enemies->rifts[1], QB_GRID_W / 3, QB_GRID_H / 3);
  prv_add_runner_pair(enemies);
  if(level>=5) {
    prv_add_runner_pair(enemies);
    enemies->runners[2].cell.x=QB_GRID_W/4;
    enemies->runners[3].cell.x=QB_GRID_W*3/4;
  }
  if(level>=7) {
    enemies->runners_super=true;
    enemies->super_ticks_remaining=0;
  }
}

void enemies_init(QbEnemies *enemies, uint8_t level) {
  enemies_init_seed(enemies, level, 0xC0FFEEUL + (uint32_t)level * 97UL);
}

QbCell enemies_get_rift_cell(const QbEnemies *enemies, uint8_t index) {
  if (index >= enemies->rift_count) return (QbCell){1, 1};
  return (QbCell){enemies->rifts[index].x_fp / FP_ONE, enemies->rifts[index].y_fp / FP_ONE};
}

QbCell enemies_get_runner_cell(const QbEnemies *enemies, uint8_t index) {
  return index < enemies->runner_count ? enemies->runners[index].cell : (QbCell){0, 0};
}

QbPixel enemies_get_runner_center(const QbEnemies *enemies,uint8_t index) {
  if(index>=enemies->runner_count) return (QbPixel){0,0};
  const QbRunner *runner=&enemies->runners[index];
  const QbCell from=runner->has_step?runner->previous_cell:runner->cell;
  const int progress=enemies->runner_subtick;
  return (QbPixel){qb_cell_center_px(from.x)+(runner->cell.x-from.x)*QB_CELL_PX*progress/QB_RUNNER_TICKS,
                   qb_cell_center_px(from.y)+(runner->cell.y-from.y)*QB_CELL_PX*progress/QB_RUNNER_TICKS};
}

uint8_t enemies_get_claim_cells(const QbEnemies *enemies, QbCell *out, uint8_t maximum) {
  const uint8_t count = enemies->rift_count < maximum ? enemies->rift_count : maximum;
  for (uint8_t i = 0; i < count; i++) out[i] = enemies_get_rift_cell(enemies, i);
  return count;
}

static bool prv_move_rift(QbEnemies *enemies, QbRift *rift,QbCell player,bool drawing,bool *trail_hit) {
  prv_update_envelopes(enemies, rift);
  if (!rift->turn_timer) {
    prv_choose_motion(enemies, rift);
    /* Aggressive stages occasionally bias a new heading, never lock onto the marker. */
    if(drawing && enemies->aggression && prv_rng(enemies)%8<enemies->aggression) {
      const int dx=player.x*FP_ONE+FP_ONE/2-rift->x_fp;
      const int dy=player.y*FP_ONE+FP_ONE/2-rift->y_fp;
      const int distance=qb_abs_i(dx)>qb_abs_i(dy)?qb_abs_i(dx):qb_abs_i(dy);
      if(distance) {
        rift->target_vx=(rift->target_vx+dx*MAX_ENDPOINT_STEP/distance)/2;
        rift->target_vy=(rift->target_vy+dy*MAX_ENDPOINT_STEP/distance)/2;
      }
    }
  }
  else rift->turn_timer--;
  if (!rift->spin_timer) prv_choose_spin(enemies, rift);
  else rift->spin_timer--;
  if (!rift->length_timer) {
    rift->target_length = qb_pixel_to_fp(prv_random(enemies, QB_RIFT_MIN_LENGTH_PX, QB_RIFT_MAX_LENGTH_PX)) / 2;
    rift->length_timer = prv_random(enemies, 10, 38);
  } else rift->length_timer--;

  const int cx = rift->x_fp / FP_ONE, cy = rift->y_fp / FP_ONE;
  const bool corner = (cx < QB_CORNER_CELLS || cx >= QB_GRID_W - QB_CORNER_CELLS) &&
                      (cy < QB_CORNER_CELLS || cy >= QB_GRID_H - QB_CORNER_CELLS);
  if (!corner) rift->corner_ticks = 0;
  else if (rift->corner_ticks < 255) rift->corner_ticks++;
  /* Quiet phases must not pin the band in a corner; every escape pose is still checked. */
  if (rift->corner_ticks > 45) {
    rift->target_vx = cx < QB_GRID_W / 2 ? 16 : -16;
    rift->target_vy = cy < QB_GRID_H / 2 ? 16 : -16;
    if (rift->motion_intensity < 80) rift->motion_intensity = 80;
    if (rift->target_motion_intensity < 80) rift->target_motion_intensity = 80;
    if (rift->turn_intensity > 32) rift->turn_intensity = 32;
    rift->turn_timer = 4;
  }

  int vx = prv_approach(rift->vx, rift->target_vx * rift->motion_intensity / 128, 4);
  int vy = prv_approach(rift->vy, rift->target_vy * rift->motion_intensity / 128, 4);
  int spin = prv_approach(rift->angular_velocity, rift->target_spin * rift->turn_intensity / 128, 4);
  int length = prv_approach(rift->half_length, rift->target_length, 6);
  rift->pivot = prv_approach(rift->pivot, rift->target_pivot, 5);
  const QbStroke previous = rift->strokes[0];
  QbStroke next = previous;
  int angle = (rift->angle + spin) & ANGLE_MASK;

  /* A rejected pose never replaces the valid visible history. No teleporting. */
  bool accepted = false;
  int avoidance_heading = 0;
  int attempt;
  for (attempt = 0; attempt < 18; attempt++) {
    if (attempt) {
      if (attempt == 1) avoidance_heading = prv_random(enemies, 0, ANGLE_MASK);
      const int heading = (avoidance_heading + attempt * 253) & ANGLE_MASK;
      const int speed = 4 + rift->motion_intensity * 8 / 128;
      vx = prv_sin(heading + 1024) * speed / 256;
      vy = prv_sin(heading) * speed / 256;
      if (attempt >= 9) {
        angle = rift->angle;
        spin = 0;
        length = rift->half_length > 18 ? rift->half_length - 2 : rift->half_length;
      }
    }
    next = prv_bounded_pose(rift, &vx, &vy, &spin, &length);
    angle = (rift->angle + spin) & ANGLE_MASK;
    const Space space = prv_swept_space(previous, next);
    if (attempt == 0 && (space & SPACE_TRAIL)) { *trail_hit = true; return false; }
    if (space & SPACE_WALL) continue;

    /* Look ahead only for walls, never steer in response to a player's trail. */
    const QbStroke ahead = {next.ax + vx * 4, next.ay + vy * 4,
                            next.bx + vx * 4, next.by + vy * 4};
    if (attempt < 9 && (prv_space(ahead) & SPACE_WALL)) continue;
    if (space & SPACE_TRAIL) { *trail_hit = true; return false; }
    accepted = true;
    break;
  }
  if (!accepted) {
    length = rift->half_length > 18 ? rift->half_length - 2 : rift->half_length;
    angle = rift->angle;
    spin = 0;
    vx = vy = 0;
    next = prv_pose(rift, 0, 0, 0, length);
    accepted = prv_swept_space(previous, next) == SPACE_OK;
  }
  if (accepted) {
    rift->x_fp = (next.ax + next.bx) / 2;
    rift->y_fp = (next.ay + next.by) / 2;
    rift->angle = angle;
    rift->half_length = length;
    rift->vx = vx;
    rift->vy = vy;
    rift->angular_velocity = spin;
    if (attempt) {
      rift->target_vx = vx;
      rift->target_vy = vy;
      rift->turn_timer = 7;
    }
  }
  for (int i = QB_RIFT_LINES - 1; i > 0; i--) rift->strokes[i] = rift->strokes[i - 1];
  rift->strokes[0] = accepted ? next : previous;
  return accepted;
}

static int prv_dx(QbDir dir) { return dir == QB_DIR_RIGHT ? 1 : dir == QB_DIR_LEFT ? -1 : 0; }
static int prv_dy(QbDir dir) { return dir == QB_DIR_DOWN ? 1 : dir == QB_DIR_UP ? -1 : 0; }
static QbDir prv_left(QbDir dir) { return dir == QB_DIR_UP ? QB_DIR_LEFT : (QbDir)(dir - 1); }
static QbDir prv_right(QbDir dir) { return dir == QB_DIR_LEFT ? QB_DIR_UP : (QbDir)(dir + 1); }

static bool prv_free_on_side(int x, int y, QbDir direction, int hand) {
  const QbDir side = hand < 0 ? prv_left(direction) : prv_right(direction);
  const int sx = prv_dx(side), sy = prv_dy(side);
  const int tx = prv_dx(direction), ty = prv_dy(direction);
  for (int offset = -1; offset <= 1; offset++) {
    if (grid_is_empty(x + sx + tx * offset, y + sy + ty * offset)) return true;
  }
  return false;
}

static bool prv_runner_step(QbRunner *runner, QbDir dir, bool super) {
  const int x = runner->cell.x + prv_dx(dir), y = runner->cell.y + prv_dy(dir);
  if (!grid_is_runner_path(x, y, super)) return false;
  runner->previous_cell=runner->cell;
  runner->has_step=true;
  runner->cell = (QbCell){x, y};
  runner->dir = dir;
  if (!prv_free_on_side(x, y, dir, runner->hand) &&
      prv_free_on_side(x, y, dir, -runner->hand)) runner->hand = -runner->hand;
  return true;
}

static bool prv_runner_option(const QbRunner *runner, QbDir dir, bool super) {
  const int x = runner->cell.x + prv_dx(dir), y = runner->cell.y + prv_dy(dir);
  if (!grid_is_runner_path(x, y, super)) return false;
  if (x == 0 || y == 0 || x == QB_GRID_W - 1 || y == QB_GRID_H - 1) return true;
  return prv_free_on_side(x, y, dir, -1) || prv_free_on_side(x, y, dir, 1) ||
         (super && grid_is_trail_state(grid_get(x, y)));
}

static void prv_move_runner(QbRunner *runner, bool super) {
  const QbDir inside = runner->hand < 0 ? prv_left(runner->dir) : prv_right(runner->dir);
  const QbDir outside = runner->hand < 0 ? prv_right(runner->dir) : prv_left(runner->dir);
  const QbDir dirs[] = {inside, runner->dir, outside,
                         prv_left(prv_left(runner->dir))};
  if (super) {
    for (uint8_t i = 0; i < 4; i++) {
      const int x = runner->cell.x + prv_dx(dirs[i]), y = runner->cell.y + prv_dy(dirs[i]);
      if (grid_in_bounds(x, y) && grid_is_trail_state(grid_get(x, y)) &&
          prv_runner_step(runner, dirs[i], true)) return;
    }
  }
  if (runner->junction_cooldown) {
    runner->junction_cooldown--;
    if (prv_runner_step(runner, runner->dir, super)) return;
  }

  const QbDir options[3] = {runner->dir, inside, outside};
  unsigned weights[3] = {0, 0, 0}, total = 0, count = 0;
  for (uint8_t i = 0; i < 3; i++) {
    if (prv_runner_option(runner, options[i], super)) {
      weights[i] = i == 0 ? 2 : 1;
      total += weights[i];
      count++;
    }
  }
  if (total) {
    unsigned choice = count > 1 ? prv_rng_state(&runner->rng) % total : 0;
    for (uint8_t i = 0; i < 3; i++) {
      if (choice < weights[i]) {
        if (count > 1) runner->junction_cooldown = 4;
        prv_runner_step(runner, options[i], super);
        return;
      }
      choice -= weights[i];
    }
  }

  for (uint8_t i = 0; i < 4; i++) if (prv_runner_step(runner, dirs[i], super)) return;
}

uint8_t enemies_runner_phase(const QbEnemies *enemies,uint8_t index) {
  static const uint8_t ends[8]={2,5,7,9,12,14,16,19};
  const unsigned tick=((unsigned)enemies->anim+index*7)%19;
  for(uint8_t phase=0;phase<8;phase++) if(tick<ends[phase]) return phase;
  return 0;
}

static void prv_runner_brush(int x,int y,uint8_t ink,QbRunnerPixelVisitor visitor,void *context) {
  for(int oy=0;oy<QB_RUNNER_BRUSH_PX;oy++)
    for(int ox=0;ox<QB_RUNNER_BRUSH_PX;ox++) visitor(x+ox,y+oy,ink,context);
}

void enemies_visit_runner_colored_pixels(const QbEnemies *enemies,uint8_t index,
                                          QbRunnerPixelVisitor visitor,void *context) {
  if (index >= enemies->runner_count) return;
  const QbPixel center=enemies_get_runner_center(enemies,index);
  const int cx=center.x,cy=center.y;
  const int phase=enemies_runner_phase(enemies,index);
  /* Short stepped curls leave the border visible through a changing open center. */
  static const int8_t rim[12][2]={
    {0,-2},{1,-2},{2,-1},{2,0},{2,1},{1,2},
    {0,2},{-1,2},{-2,1},{-2,0},{-2,-1},{-1,-2}
  };
  for(int curl=0;curl<3;curl++) {
    const int start=(phase+phase/2+curl*4+((phase&1) && curl==1))%12;
    const int length=2+((phase+curl)%3==0);
    for(int step=0;step<length;step++) {
      const int node=(start+step)%12;
      const uint8_t ink=(step+curl+phase/2)&1;
      prv_runner_brush(cx-1+2*rim[node][0],cy-1+2*rim[node][1],ink,visitor,context);
    }
    const int tip=(start+length-1)%12;
    int x=rim[tip][0],y=rim[tip][1];
    x-=x>0?1:x<0?-1:0;
    y-=y>0?1:y<0?-1:0;
    prv_runner_brush(cx-1+2*x,cy-1+2*y,(curl+phase)&1,visitor,context);
  }
}

typedef struct { QbRiftPixelVisitor visitor; void *context; } RunnerMask;

static void prv_runner_mask_pixel(int x,int y,uint8_t ink,void *context) {
  (void)ink;
  RunnerMask *mask=context;
  mask->visitor(x,y,mask->context);
}

void enemies_visit_runner_pixels(const QbEnemies *enemies,uint8_t index,
                                  QbRiftPixelVisitor visitor,void *context) {
  RunnerMask mask={visitor,context};
  enemies_visit_runner_colored_pixels(enemies,index,prv_runner_mask_pixel,&mask);
}

void enemies_update(QbEnemies *enemies, QbCell player, bool drawing, bool *trail_hit) {
  *trail_hit = enemies_trail_collision(enemies);
  if (*trail_hit) return;
  enemies->anim++;
  enemies->runner_subtick=(enemies->runner_subtick+1)%QB_RUNNER_TICKS;
  if (enemies->super_ticks_remaining > 0 && --enemies->super_ticks_remaining == 0) {
    if(enemies->runner_count<QB_MAX_RUNNERS) {
      prv_add_runner_pair(enemies);
      enemies->super_ticks_remaining=enemies->reinforcement_ticks;
    } else enemies->runners_super=true;
  }
  for (uint8_t i = 0; i < enemies->rift_count; i++) {
    prv_move_rift(enemies, &enemies->rifts[i],player,drawing,trail_hit);
    if (*trail_hit) return;
  }
  if (enemies->runner_subtick == 0) {
    for (uint8_t i = 0; i < enemies->runner_count; i++) {
      enemies->runners[i].previous_cell=enemies->runners[i].cell;
      enemies->runners[i].has_step=false;
      prv_move_runner(&enemies->runners[i], enemies->runners_super);
    }
  }
}

typedef struct { int x, y; bool hit; } RunnerContact;

static void prv_runner_contact(int x, int y, void *context) {
  RunnerContact *contact = context;
  if (qb_abs_i(x - contact->x) + qb_abs_i(y - contact->y) <= QB_PLAYER_RADIUS) contact->hit = true;
}

bool enemies_player_collision(const QbEnemies *enemies, QbCell player, bool drawing) {
  if (drawing && enemies_trail_collision(enemies)) return true;
  for (uint8_t i = 0; i < enemies->runner_count; i++) {
    RunnerContact contact = {qb_cell_center_px(player.x), qb_cell_center_px(player.y), false};
    enemies_visit_runner_pixels(enemies, i, prv_runner_contact, &contact);
    if (contact.hit) return true;
  }
  return false;
}
