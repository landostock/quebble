#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enemies.h"

static unsigned char image[QB_SCREEN_H][QB_SCREEN_W][3];
static const unsigned char colors[4][3] = {{255,0,0},{255,255,0},{0,255,0},{0,0,255}};

static void pixel(int x, int y, void *context) {
  const unsigned char *color = context;
  assert(grid_get(x / QB_CELL_PX, y / QB_CELL_PX) == CELL_EMPTY);
  assert(x >= 0 && x < QB_BOARD_W && y >= 0 && y < QB_BOARD_H);
  memcpy(image[y + QB_BOARD_Y][x + QB_BOARD_X], color, 3);
}

int main(int argc, char **argv) {
  if (argc != 2) return 1;
  grid_init_level();
  for (int y = QB_GRID_H*3/4; y < QB_GRID_H - 1; y++)
    for (int x = 1; x < QB_GRID_W/3; x++) grid_test_set(x, y, CELL_CLAIM_FAST);
  QbEnemies e;
  enemies_init_seed(&e, 1, 9187);
  for (int frame = 0; frame < 600; frame++) {
    memset(image, 0, sizeof(image));
    for (int y = 0; y < QB_BOARD_H; y++) {
      for (int x = 0; x < QB_BOARD_W; x++) {
        const QbCellState state = grid_get(x / QB_CELL_PX, y / QB_CELL_PX);
        if (state == CELL_BORDER) memset(image[y + QB_BOARD_Y][x + QB_BOARD_X], 255, 3);
        else if (state == CELL_CLAIM_FAST) {
          image[y + QB_BOARD_Y][x + QB_BOARD_X][1] = 170;
          image[y + QB_BOARD_Y][x + QB_BOARD_X][2] = 255;
        }
      }
    }
    for (int age = QB_RIFT_LINES - 1; age >= 0; age--)
      enemies_visit_rift_line(&e.rifts[0], age, pixel, (void *)colors[(age + e.anim / 8) % 4]);
    char path[512];
    snprintf(path, sizeof(path), "%s/frame_%04d.ppm", argv[1], frame);
    FILE *out = fopen(path, "wb");
    if (!out) return 2;
    fprintf(out, "P6\n%d %d\n255\n", QB_SCREEN_W, QB_SCREEN_H);
    fwrite(image, 1, sizeof(image), out);
    fclose(out);
    bool hit;
    enemies_update(&e, (QbCell){50, 102}, false, &hit);
    assert(!hit);
  }
  return 0;
}
