#ifndef GAME_H
#define GAME_H

#include <pebble.h>

#include "enemies.h"
#include "difficulty.h"

#define QB_GUIDE_PAGES 3

typedef enum {
  GAME_SCREEN_TITLE = 0,
  GAME_SCREEN_LEVEL,
  GAME_SCREEN_COUNTDOWN,
  GAME_SCREEN_PLAYING,
  GAME_SCREEN_CLAIM_PAUSE,
  GAME_SCREEN_DEATH,
  GAME_SCREEN_LEVEL_CLEAR,
  GAME_SCREEN_GAME_OVER,
  GAME_SCREEN_HELP,
  GAME_SCREEN_SOUND,
  GAME_SCREEN_CREDITS,
  GAME_SCREEN_DIFFICULTY
} QbGameScreen;

typedef struct {
  QbGameScreen screen;
  uint8_t level;
  uint8_t lives;
  uint8_t target_percent;
  uint8_t claimed_percent;
  uint8_t multiplier;
  bool slow_mode;
  bool moving;
  bool drawing;
  bool cut_slow;
  bool runners_super;
  int super_seconds;
  uint8_t countdown;
  uint16_t claim_cells;
  uint16_t menu_ticks;
  uint16_t transition_ticks;
  bool music_enabled,effects_enabled,haptics_enabled;
  uint8_t volume,sound_row,help_page;
  QbDifficulty difficulty;
  QbDir direction;
  bool choosing_path,edge_dead_end;
  QbDir upper_path,lower_path;
  int32_t score;
  int32_t highscore;
} QbGameView;

void game_init(void);
void game_deinit(void);
void game_update(void);
/* Buttons: stop; when stopped go straight, or back if straight is blocked. */
void game_stop_go(void);
/* Buttons: relative quarter turns. A long press also selects slow mode. */
void game_turn_left(bool slow);
void game_turn_right(bool slow);
void game_toggle_slow(void);
void game_back_to_title(void);
void game_tap(void);
void game_touch_tap(int x, int y);
void game_set_direction(QbDir direction);
bool game_backlight_required(void);

QbGameView game_view(void);
QbCell game_player_cell(void);
bool game_fuse_cell(QbCell *out_cell);
const QbEnemies *game_enemies(void);
uint16_t game_anim(void);

#endif
