#include <assert.h>
#include <stdio.h>

#include "grid.h"

static void border_all(void) {
  grid_test_clear_all(CELL_EMPTY);
  for (int x = 0; x < QB_GRID_W; x++) {
    grid_test_set(x, 0, CELL_BORDER);
    grid_test_set(x, QB_GRID_H - 1, CELL_BORDER);
  }
  for (int y = 0; y < QB_GRID_H; y++) {
    grid_test_set(0, y, CELL_BORDER);
    grid_test_set(QB_GRID_W - 1, y, CELL_BORDER);
  }
}

static void test_claims_region_opposite_enemy(void) {
  border_all();
  const int wall_x = QB_GRID_W / 2;
  for (int y = 1; y < QB_GRID_H - 1; y++) {
    grid_test_set(wall_x, y, CELL_BORDER);
  }

  QbCell enemy = {(uint8_t)(wall_x - 2), 2};
  QbClaimResult result = grid_claim_from_enemies(&enemy, 1, false);

  assert(result.new_claimed > 100);
  assert(grid_get(wall_x + 2, 2) == CELL_CLAIM_FAST);
  assert(grid_get(wall_x - 2, 2) == CELL_EMPTY);
}

static void test_slow_claim_uses_slow_cells(void) {
  border_all();
  const int wall_y = QB_GRID_H / 2;
  for (int x = 1; x < QB_GRID_W - 1; x++) {
    grid_test_set(x, wall_y, CELL_BORDER);
  }

  QbCell enemy = {2, (uint8_t)(wall_y - 2)};
  QbClaimResult result = grid_claim_from_enemies(&enemy, 1, true);

  assert(result.new_claimed > 100);
  assert(grid_get(2, wall_y + 2) == CELL_CLAIM_SLOW);
}

static void test_detects_split_enemies(void) {
  border_all();
  const int wall_x = QB_GRID_W / 2;
  for (int y = 1; y < QB_GRID_H - 1; y++) {
    grid_test_set(wall_x, y, CELL_BORDER);
  }

  QbCell enemies[2] = {
    {(uint8_t)(wall_x - 2), 2},
    {(uint8_t)(wall_x + 2), 2}
  };
  QbClaimResult result = grid_claim_from_enemies(enemies, 2, false);

  assert(result.split_enemies);
  assert(result.new_claimed == 0);
}

static void test_keeps_old_borders_visible(void) {
  border_all();
  const int wall_x = QB_GRID_W / 2;
  for (int y = 1; y < QB_GRID_H - 1; y++) {
    grid_test_set(wall_x, y, CELL_BORDER);
  }

  QbCell enemy = {(uint8_t)(wall_x - 2), 2};
  (void)grid_claim_from_enemies(&enemy, 1, false);

  assert(grid_get(wall_x, QB_GRID_H - 3) == CELL_BORDER);
  assert(grid_is_player_path(wall_x, 2));
}

static void test_claimed_edge_is_safe_path(void) {
  border_all();
  const int x = QB_GRID_W / 2;
  const int y = QB_GRID_H / 2;

  grid_test_set(x, y, CELL_CLAIM_FAST);
  grid_test_set(x + 1, y, CELL_CLAIM_FAST);
  grid_test_set(x - 1, y, CELL_EMPTY);

  assert(grid_is_player_path(x, y));

  grid_test_set(x - 1, y, CELL_CLAIM_FAST);
  grid_test_set(x, y - 1, CELL_CLAIM_FAST);
  grid_test_set(x, y + 1, CELL_CLAIM_FAST);
  assert(grid_is_player_path(x, y));
  for (int dy = -1; dy <= 1; dy += 2)
    for (int dx = -1; dx <= 1; dx += 2) grid_test_set(x + dx, y + dy, CELL_CLAIM_FAST);
  assert(!grid_is_player_path(x, y));
}

static void test_preserves_historical_occupied_region(void) {
  border_all();
  for (int y = 1; y < QB_GRID_H - 1; y++) grid_test_set(QB_GRID_W/2, y, CELL_BORDER);
  const QbCell head = {QB_GRID_W*3/4, QB_GRID_H/2};
  const QbCell history = {QB_GRID_W/4, QB_GRID_H/2};
  QbClaimResult result = grid_claim_with_footprint(&head, 1, &history, 1, false);
  assert(result.new_claimed == 0);
  assert(!result.split_enemies);
  assert(grid_get(history.x, history.y) == CELL_EMPTY);
  assert(grid_get(head.x, head.y) == CELL_EMPTY);
}

int main(void) {
  test_claims_region_opposite_enemy();
  test_slow_claim_uses_slow_cells();
  test_detects_split_enemies();
  test_keeps_old_borders_visible();
  test_claimed_edge_is_safe_path();
  test_preserves_historical_occupied_region();
  puts("floodfill ok");
  return 0;
}
