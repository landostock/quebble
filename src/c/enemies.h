#ifndef ENEMIES_H
#define ENEMIES_H

#include "grid.h"

#define QB_MAX_RIFTS 2
#define QB_RIFT_LINES 7
#define QB_RIFT_POINTS 2
#define QB_MAX_RUNNERS 4

typedef struct {
  int16_t ax, ay, bx, by;
} QbStroke;

typedef struct {
  int16_t x_fp;
  int16_t y_fp;
  int16_t vx, vy;
  int16_t target_vx, target_vy;
  uint16_t angle;
  int16_t angular_velocity;
  int16_t target_spin;
  int16_t half_length;
  int16_t target_length;
  int16_t pivot, target_pivot;
  uint8_t turn_timer;
  uint8_t spin_timer;
  uint8_t length_timer;
  uint8_t motion_intensity, target_motion_intensity;
  uint8_t turn_intensity, target_turn_intensity;
  uint8_t motion_envelope_timer, turn_envelope_timer;
  uint8_t corner_ticks;
  QbStroke strokes[QB_RIFT_LINES];
} QbRift;

typedef struct {
  QbCell cell;
  QbCell previous_cell;
  bool has_step;
  QbDir dir;
  int8_t hand;
  uint8_t junction_cooldown;
  uint32_t rng;
} QbRunner;

typedef struct {
  QbRift rifts[QB_MAX_RIFTS];
  QbRunner runners[QB_MAX_RUNNERS];
  uint8_t rift_count;
  uint8_t runner_count;
  uint8_t aggression;
  uint8_t runner_subtick;
  bool runners_super;
  int super_ticks_remaining;
  int reinforcement_ticks;
  uint16_t anim;
  uint32_t rng;
} QbEnemies;

void enemies_init(QbEnemies *enemies, uint8_t level);
void enemies_init_seed(QbEnemies *enemies, uint8_t level, uint32_t seed);
void enemies_update(QbEnemies *enemies, QbCell player, bool drawing, bool *trail_hit);
uint8_t enemies_get_claim_cells(const QbEnemies *enemies, QbCell *out_cells, uint8_t max_cells);
QbCell enemies_get_rift_cell(const QbEnemies *enemies, uint8_t index);
QbCell enemies_get_runner_cell(const QbEnemies *enemies, uint8_t index);
QbPixel enemies_get_runner_center(const QbEnemies *enemies, uint8_t index);
QbPixel enemies_get_rift_visual_point(const QbRift *rift, uint8_t line, uint8_t point);
typedef void (*QbRiftPixelVisitor)(int x, int y, void *context);
/* Ink 0 is the bright body color; ink 1 is its contrasting accent. */
typedef void (*QbRunnerPixelVisitor)(int x, int y, uint8_t ink, void *context);
void enemies_visit_rift_line(const QbRift *rift, uint8_t line,
                             QbRiftPixelVisitor visitor, void *context);
void enemies_visit_runner_pixels(const QbEnemies *enemies, uint8_t index,
                                  QbRiftPixelVisitor visitor, void *context);
void enemies_visit_runner_colored_pixels(const QbEnemies *enemies, uint8_t index,
                                         QbRunnerPixelVisitor visitor, void *context);
uint8_t enemies_runner_phase(const QbEnemies *enemies, uint8_t index);
bool enemies_trail_collision(const QbEnemies *enemies);
bool enemies_player_collision(const QbEnemies *enemies, QbCell player, bool drawing);

#endif
