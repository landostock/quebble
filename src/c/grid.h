#ifndef GRID_H
#define GRID_H

#include "quebble_config.h"

typedef enum {
  CELL_EMPTY = 0,
  CELL_CLAIM_FAST = 1,
  CELL_CLAIM_SLOW = 2,
  CELL_BORDER = 3,
  CELL_TRAIL_FAST = 4,
  CELL_TRAIL_SLOW = 5
} QbCellState;

typedef struct {
  int new_claimed;
  int total_claimed;
  int claimed_percent;
  bool split_enemies;
} QbClaimResult;

void grid_init_level(void);
bool grid_in_bounds(int x, int y);
QbCellState grid_get(int x, int y);
void grid_set_trail(int x, int y, bool slow);
void grid_clear_trails(void);
void grid_commit_trails_to_border(void);
bool grid_is_safe_state(QbCellState state);
bool grid_is_claimed_state(QbCellState state);
bool grid_is_trail_state(QbCellState state);
bool grid_is_empty(int x, int y);
bool grid_is_safe(int x, int y);
bool grid_is_player_path(int x, int y);
bool grid_nearest_frontier(QbCell from, QbCell *out);
bool grid_is_runner_path(int x, int y, bool super_runner);
int grid_total_claimable(void);
int grid_claimed_cells(void);
int grid_claimed_percent(void);
QbClaimResult grid_claim_from_enemies(const QbCell *enemies, uint8_t enemy_count, bool slow_fill);
QbClaimResult grid_claim_with_footprint(const QbCell *enemies, uint8_t enemy_count,
                                      const QbCell *occupied, uint8_t occupied_count, bool slow_fill);

#ifdef QUEBBLE_GRID_TEST
void grid_test_clear_all(QbCellState state);
void grid_test_set(int x, int y, QbCellState state);
int grid_test_count(QbCellState state);
#endif

#endif
