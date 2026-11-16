#include <assert.h>
#include <stdio.h>

#include "enemies.h"

int main(void) {
  grid_init_level();

  QbEnemies enemies;
  enemies_init(&enemies, 1);

  const QbCell player = {QB_GRID_W / 2, QB_GRID_H - 1};
  const QbCell left = enemies_get_runner_cell(&enemies, 0);
  const QbCell right = enemies_get_runner_cell(&enemies, 1);

  assert(enemies.runner_count == 2);
  assert(left.y == 0);
  assert(right.y == 0);
  assert(left.x < QB_GRID_W / 2);
  assert(right.x > QB_GRID_W / 2);
  assert((right.x-left.x)*QB_CELL_PX>2*QB_RUNNER_RADIUS);
  assert(!enemies_player_collision(&enemies, player, false));

  puts("start positions ok");
  return 0;
}
