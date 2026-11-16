#include "game.h"

#include <string.h>
#include <time.h>

#define KEY_HIGHSCORE 1001
#define KEY_SOUND 1002
#define KEY_DIFFICULTY 1003

typedef enum {
  PENDING_NONE, PENDING_LEFT_TURN, PENDING_RIGHT_TURN, PENDING_SLOW_LEFT,
  PENDING_SLOW_RIGHT, PENDING_STOP, PENDING_GO, PENDING_TOUCH_REVERSE, PENDING_UP, PENDING_RIGHT,
  PENDING_DOWN, PENDING_LEFT
} QbPendingAction;

typedef struct {
  QbGameScreen screen;
  uint8_t level;
  uint8_t lives;
  uint8_t target_percent;
  uint8_t multiplier;
  bool slow_mode;
  bool moving;
  bool drawing;
  bool trail_slow;
  QbDir dir;
  QbCell player;
  QbCell trail[QB_TRAIL_MAX];
  uint16_t trail_len;
  uint16_t fuse_index;
  uint16_t tick;
  uint16_t screen_ticks;
  uint16_t last_move_tick;
  uint16_t claim_cells;
  bool edge_follow;
  bool touch_controls;
  QbDir arrival_dir,edge_choice,pending_heading;
  QbPendingAction pending_action;
  QbDifficulty difficulty;
  uint16_t menu_ticks;
  bool music_enabled,effects_enabled,haptics_enabled;
  uint8_t volume,sound_row,help_page;
  int32_t score;
  int32_t highscore;
  QbEnemies enemies;
} QbGame;

static QbGame s_game;
static uint32_t s_round_seed;

static void prv_open_help(void) {
  s_game.screen=GAME_SCREEN_HELP;
  s_game.help_page=0;
  s_game.menu_ticks=0;
}

static void prv_help_page(int step) {
  int page=s_game.help_page+step;
  if(page<0 || page>=QB_GUIDE_PAGES) return;
  s_game.help_page=(uint8_t)page;
  s_game.menu_ticks=0;
}

static QbDir prv_turn_left(QbDir dir);
static QbDir prv_turn_right(QbDir dir);
static int prv_dx(QbDir dir);
static int prv_dy(QbDir dir);
static void prv_button_heading(QbDir direction,bool slow);
static void prv_button_resume(void);

static QbDir prv_reverse(QbDir dir) {
  return dir==QB_DIR_NONE?QB_DIR_NONE:prv_turn_left(prv_turn_left(dir));
}

static void prv_ensure_frontier(void) {
  QbCell edge;
  if (!grid_is_player_path(s_game.player.x, s_game.player.y) &&
      grid_nearest_frontier(s_game.player, &edge)) {
    s_game.player=edge;
    s_game.arrival_dir=s_game.edge_choice=QB_DIR_NONE;
  }
}

static bool prv_safe_exit(QbDir dir) {
  return dir!=QB_DIR_NONE && grid_is_player_path(s_game.player.x+prv_dx(dir),
                                                s_game.player.y+prv_dy(dir));
}

static unsigned prv_forward_exits(bool cut,QbDir options[4]) {
  /* Screen order is independent of approach and of the preview heading. */
  const QbDir order[]={QB_DIR_UP,QB_DIR_LEFT,QB_DIR_RIGHT,QB_DIR_DOWN};
  const QbDir back=prv_reverse(s_game.arrival_dir);
  unsigned count=0;
  for(unsigned i=0;i<ARRAY_LENGTH(order);i++) {
    QbDir dir=order[i];
    if(dir==back) continue;
    int x=s_game.player.x+prv_dx(dir),y=s_game.player.y+prv_dy(dir);
    if(cut?grid_is_empty(x,y):prv_safe_exit(dir)) options[count++]=dir;
  }
  return count;
}

static void prv_continue_on_edge(void) {
  prv_ensure_frontier();
  if(!s_game.touch_controls) {
    s_game.edge_choice=QB_DIR_NONE;
    s_game.moving=prv_safe_exit(s_game.dir);
    if(s_game.moving) return;
    QbDir options[4];
    if(prv_forward_exits(false,options)==1) {
      s_game.dir=options[0];
      s_game.moving=true;
    }
    return;
  }
  if(prv_safe_exit(s_game.edge_choice)) {
    s_game.dir=s_game.edge_choice;
    s_game.moving=true;
    return;
  }
  s_game.edge_choice=QB_DIR_NONE;
  QbDir options[4];
  const unsigned count=prv_forward_exits(false,options);
  s_game.moving=count==1;
  if(count==1) s_game.dir=options[0];
  else if(s_game.arrival_dir!=QB_DIR_NONE) s_game.dir=s_game.arrival_dir;
}

static void prv_apply_pending(void) {
  QbPendingAction action=s_game.pending_action;
  s_game.pending_action=PENDING_NONE;
  switch(action) {
    case PENDING_LEFT_TURN:
    case PENDING_RIGHT_TURN: prv_button_heading(s_game.pending_heading,false); break;
    case PENDING_SLOW_LEFT:
    case PENDING_SLOW_RIGHT: prv_button_heading(s_game.pending_heading,true); break;
    case PENDING_STOP: s_game.moving=false; break;
    case PENDING_GO:
      s_game.dir=s_game.pending_heading;
      prv_button_resume();
      break;
    case PENDING_TOUCH_REVERSE: game_tap(); break;
    default:
      if(action>=PENDING_UP && action<=PENDING_LEFT)
        game_set_direction((QbDir)(QB_DIR_UP+action-PENDING_UP));
      break;
  }
}

static int prv_dx(QbDir dir) {
  return dir == QB_DIR_RIGHT ? 1 : (dir == QB_DIR_LEFT ? -1 : 0);
}

static int prv_dy(QbDir dir) {
  return dir == QB_DIR_DOWN ? 1 : (dir == QB_DIR_UP ? -1 : 0);
}

static QbDir prv_turn_left(QbDir dir) {
  switch (dir) {
    case QB_DIR_UP:
      return QB_DIR_LEFT;
    case QB_DIR_LEFT:
      return QB_DIR_DOWN;
    case QB_DIR_DOWN:
      return QB_DIR_RIGHT;
    case QB_DIR_RIGHT:
    default:
      return QB_DIR_UP;
  }
}

static QbDir prv_turn_right(QbDir dir) {
  switch (dir) {
    case QB_DIR_UP:
      return QB_DIR_RIGHT;
    case QB_DIR_RIGHT:
      return QB_DIR_DOWN;
    case QB_DIR_DOWN:
      return QB_DIR_LEFT;
    case QB_DIR_LEFT:
    default:
      return QB_DIR_UP;
  }
}

static int prv_highscore_key(void) {
  return s_game.difficulty==QB_STANDARD?KEY_HIGHSCORE:s_game.difficulty==QB_EASY?1004:1005;
}

static void prv_select_difficulty(QbDifficulty mode) {
  s_game.score=0;
  s_game.menu_ticks=0;
  s_game.difficulty=mode;
  const int key=prv_highscore_key();
  s_game.highscore=persist_exists(key)?persist_read_int(key):0;
  persist_write_int(KEY_DIFFICULTY,mode);
}

static void prv_flash_claim(void) {
#ifdef PBL_COLOR
  light_enable_interaction();
#endif
}

static void prv_flash_death(void) {
#ifdef PBL_COLOR
  light_enable_interaction();
#endif
}

static void prv_save_highscore(void) {
  if (s_game.score > s_game.highscore) {
    s_game.highscore = s_game.score;
    persist_write_int(prv_highscore_key(), s_game.highscore);
  }
}

static void prv_begin_level(void) {
  grid_init_level();
  const QbDifficultyRules rules=qb_difficulty_rules(s_game.difficulty);
  int stage=s_game.level+rules.stage_offset;
  if(stage<1) stage=1;
  if(stage>UINT8_MAX) stage=UINT8_MAX;
  enemies_init_seed(&s_game.enemies, (uint8_t)stage,
                   (uint32_t)time(NULL) ^ (++s_round_seed * 2654435761UL));
  s_game.enemies.reinforcement_ticks=rules.pressure_seconds*(1000/QB_TIMER_MS);
  if(!s_game.enemies.runners_super)
    s_game.enemies.super_ticks_remaining=s_game.enemies.reinforcement_ticks;
  s_game.player = (QbCell){QB_GRID_W / 2, QB_GRID_H - 1};
  s_game.dir = QB_DIR_UP;
  s_game.moving = false;
  s_game.edge_follow=false;
  s_game.arrival_dir=s_game.edge_choice=QB_DIR_NONE;
  s_game.pending_action=PENDING_NONE;
  s_game.drawing = false;
  s_game.trail_slow = false;
  s_game.trail_len = 0;
  s_game.fuse_index = 0;
  s_game.tick = 0;
  s_game.last_move_tick = 0;
  s_game.target_percent = rules.target;
  s_game.claim_cells = 0;
  s_game.screen = GAME_SCREEN_COUNTDOWN;
  s_game.screen_ticks = 3 * (1000 / QB_TIMER_MS);
}

static void prv_new_game(void) {
  memset(&s_game.trail, 0, sizeof(s_game.trail));
  s_game.level = 1;
  s_game.lives = qb_difficulty_rules(s_game.difficulty).lives;
  s_game.multiplier = 1;
  s_game.score = 0;
  s_game.slow_mode = false;
  prv_begin_level();
}

static void prv_die(void) {
  grid_clear_trails();
  s_game.moving = false;
  s_game.drawing = false;
  s_game.trail_len = 0;
  s_game.fuse_index = 0;
  s_game.edge_choice=QB_DIR_NONE;
  s_game.pending_action=PENDING_NONE;
  if (s_game.lives > 0) {
    s_game.lives--;
  }
  prv_flash_death();

  if (s_game.lives == 0) prv_save_highscore();
  s_game.screen = GAME_SCREEN_DEATH;
  s_game.screen_ticks = QB_DEATH_CARD_TICKS;
}

static void prv_level_clear(bool split_bonus) {
  if (split_bonus) {
    if (s_game.multiplier < 9) {
      s_game.multiplier++;
    }
  } else {
    s_game.score += (int32_t)(s_game.level * 250 * s_game.multiplier);
  }
  prv_save_highscore();
  s_game.moving = false;
  s_game.screen = GAME_SCREEN_LEVEL_CLEAR;
  s_game.screen_ticks = QB_LEVEL_CARD_TICKS;
}

static void prv_push_trail(QbCell cell) {
  if (s_game.trail_len < QB_TRAIL_MAX) {
    s_game.trail[s_game.trail_len++] = cell;
  } else {
    prv_die();
  }
}

static void prv_close_trail(void) {
  if (enemies_trail_collision(&s_game.enemies)) {
    prv_die();
    return;
  }
  QbCell enemy_cells[QB_MAX_RIFTS];
  const uint8_t enemy_count = enemies_get_claim_cells(&s_game.enemies, enemy_cells, QB_MAX_RIFTS);
  QbCell occupied[QB_MAX_RIFTS * QB_RIFT_LINES * 2 * QB_RIFT_BRUSH_PX * QB_RIFT_BRUSH_PX];
  uint8_t occupied_count = 0;
  for (uint8_t i = 0; i < enemy_count; i++) {
    for (uint8_t age = 0; age < QB_RIFT_LINES; age++) {
      for (uint8_t endpoint = 0; endpoint < 2; endpoint++) {
        const QbPixel p = enemies_get_rift_visual_point(&s_game.enemies.rifts[i], age, endpoint);
        for (int dy = 0; dy < QB_RIFT_BRUSH_PX; dy++)
          for (int dx = 0; dx < QB_RIFT_BRUSH_PX; dx++)
            occupied[occupied_count++] = (QbCell){(p.x + dx) / QB_CELL_PX, (p.y + dy) / QB_CELL_PX};
      }
    }
  }

  grid_commit_trails_to_border();
  const QbClaimResult claim = grid_claim_with_footprint(enemy_cells, enemy_count,
    occupied, occupied_count, s_game.trail_slow);

  const int speed_mult = s_game.trail_slow ? 2 : 1;
  s_game.score += (int32_t)(claim.new_claimed * speed_mult * s_game.multiplier);
  s_game.trail_len = 0;
  s_game.fuse_index = 0;
  s_game.drawing = false;
  s_game.edge_follow=true;
  s_game.edge_choice=QB_DIR_NONE;
  prv_ensure_frontier();
  prv_flash_claim();

  if (claim.split_enemies || claim.claimed_percent >= s_game.target_percent) {
    if (!claim.split_enemies && claim.claimed_percent > s_game.target_percent) {
      const int over = claim.claimed_percent - s_game.target_percent;
      s_game.score += (int32_t)(over * 100 * s_game.multiplier);
    }
    prv_level_clear(claim.split_enemies);
  } else {
    s_game.claim_cells = (uint16_t)claim.new_claimed;
    s_game.moving = false;
    s_game.screen = GAME_SCREEN_CLAIM_PAUSE;
    s_game.screen_ticks = 1000 / QB_TIMER_MS;
  }
}

static bool prv_can_step(QbDir dir) {
  if(dir==QB_DIR_NONE || (s_game.drawing && dir==prv_reverse(
     s_game.arrival_dir!=QB_DIR_NONE?s_game.arrival_dir:s_game.dir))) return false;
  const int nx = (int)s_game.player.x + prv_dx(dir);
  const int ny = (int)s_game.player.y + prv_dy(dir);
  if (!grid_in_bounds(nx, ny)) {
    return false;
  }
  const QbCellState state = grid_get(nx, ny);
  if (s_game.drawing) {
    return state == CELL_EMPTY || grid_is_safe(nx, ny);
  }
  return state == CELL_EMPTY || grid_is_player_path(nx, ny);
}

static void prv_move_player(void) {
  if (!s_game.moving || s_game.dir == QB_DIR_NONE) {
    return;
  }
  if (!s_game.drawing) prv_ensure_frontier();
  if(!s_game.drawing && s_game.edge_follow) {
    prv_continue_on_edge();
    if(!s_game.moving) return;
  }

  const int nx = (int)s_game.player.x + prv_dx(s_game.dir);
  const int ny = (int)s_game.player.y + prv_dy(s_game.dir);
  if (!grid_in_bounds(nx, ny)) {
    s_game.moving = false;
    return;
  }

  const QbCellState state = grid_get(nx, ny);
  const QbCell next = {(uint8_t)nx, (uint8_t)ny};

  if (!s_game.drawing) {
    if (grid_is_player_path(nx, ny)) {
      s_game.edge_follow=true;
      s_game.arrival_dir=s_game.dir;
      s_game.edge_choice=QB_DIR_NONE;
      s_game.player = next;
      s_game.last_move_tick = s_game.tick;
      prv_continue_on_edge();
    } else if (state == CELL_EMPTY) {
      s_game.drawing = true;
      s_game.trail_slow = s_game.slow_mode;
      s_game.trail_len = 0;
      s_game.fuse_index = 0;
      grid_set_trail(nx, ny, s_game.trail_slow);
      prv_push_trail(next);
      s_game.player = next;
      s_game.arrival_dir=s_game.dir;
      s_game.last_move_tick = s_game.tick;
    } else {
      s_game.moving = false;
    }
    return;
  }

  if (state == CELL_EMPTY) {
    grid_set_trail(nx, ny, s_game.trail_slow);
    prv_push_trail(next);
    s_game.player = next;
    s_game.arrival_dir=s_game.dir;
    s_game.last_move_tick = s_game.tick;
  } else if (grid_is_safe(nx, ny)) {
    s_game.player = next;
    s_game.arrival_dir=s_game.dir;
    s_game.last_move_tick = s_game.tick;
    if (s_game.trail_len > 2) {
      prv_close_trail();
    } else {
      grid_clear_trails();
      s_game.trail_len = 0;
      s_game.drawing = false;
      s_game.edge_follow=true;
      s_game.edge_choice=QB_DIR_NONE;
      prv_continue_on_edge();
    }
  } else if (grid_is_trail_state(state)) {
    prv_die();
  } else {
    s_game.moving = false;
  }
}

static void prv_update_fuse(void) {
  if (!s_game.drawing || s_game.trail_len == 0) {
    s_game.fuse_index = 0;
    return;
  }
  if ((uint16_t)(s_game.tick - s_game.last_move_tick) < qb_difficulty_rules(s_game.difficulty).fuse_ticks) {
    s_game.fuse_index = 0;
    return;
  }
  if (s_game.tick % 3 == 0 && s_game.fuse_index < s_game.trail_len) {
    s_game.fuse_index++;
  }
  if (s_game.fuse_index >= s_game.trail_len) {
    prv_die();
  }
}

void game_init(void) {
  memset(&s_game, 0, sizeof(s_game));
  int mode=persist_exists(KEY_DIFFICULTY)?persist_read_int(KEY_DIFFICULTY):QB_STANDARD;
  s_game.difficulty=mode>=0 && mode<QB_DIFFICULTY_COUNT?(QbDifficulty)mode:QB_STANDARD;
  const int key=prv_highscore_key();
  s_game.highscore = persist_exists(key) ? persist_read_int(key) : 0;
  s_game.screen = GAME_SCREEN_TITLE;
  s_game.dir = QB_DIR_RIGHT;
  s_game.music_enabled=s_game.effects_enabled=s_game.haptics_enabled=true;
  s_game.volume=50;
  if(persist_exists(KEY_SOUND)) {
    uint32_t sound=(uint32_t)persist_read_int(KEY_SOUND);
    if((sound>>8)<=100) {
      s_game.music_enabled=(sound&1)!=0;
      s_game.effects_enabled=(sound&2)!=0;
      s_game.haptics_enabled=(sound&4)!=0;
      s_game.volume=(uint8_t)(sound>>8);
    }
  }
}

void game_deinit(void) {
  prv_save_highscore();
}

void game_update(void) {
  s_game.tick++;

  if (s_game.screen == GAME_SCREEN_TITLE || s_game.screen == GAME_SCREEN_GAME_OVER ||
      s_game.screen == GAME_SCREEN_HELP || s_game.screen==GAME_SCREEN_SOUND || s_game.screen==GAME_SCREEN_CREDITS ||
      s_game.screen==GAME_SCREEN_DIFFICULTY) {
    if (s_game.menu_ticks < 120) s_game.menu_ticks++;
    return;
  }

  if (s_game.screen != GAME_SCREEN_PLAYING) {
    if (s_game.screen_ticks > 0) {
      s_game.screen_ticks--;
    }
    if (s_game.screen_ticks == 0) {
      if (s_game.screen == GAME_SCREEN_DEATH) {
        if (s_game.lives == 0) {
          s_game.screen = GAME_SCREEN_GAME_OVER;
          s_game.menu_ticks = 0;
        } else prv_begin_level();
      } else if (s_game.screen == GAME_SCREEN_LEVEL_CLEAR) {
        if (s_game.level < UINT8_MAX) s_game.level++;
        prv_begin_level();
      } else if (s_game.screen == GAME_SCREEN_LEVEL || s_game.screen == GAME_SCREEN_COUNTDOWN) {
        s_game.screen = GAME_SCREEN_PLAYING;
        s_game.dir = QB_DIR_UP;
        s_game.moving = true;
        s_game.last_move_tick = s_game.tick;
      } else if (s_game.screen == GAME_SCREEN_CLAIM_PAUSE) {
        s_game.screen = GAME_SCREEN_PLAYING;
        s_game.edge_follow=true;
        prv_continue_on_edge();
        prv_apply_pending();
        s_game.last_move_tick = s_game.tick;
      }
    }
    return;
  }

  const int move_mod = s_game.drawing && s_game.trail_slow ? QB_PLAYER_SLOW_TICKS : QB_PLAYER_FAST_TICKS;
  if (s_game.tick % move_mod == 0) {
    prv_move_player();
  }
  if (s_game.screen != GAME_SCREEN_PLAYING) {
    return;
  }

  bool trail_hit = false;
  enemies_update(&s_game.enemies, s_game.player, s_game.drawing, &trail_hit);
  if (trail_hit || enemies_player_collision(&s_game.enemies, s_game.player, s_game.drawing)) {
    prv_die();
    return;
  }

  prv_update_fuse();
}

void game_stop_go(void) {
  if(s_game.screen==GAME_SCREEN_HELP && s_game.help_page+1<QB_GUIDE_PAGES) { prv_help_page(1); return; }
  if(s_game.screen==GAME_SCREEN_DIFFICULTY) { s_game.touch_controls=false; prv_new_game(); return; }
  if(s_game.screen==GAME_SCREEN_CLAIM_PAUSE) {
    s_game.touch_controls=false;
    if(s_game.pending_action==PENDING_GO) s_game.pending_action=PENDING_STOP;
    else {
      if(s_game.pending_action<PENDING_LEFT_TURN || s_game.pending_action>PENDING_SLOW_RIGHT)
        s_game.pending_heading=s_game.dir;
      s_game.pending_action=PENDING_GO;
    }
    return;
  }
  if(s_game.screen==GAME_SCREEN_CREDITS) { s_game.screen=GAME_SCREEN_SOUND; return; }
  if(s_game.screen==GAME_SCREEN_SOUND) {
    switch(s_game.sound_row) {
      case 0: s_game.music_enabled=!s_game.music_enabled; break;
      case 1: s_game.effects_enabled=!s_game.effects_enabled; break;
      case 2: s_game.haptics_enabled=!s_game.haptics_enabled; break;
      case 3: s_game.volume=s_game.volume<25?25:s_game.volume<50?50:s_game.volume<75?75:s_game.volume<100?100:0; break;
      default: s_game.screen=GAME_SCREEN_CREDITS; return;
    }
    persist_write_int(KEY_SOUND,(s_game.music_enabled?1:0)|(s_game.effects_enabled?2:0)|
                     (s_game.haptics_enabled?4:0)|((uint32_t)s_game.volume<<8));
    return;
  }
  if(s_game.screen==GAME_SCREEN_TITLE || s_game.screen==GAME_SCREEN_HELP) {
    s_game.screen=GAME_SCREEN_DIFFICULTY;
    s_game.menu_ticks=0;
    return;
  }
  if (s_game.screen == GAME_SCREEN_GAME_OVER) {
    s_game.touch_controls=false;
    prv_new_game();
    return;
  }
  if (s_game.screen != GAME_SCREEN_PLAYING) {
    return;
  }
  s_game.touch_controls=false;
  if(!s_game.drawing) prv_ensure_frontier();
  if(s_game.moving) { s_game.moving=false; return; }
  prv_button_resume();
}

void game_tap(void) {
  if (s_game.screen == GAME_SCREEN_TITLE || s_game.screen == GAME_SCREEN_GAME_OVER) {
    game_stop_go();
    s_game.touch_controls=true;
    return;
  }
  if(s_game.screen==GAME_SCREEN_CLAIM_PAUSE) {
    s_game.touch_controls=true;
    s_game.pending_action=s_game.pending_action==PENDING_TOUCH_REVERSE?PENDING_NONE:PENDING_TOUCH_REVERSE;
    return;
  }
  if (s_game.screen == GAME_SCREEN_PLAYING) {
    s_game.touch_controls=true;
    if(s_game.drawing) {
      if(s_game.moving) s_game.moving=false;
      else if(prv_can_step(s_game.dir)) s_game.moving=true;
    } else {
      prv_ensure_frontier();
      QbDir back=prv_reverse(s_game.arrival_dir!=QB_DIR_NONE?s_game.arrival_dir:s_game.dir);
      if(prv_safe_exit(back)) {
        s_game.edge_choice=s_game.dir=back;
        s_game.edge_follow=s_game.moving=true;
      }
    }
  }
}

void game_touch_tap(int x, int y) {
  if(s_game.screen==GAME_SCREEN_DIFFICULTY) {
    if(y>=202) { game_back_to_title(); return; }
    if(y>=43 && y<196) {
      prv_select_difficulty((QbDifficulty)((y-43)/51));
      s_game.touch_controls=true;
      prv_new_game();
    }
    return;
  }
  if(s_game.screen==GAME_SCREEN_SOUND) {
    if(y>=200) { game_back_to_title(); return; }
    if(y>=44 && y<200) {
      s_game.sound_row=(uint8_t)(y<160?(y-44)/29:4);
      game_stop_go();
    }
    return;
  }
  if(s_game.screen==GAME_SCREEN_CREDITS) { s_game.screen=GAME_SCREEN_SOUND; return; }
  if(s_game.screen==GAME_SCREEN_GAME_OVER && y>=200) {
    game_back_to_title();
    return;
  }
  if (s_game.screen == GAME_SCREEN_TITLE && y >= 200) {
    if(x<100) prv_open_help();
    else s_game.screen=GAME_SCREEN_SOUND;
    return;
  }
  if (s_game.screen == GAME_SCREEN_HELP) {
    if (y >= 200) {
      if (x < 90) game_back_to_title();
      else game_stop_go();
    }
    return;
  }
  game_tap();
}

static void prv_button_heading(QbDir direction,bool slow) {
  const bool can_move=prv_can_step(direction);
  s_game.touch_controls=false;
  if(slow) s_game.slow_mode=true;
  s_game.dir=direction;
  s_game.edge_choice=QB_DIR_NONE;
  s_game.edge_follow=!s_game.drawing && prv_safe_exit(direction);
  s_game.moving=can_move;
}

static void prv_button_resume(void) {
  s_game.touch_controls=false;
  s_game.moving=false;
  s_game.edge_choice=QB_DIR_NONE;
  QbDir direction=s_game.dir;
  if(!prv_can_step(direction)) direction=prv_reverse(direction);
  if(prv_can_step(direction)) prv_button_heading(direction,false);
}

static void prv_button_turn(bool right,bool slow) {
  if(!s_game.drawing) prv_ensure_frontier();
  /* Buffered quarter turns compose, even through an impassable intermediate heading. */
  const bool queued=s_game.pending_action>=PENDING_LEFT_TURN &&
                    s_game.pending_action<=PENDING_SLOW_RIGHT;
  const QbDir base=s_game.screen==GAME_SCREEN_CLAIM_PAUSE && queued?
                   s_game.pending_heading:s_game.dir;
  const QbDir direction=right?prv_turn_right(base):prv_turn_left(base);
  if(s_game.screen==GAME_SCREEN_CLAIM_PAUSE) {
    s_game.touch_controls=false;
    s_game.pending_heading=direction;
    s_game.pending_action=slow?(right?PENDING_SLOW_RIGHT:PENDING_SLOW_LEFT):
                               (right?PENDING_RIGHT_TURN:PENDING_LEFT_TURN);
    return;
  }
  prv_button_heading(direction,slow);
}

void game_turn_left(bool slow) {
  if(s_game.screen==GAME_SCREEN_HELP) { prv_help_page(-1); return; }
  if(s_game.screen==GAME_SCREEN_DIFFICULTY) {
    prv_select_difficulty((s_game.difficulty+QB_DIFFICULTY_COUNT-1)%QB_DIFFICULTY_COUNT); return;
  }
  if(s_game.screen==GAME_SCREEN_SOUND) { s_game.sound_row=(s_game.sound_row+4)%5; return; }
  if (s_game.screen == GAME_SCREEN_TITLE || s_game.screen == GAME_SCREEN_GAME_OVER) {
    prv_open_help();
    return;
  }
  if (s_game.screen != GAME_SCREEN_PLAYING && s_game.screen!=GAME_SCREEN_CLAIM_PAUSE) {
    return;
  }
  prv_button_turn(false,slow);
}

void game_turn_right(bool slow) {
  if(s_game.screen==GAME_SCREEN_HELP) { prv_help_page(1); return; }
  if(s_game.screen==GAME_SCREEN_DIFFICULTY) {
    prv_select_difficulty((s_game.difficulty+1)%QB_DIFFICULTY_COUNT); return;
  }
  if(s_game.screen==GAME_SCREEN_SOUND) { s_game.sound_row=(s_game.sound_row+1)%5; return; }
  if (s_game.screen == GAME_SCREEN_TITLE || s_game.screen == GAME_SCREEN_GAME_OVER) {
    s_game.screen = GAME_SCREEN_SOUND;
    return;
  }
  if (s_game.screen != GAME_SCREEN_PLAYING && s_game.screen!=GAME_SCREEN_CLAIM_PAUSE) {
    return;
  }
  prv_button_turn(true,slow);
}

void game_set_direction(QbDir direction) {
  if(direction<QB_DIR_UP || direction>QB_DIR_LEFT) return;
  if(s_game.screen==GAME_SCREEN_HELP) {
    prv_help_page(direction==QB_DIR_LEFT || direction==QB_DIR_UP?1:-1);
    return;
  }
  if(s_game.screen==GAME_SCREEN_CLAIM_PAUSE) {
    if(direction!=prv_reverse(s_game.arrival_dir) && prv_can_step(direction)) {
      s_game.touch_controls=true;
      s_game.pending_action=(QbPendingAction)(PENDING_UP+direction-QB_DIR_UP);
    }
    return;
  }
  if(s_game.screen!=GAME_SCREEN_PLAYING) {
    if(direction==QB_DIR_UP) game_turn_left(false);
    else if(direction==QB_DIR_DOWN) game_turn_right(false);
    return;
  }
  if(!s_game.drawing) prv_ensure_frontier();
  if(direction==prv_reverse(s_game.arrival_dir)) return;
  /* Invalid gestures never replace a valid heading or backtrack a live trail. */
  if(!prv_can_step(direction)) return;
  s_game.touch_controls=true;
  s_game.dir=direction;
  s_game.moving=true;
  s_game.edge_follow=!s_game.drawing && grid_is_player_path(
    s_game.player.x+prv_dx(direction),s_game.player.y+prv_dy(direction));
  s_game.edge_choice=s_game.edge_follow?direction:QB_DIR_NONE;
}

void game_toggle_slow(void) {
  if(s_game.screen==GAME_SCREEN_HELP) { game_stop_go(); return; }
  if(s_game.screen==GAME_SCREEN_SOUND || s_game.screen==GAME_SCREEN_CREDITS) { game_stop_go(); return; }
  if (s_game.screen == GAME_SCREEN_TITLE || s_game.screen == GAME_SCREEN_GAME_OVER) {
    game_stop_go();
    return;
  }
  s_game.slow_mode = !s_game.slow_mode;
}

void game_back_to_title(void) {
  if(s_game.screen==GAME_SCREEN_CREDITS) { s_game.screen=GAME_SCREEN_SOUND; return; }
  if (s_game.screen == GAME_SCREEN_TITLE) {
    return;
  }
  grid_clear_trails();
  s_game.moving = false;
  s_game.drawing = false;
  s_game.trail_len = 0;
  s_game.fuse_index = 0;
  prv_save_highscore();
  s_game.score=0;
  s_game.screen = GAME_SCREEN_TITLE;
  s_game.menu_ticks = 0;
}

bool game_backlight_required(void) {
  return s_game.screen != GAME_SCREEN_TITLE && s_game.screen != GAME_SCREEN_HELP &&
         s_game.screen!=GAME_SCREEN_SOUND && s_game.screen!=GAME_SCREEN_CREDITS &&
         s_game.screen!=GAME_SCREEN_DIFFICULTY;
}

QbGameView game_view(void) {
  QbDir exits[4];
  const unsigned count=prv_forward_exits(false,exits);
  const bool on_edge=s_game.touch_controls && s_game.screen==GAME_SCREEN_PLAYING &&
                     !s_game.drawing && s_game.edge_follow;
  QbGameView view = {
    .screen = s_game.screen,
    .level = s_game.level,
    .lives = s_game.lives,
    .target_percent = s_game.target_percent,
    .claimed_percent = (uint8_t)grid_claimed_percent(),
    .multiplier = s_game.multiplier,
    .slow_mode = s_game.slow_mode,
    .moving = s_game.moving,
    .drawing = s_game.drawing,
    .cut_slow = s_game.drawing && s_game.trail_slow,
    .runners_super = s_game.enemies.runners_super,
    .super_seconds = (s_game.enemies.super_ticks_remaining + (1000 / QB_TIMER_MS) - 1) / (1000 / QB_TIMER_MS),
    .countdown = s_game.screen == GAME_SCREEN_COUNTDOWN ?
                 (uint8_t)((s_game.screen_ticks + (1000 / QB_TIMER_MS) - 1) / (1000 / QB_TIMER_MS)) : 0,
    .claim_cells = s_game.claim_cells,
    .menu_ticks = s_game.menu_ticks,
    .transition_ticks = s_game.screen == GAME_SCREEN_DEATH ? QB_DEATH_CARD_TICKS - s_game.screen_ticks :
      s_game.screen == GAME_SCREEN_LEVEL_CLEAR ? QB_LEVEL_CARD_TICKS - s_game.screen_ticks :
      s_game.screen == GAME_SCREEN_COUNTDOWN ?
        (3 * (1000 / QB_TIMER_MS) - s_game.screen_ticks) % (1000 / QB_TIMER_MS) : 0,
    .score = s_game.score,
    .music_enabled=s_game.music_enabled,.effects_enabled=s_game.effects_enabled,
    .haptics_enabled=s_game.haptics_enabled,.volume=s_game.volume,.sound_row=s_game.sound_row,
    .help_page=s_game.help_page,
    .highscore = s_game.highscore,
    .difficulty=s_game.difficulty,.direction=s_game.dir,
    .choosing_path=on_edge && !s_game.moving && count>1,
    .edge_dead_end=on_edge && !s_game.moving && count==0,
    .upper_path=count?exits[0]:QB_DIR_NONE,.lower_path=count?exits[count-1]:QB_DIR_NONE
  };
  return view;
}

QbCell game_player_cell(void) {
  return s_game.player;
}

bool game_fuse_cell(QbCell *out_cell) {
  if (!s_game.drawing || s_game.fuse_index == 0 || s_game.trail_len == 0) {
    return false;
  }
  uint16_t index = s_game.fuse_index;
  if (index >= s_game.trail_len) {
    index = s_game.trail_len - 1;
  }
  *out_cell = s_game.trail[index];
  return true;
}

const QbEnemies *game_enemies(void) {
  return &s_game.enemies;
}

uint16_t game_anim(void) {
  return s_game.tick;
}
