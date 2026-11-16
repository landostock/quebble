#ifndef QUEBBLE_CONFIG_H
#define QUEBBLE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define QB_APP_NAME "Quebble"

#define QB_SCREEN_W 200
#define QB_SCREEN_H 228
#define QB_HUD_H 38
#define QB_CELL_PX 3
#define QB_BOARD_MARGIN 4
#define QB_BOARD_X QB_BOARD_MARGIN
#define QB_BOARD_Y (QB_HUD_H + QB_BOARD_MARGIN)
#define QB_BOARD_W (((QB_SCREEN_W - 2 * QB_BOARD_MARGIN) / QB_CELL_PX) * QB_CELL_PX)
#define QB_BOARD_H (((QB_SCREEN_H - QB_HUD_H - 2 * QB_BOARD_MARGIN) / QB_CELL_PX) * QB_CELL_PX)

#define QB_GRID_W (QB_BOARD_W / QB_CELL_PX)
#define QB_GRID_H (QB_BOARD_H / QB_CELL_PX)
#define QB_TRAIL_MAX 768

#define QB_TIMER_MS 33
#define QB_PLAYER_FAST_TICKS 3
#define QB_PLAYER_SLOW_TICKS 5
#define QB_RUNNER_TICKS 6
#define QB_PLAYER_RADIUS 5
#define QB_RUNNER_RADIUS 5
#define QB_RUNNER_BRUSH_PX 2
#define QB_RIFT_BRUSH_PX 2
#define QB_RIFT_MAX_STEP_PX 4
#define QB_RIFT_MIN_LENGTH_PX 18
#define QB_RIFT_MAX_LENGTH_PX 64
#define QB_FP_ONE 16
#define QB_CORNER_CELLS 8
#define QB_FUSE_DELAY_TICKS 21
#define QB_LEVEL_CARD_TICKS 45
#define QB_DEATH_CARD_TICKS 42

#define QB_TARGET_BASE 75

typedef enum {
  QB_DIR_NONE = 0,
  QB_DIR_UP,
  QB_DIR_RIGHT,
  QB_DIR_DOWN,
  QB_DIR_LEFT
} QbDir;

typedef struct {
  uint8_t x;
  uint8_t y;
} QbCell;

typedef struct {
  int16_t x;
  int16_t y;
} QbPixel;

static inline int qb_abs_i(int v) {
  return v < 0 ? -v : v;
}

static inline int qb_fp_to_pixel(int fp) { return fp * QB_CELL_PX / QB_FP_ONE; }
static inline int qb_pixel_to_fp(int px) { return px * QB_FP_ONE / QB_CELL_PX; }
static inline int qb_cell_center_px(int cell) { return cell * QB_CELL_PX + QB_CELL_PX / 2; }

#endif
