#include "grid.h"

#include <string.h>

#define CELL_REACHABLE 240
#define CELL_FIRST_REACH 241

static uint8_t s_grid[QB_GRID_H][QB_GRID_W];
static int16_t s_queue[QB_GRID_W * QB_GRID_H];
static uint8_t s_seen[(QB_GRID_W * QB_GRID_H + 7) / 8];
static int s_claimed_cells;

static bool prv_is_grid_cell(uint8_t state) {
  return state == CELL_EMPTY || state == CELL_CLAIM_FAST ||
         state == CELL_CLAIM_SLOW || state == CELL_BORDER ||
         state == CELL_TRAIL_FAST || state == CELL_TRAIL_SLOW;
}

bool grid_in_bounds(int x, int y) {
  return x >= 0 && y >= 0 && x < QB_GRID_W && y < QB_GRID_H;
}

QbCellState grid_get(int x, int y) {
  if (!grid_in_bounds(x, y)) {
    return CELL_BORDER;
  }
  return (QbCellState)s_grid[y][x];
}

bool grid_is_claimed_state(QbCellState state) {
  return state == CELL_CLAIM_FAST || state == CELL_CLAIM_SLOW;
}

bool grid_is_safe_state(QbCellState state) {
  return state == CELL_BORDER || grid_is_claimed_state(state);
}

bool grid_is_trail_state(QbCellState state) {
  return state == CELL_TRAIL_FAST || state == CELL_TRAIL_SLOW;
}

bool grid_is_empty(int x, int y) {
  return grid_in_bounds(x, y) && s_grid[y][x] == CELL_EMPTY;
}

bool grid_is_safe(int x, int y) {
  return grid_in_bounds(x, y) && grid_is_safe_state((QbCellState)s_grid[y][x]);
}

static bool prv_is_outer_border(int x, int y) {
  return x == 0 || y == 0 || x == QB_GRID_W - 1 || y == QB_GRID_H - 1;
}

static bool prv_has_adjacent_state(int x, int y, QbCellState state) {
  const int dx[4] = {1, -1, 0, 0};
  const int dy[4] = {0, 0, 1, -1};
  for (int i = 0; i < 4; i++) {
    const int nx = x + dx[i];
    const int ny = y + dy[i];
    if (grid_in_bounds(nx, ny) && s_grid[ny][nx] == state) {
      return true;
    }
  }
  return false;
}

static bool prv_is_exposed_safe_edge(int x, int y) {
  if (!grid_in_bounds(x, y)) {
    return false;
  }
  const QbCellState state = (QbCellState)s_grid[y][x];
  if (!grid_is_safe_state(state)) return false;
  if (state == CELL_BORDER && prv_is_outer_border(x, y)) return true;
  if (prv_has_adjacent_state(x, y, CELL_EMPTY)) return true;
  /* A re-entrant corner needs its diagonal connector for a 4-way marker. */
  for (int dy = -1; dy <= 1; dy += 2) {
    for (int dx = -1; dx <= 1; dx += 2) {
      if (grid_is_empty(x + dx, y + dy)) return true;
    }
  }
  return false;
}

bool grid_is_player_path(int x, int y) {
  return prv_is_exposed_safe_edge(x, y);
}

bool grid_nearest_frontier(QbCell from, QbCell *out) {
  if (!grid_is_safe(from.x, from.y)) return false;
  memset(s_seen, 0, sizeof(s_seen));
  int head = 0, tail = 0;
  int start = from.y * QB_GRID_W + from.x;
  s_queue[tail++] = start;
  s_seen[start / 8] |= 1U << (start % 8);
  const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
  while (head < tail) {
    const int index = s_queue[head++], x = index % QB_GRID_W, y = index / QB_GRID_W;
    if (grid_is_player_path(x, y)) {
      *out = (QbCell){x, y};
      return true;
    }
    for (int i = 0; i < 4; i++) {
      const int nx = x + dx[i], ny = y + dy[i];
      if (!grid_is_safe(nx, ny)) continue;
      const int next = ny * QB_GRID_W + nx;
      const uint8_t bit = 1U << (next % 8);
      if (s_seen[next / 8] & bit) continue;
      s_seen[next / 8] |= bit;
      s_queue[tail++] = next;
    }
  }
  return false;
}

bool grid_is_runner_path(int x, int y, bool super_runner) {
  if (!grid_in_bounds(x, y)) {
    return false;
  }
  if (prv_is_exposed_safe_edge(x, y)) {
    return true;
  }
  return super_runner && grid_is_trail_state((QbCellState)s_grid[y][x]);
}

int grid_total_claimable(void) {
  return (QB_GRID_W - 2) * (QB_GRID_H - 2);
}

int grid_claimed_cells(void) {
  return s_claimed_cells;
}

int grid_claimed_percent(void) {
  const int total = grid_total_claimable();
  return total > 0 ? (s_claimed_cells * 100) / total : 0;
}

void grid_init_level(void) {
  memset(s_grid, CELL_EMPTY, sizeof(s_grid));
  s_claimed_cells = 0;

  for (int x = 0; x < QB_GRID_W; x++) {
    s_grid[0][x] = CELL_BORDER;
    s_grid[QB_GRID_H - 1][x] = CELL_BORDER;
  }
  for (int y = 0; y < QB_GRID_H; y++) {
    s_grid[y][0] = CELL_BORDER;
    s_grid[y][QB_GRID_W - 1] = CELL_BORDER;
  }
}

void grid_set_trail(int x, int y, bool slow) {
  if (grid_in_bounds(x, y) && s_grid[y][x] == CELL_EMPTY) {
    s_grid[y][x] = slow ? CELL_TRAIL_SLOW : CELL_TRAIL_FAST;
  }
}

void grid_clear_trails(void) {
  for (int y = 0; y < QB_GRID_H; y++) {
    for (int x = 0; x < QB_GRID_W; x++) {
      if (grid_is_trail_state((QbCellState)s_grid[y][x])) {
        s_grid[y][x] = CELL_EMPTY;
      }
    }
  }
}

void grid_commit_trails_to_border(void) {
  for (int y = 0; y < QB_GRID_H; y++) {
    for (int x = 0; x < QB_GRID_W; x++) {
      if (grid_is_trail_state((QbCellState)s_grid[y][x])) {
        s_grid[y][x] = CELL_BORDER;
      }
    }
  }
}

static int prv_flood_mark(int sx, int sy, uint8_t target, uint8_t mark) {
  if (!grid_in_bounds(sx, sy) || s_grid[sy][sx] != target) {
    return 0;
  }

  int head = 0;
  int tail = 0;
  int count = 0;
  s_queue[tail++] = (int16_t)(sy * QB_GRID_W + sx);
  s_grid[sy][sx] = mark;

  while (head < tail) {
    const int idx = s_queue[head++];
    const int x = idx % QB_GRID_W;
    const int y = idx / QB_GRID_W;
    count++;

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; i++) {
      const int nx = x + dx[i];
      const int ny = y + dy[i];
      if (!grid_in_bounds(nx, ny) || s_grid[ny][nx] != target) {
        continue;
      }
      s_grid[ny][nx] = mark;
      s_queue[tail++] = (int16_t)(ny * QB_GRID_W + nx);
    }
  }

  return count;
}

static void prv_restore_mark(uint8_t mark, uint8_t state) {
  for (int y = 0; y < QB_GRID_H; y++) {
    for (int x = 0; x < QB_GRID_W; x++) {
      if (s_grid[y][x] == mark) {
        s_grid[y][x] = state;
      }
    }
  }
}

static bool prv_enemies_separated(const QbCell *enemies, uint8_t enemy_count) {
  if (enemy_count < 2) {
    return false;
  }
  const QbCell first = enemies[0];
  const QbCell second = enemies[1];
  if (!grid_in_bounds(first.x, first.y) || !grid_in_bounds(second.x, second.y)) {
    return false;
  }
  if (s_grid[first.y][first.x] != CELL_EMPTY || s_grid[second.y][second.x] != CELL_EMPTY) {
    return false;
  }

  prv_flood_mark(first.x, first.y, CELL_EMPTY, CELL_FIRST_REACH);
  const bool connected = s_grid[second.y][second.x] == CELL_FIRST_REACH;
  prv_restore_mark(CELL_FIRST_REACH, CELL_EMPTY);
  return !connected;
}

QbClaimResult grid_claim_from_enemies(const QbCell *enemies, uint8_t enemy_count, bool slow_fill) {
  return grid_claim_with_footprint(enemies, enemy_count, NULL, 0, slow_fill);
}

QbClaimResult grid_claim_with_footprint(const QbCell *enemies, uint8_t enemy_count,
                                      const QbCell *occupied, uint8_t occupied_count, bool slow_fill) {
  QbClaimResult result = {
    .new_claimed = 0,
    .total_claimed = s_claimed_cells,
    .claimed_percent = grid_claimed_percent(),
    .split_enemies = false
  };

  result.split_enemies = prv_enemies_separated(enemies, enemy_count);

  for (uint8_t i = 0; i < enemy_count; i++) {
    if (grid_in_bounds(enemies[i].x, enemies[i].y)) {
      prv_flood_mark(enemies[i].x, enemies[i].y, CELL_EMPTY, CELL_REACHABLE);
    }
  }

  for (uint8_t i = 0; i < occupied_count; i++) {
    prv_flood_mark(occupied[i].x, occupied[i].y, CELL_EMPTY, CELL_REACHABLE);
  }

  const uint8_t fill_state = slow_fill ? CELL_CLAIM_SLOW : CELL_CLAIM_FAST;
  for (int y = 1; y < QB_GRID_H - 1; y++) {
    for (int x = 1; x < QB_GRID_W - 1; x++) {
      if (s_grid[y][x] == CELL_EMPTY) {
        s_grid[y][x] = fill_state;
        s_claimed_cells++;
        result.new_claimed++;
      } else if (s_grid[y][x] == CELL_REACHABLE) {
        s_grid[y][x] = CELL_EMPTY;
      } else if (!prv_is_grid_cell(s_grid[y][x])) {
        s_grid[y][x] = CELL_EMPTY;
      }
    }
  }
  result.total_claimed = s_claimed_cells;
  result.claimed_percent = grid_claimed_percent();
  return result;
}

#ifdef QUEBBLE_GRID_TEST
void grid_test_clear_all(QbCellState state) {
  memset(s_grid, state, sizeof(s_grid));
  s_claimed_cells = grid_is_claimed_state(state) ? grid_total_claimable() : 0;
}

void grid_test_set(int x, int y, QbCellState state) {
  if (!grid_in_bounds(x, y)) {
    return;
  }
  const QbCellState old = (QbCellState)s_grid[y][x];
  if (grid_is_claimed_state(old) && !grid_is_claimed_state(state)) {
    s_claimed_cells--;
  } else if (!grid_is_claimed_state(old) && grid_is_claimed_state(state)) {
    s_claimed_cells++;
  }
  s_grid[y][x] = state;
}

int grid_test_count(QbCellState state) {
  int count = 0;
  for (int y = 0; y < QB_GRID_H; y++) {
    for (int x = 0; x < QB_GRID_W; x++) {
      if (s_grid[y][x] == state) {
        count++;
      }
    }
  }
  return count;
}
#endif
