#include "ui.h"

#include <stdio.h>
#include "game.h"
#include "grid.h"
#include "pixel_art.h"

static void prv_rect(GContext *ctx, GRect rect, GColor color) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void prv_text(GContext *ctx, const char *text, GRect box, const char *font, GColor color,
                     GTextAlignment alignment) {
  graphics_context_set_text_color(ctx, color);
  graphics_draw_text(ctx, text, fonts_get_system_font(font), box,
                     GTextOverflowModeTrailingEllipsis, alignment, NULL);
}

static void prv_center(GContext *ctx, const char *text, int y, const char *font, GColor color) {
  prv_text(ctx, text, GRect(4,y,192,36), font, color, GTextAlignmentCenter);
}

static void prv_score(char *out, size_t size, int32_t score) {
  if (score >= 1000000) snprintf(out, size, "%ld.%ldM", (long)(score/1000000), (long)(score/100000%10));
  else snprintf(out, size, "%ld", (long)score);
}

static GColor prv_cell_color(int x,int y) {
  const QbCellState state=grid_get(x,y);
  switch (state) {
    case CELL_CLAIM_FAST: return GColorVividCerulean;
    case CELL_CLAIM_SLOW: return GColorOrange;
    case CELL_BORDER: return grid_is_player_path(x,y)?GColorWhite:GColorCobaltBlue;
    case CELL_TRAIL_FAST: return GColorCyan;
    case CELL_TRAIL_SLOW: return GColorYellow;
    default: return GColorBlack;
  }
}

static GPoint prv_cell_center(QbCell p) {
  return GPoint(QB_BOARD_X + qb_cell_center_px(p.x), QB_BOARD_Y + qb_cell_center_px(p.y));
}

static void prv_diamond(GContext *ctx, GPoint p, int radius, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx,GPoint(p.x,p.y-radius),GPoint(p.x+radius,p.y));
  graphics_draw_line(ctx,GPoint(p.x+radius,p.y),GPoint(p.x,p.y+radius));
  graphics_draw_line(ctx,GPoint(p.x,p.y+radius),GPoint(p.x-radius,p.y));
  graphics_draw_line(ctx,GPoint(p.x-radius,p.y),GPoint(p.x,p.y-radius));
}

static void prv_heart(GContext *ctx, int x, int y, GColor color) {
  static const uint8_t rows[6] = {54,127,127,62,28,8};
  for (int row=0; row<6; row++) for (int col=0; col<7; col++)
    if (rows[row] & (64U >> col)) prv_rect(ctx,GRect(x+col,y+row,1,1),color);
}

static void prv_draw_grid(GContext *ctx) {
  for (int y=0; y<QB_GRID_H; y++) {
    int start=0;
    GColor previous=prv_cell_color(0,y);
    for (int x=1; x<=QB_GRID_W; x++) {
      GColor color=x<QB_GRID_W?prv_cell_color(x,y):GColorBlack;
      if (x<QB_GRID_W && color.argb==previous.argb) continue;
      prv_rect(ctx,GRect(QB_BOARD_X+start*QB_CELL_PX,QB_BOARD_Y+y*QB_CELL_PX,
                        (x-start)*QB_CELL_PX,QB_CELL_PX),previous);
      start=x;
      previous=color;
    }
  }
}

static void prv_hud(GContext *ctx, QbGameView v) {
  prv_rect(ctx,GRect(0,0,QB_SCREEN_W,QB_HUD_H),GColorBlack);
  char amount[12], goal[12], score[18], status[16], time[8];
  snprintf(amount,sizeof(amount),"%u%%",v.claimed_percent);
  snprintf(goal,sizeof(goal),"/%u%%",v.target_percent);
  prv_score(score,sizeof(score),v.score);
  prv_text(ctx,amount,GRect(5,-4,53,29),FONT_KEY_GOTHIC_24_BOLD,GColorWhite,GTextAlignmentLeft);
  GSize amount_size=graphics_text_layout_get_content_size(amount,
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),GRect(0,0,53,29),
    GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft);
  prv_text(ctx,goal,GRect(8+amount_size.w,3,43,22),FONT_KEY_GOTHIC_14_BOLD,GColorCyan,GTextAlignmentLeft);
  prv_text(ctx,"PTS",GRect(104,4,25,18),FONT_KEY_GOTHIC_14,GColorLightGray,GTextAlignmentLeft);
  prv_text(ctx,score,GRect(129,-1,66,24),FONT_KEY_GOTHIC_18_BOLD,GColorYellow,GTextAlignmentRight);

  for (int i=0;i<qb_difficulty_rules(v.difficulty).lives;i++)
    prv_heart(ctx,6+i*11,26,i<v.lives?GColorRed:GColorDarkGray);
  if(v.multiplier>1) snprintf(status,sizeof(status),"L%u x%u",v.level,v.multiplier);
  else snprintf(status,sizeof(status),"LEVEL %u",v.level);
  GColor status_color=GColorWhite;
  if(v.screen==GAME_SCREEN_PLAYING) {
    if(v.edge_dead_end) {
      snprintf(status,sizeof(status),"TAP BACK");
      status_color=GColorMagenta;
    } else if(!v.moving) {
      snprintf(status,sizeof(status),"STOPPED");
      status_color=GColorYellow;
    } else if(v.drawing) {
      snprintf(status,sizeof(status),v.cut_slow?"SLOW CUT":"CUT");
      status_color=v.cut_slow?GColorOrange:GColorYellow;
    } else if(v.slow_mode) {
      snprintf(status,sizeof(status),"SLOW NEXT");
      status_color=GColorOrange;
    }
  }
  prv_text(ctx,status,GRect(52,19,85,18),FONT_KEY_GOTHIC_14_BOLD,status_color,GTextAlignmentCenter);

  GColor clock_color=v.runners_super || v.super_seconds<=5?GColorRed:GColorYellow;
  if(v.runners_super) snprintf(time,sizeof(time),"!");
  else snprintf(time,sizeof(time),"%ds",v.super_seconds);
  GSize time_size=graphics_text_layout_get_content_size(time,
    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),GRect(0,0,34,18),
    GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft);
  const int time_left=195-time_size.w;
  const int clock_x=time_left-7;
  graphics_context_set_stroke_color(ctx,clock_color);
  graphics_context_set_stroke_width(ctx,1);
  graphics_draw_circle(ctx,GPoint(clock_x,28),3);
  graphics_draw_line(ctx,GPoint(clock_x,28),GPoint(clock_x,25));
  graphics_draw_line(ctx,GPoint(clock_x,28),GPoint(clock_x+2,28));
  prv_text(ctx,time,GRect(time_left,19,time_size.w,18),FONT_KEY_GOTHIC_14_BOLD,
           v.runners_super?GColorRed:GColorWhite,GTextAlignmentLeft);

  prv_rect(ctx,GRect(5,36,190,2),GColorDarkGray);
  int fill=190*v.claimed_percent/100;
  if(fill>0) prv_rect(ctx,GRect(5,36,fill,2),GColorCyan);
  prv_rect(ctx,GRect(5+190*v.target_percent/100,35,2,3),GColorWhite);
}

static void prv_rift_pixel(int x,int y,void *context) {
  graphics_draw_pixel(context,GPoint(QB_BOARD_X+x,QB_BOARD_Y+y));
}

typedef struct { GContext *ctx; bool super; } SparkDraw;

static void prv_spark_pixel(int x,int y,uint8_t ink,void *context) {
  SparkDraw *s=context;
  GColor color=s->super?(ink?GColorBlue:GColorCyan):(ink?GColorRed:GColorYellow);
  graphics_context_set_stroke_color(s->ctx,color);
  graphics_draw_pixel(s->ctx,GPoint(QB_BOARD_X+x,QB_BOARD_Y+y));
}

static void prv_enemies(GContext *ctx) {
  const QbEnemies *e=game_enemies();
  const GColor colors[4]={GColorCyan,GColorYellow,GColorGreen,GColorMagenta};
  graphics_context_set_stroke_width(ctx,1);
  for (int i=0;i<e->rift_count;i++) for(int age=QB_RIFT_LINES-1;age>=0;age--) {
    graphics_context_set_stroke_color(ctx,colors[(age+e->anim/8+i)%4]);
    enemies_visit_rift_line(&e->rifts[i],age,prv_rift_pixel,ctx);
  }
  for(int i=0;i<e->runner_count;i++) {
    SparkDraw spark={ctx,e->runners_super};
    enemies_visit_runner_colored_pixels(e,i,prv_spark_pixel,&spark);
  }
}

static void prv_player(GContext *ctx) {
  GPoint p=prv_cell_center(game_player_cell());
  prv_diamond(ctx,p,QB_PLAYER_RADIUS,GColorWhite);
  prv_diamond(ctx,p,QB_PLAYER_RADIUS-1,GColorCyan);
  prv_rect(ctx,GRect(p.x-1,p.y-1,3,3),GColorWhite);
  QbCell fuse;
  if(game_fuse_cell(&fuse)) {
    GPoint f=prv_cell_center(fuse);
    prv_diamond(ctx,f,4,GColorRed);
    prv_rect(ctx,GRect(f.x-1,f.y-1,3,3),GColorYellow);
  }
}

static void prv_logo_pixel(int x,int y,bool settled,void *context) {
  GContext *ctx=context;
  prv_rect(ctx,GRect(x+2,y+3,4,4),settled?GColorMagenta:GColorDarkGray);
  prv_rect(ctx,GRect(x,y,4,4),settled?(y<80?GColorWhite:GColorCyan):GColorYellow);
}

static void prv_title(GContext *ctx,QbGameView view) {
  pixel_logo_visit(view.menu_ticks,prv_logo_pixel,ctx);
  if(view.menu_ticks>=56) prv_center(ctx,"P O C K E T   A R C A D E",29,FONT_KEY_GOTHIC_14_BOLD,GColorLightGray);
  char best[24];
  snprintf(best,sizeof(best),"%s BEST",qb_difficulty_name(view.difficulty));
  prv_center(ctx,best,119,FONT_KEY_GOTHIC_14_BOLD,GColorLightGray);
  char score[18];
  prv_score(score,sizeof(score),view.highscore);
  prv_center(ctx,score,134,FONT_KEY_BITHAM_30_BLACK,GColorYellow);
  prv_center(ctx,"SELECT  START",177,FONT_KEY_GOTHIC_18_BOLD,GColorWhite);
  prv_center(ctx,"UP GUIDE / DOWN SOUND",205,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
}

static void prv_help_goal(GContext *ctx,QbGameView view) {
  const int left=24,top=49,width=152,height=76;
  prv_rect(ctx,GRect(left,top,width,height),GColorWhite);
  prv_rect(ctx,GRect(left+3,top+3,width-6,height-6),GColorBlack);
  prv_rect(ctx,GRect(left+3,top+3,34,height-6),GColorVividCerulean);
  int length=view.menu_ticks<50?70*view.menu_ticks/50:70;
  if(view.menu_ticks>=50) prv_rect(ctx,GRect(61,52,35,70),GColorVividCerulean);
  prv_rect(ctx,GRect(59,52,2,70),GColorCobaltBlue);
  prv_rect(ctx,GRect(95,122-length,2,length),view.menu_ticks<50?GColorYellow:GColorWhite);
  const GColor colors[]={GColorCyan,GColorYellow,GColorMagenta,GColorGreen};
  graphics_context_set_stroke_width(ctx,2);
  for(int i=0;i<7;i++) {
    graphics_context_set_stroke_color(ctx,colors[i%4]);
    graphics_draw_line(ctx,GPoint(119+i*2,67+i*3),GPoint(143+i*2,85+i));
  }
  graphics_context_set_stroke_width(ctx,1);
  prv_diamond(ctx,GPoint(96,122-length),4,GColorWhite);
  prv_center(ctx,"Close a line. Claim the area.",137,FONT_KEY_GOTHIC_14_BOLD,GColorWhite);
  char goal[32];
  snprintf(goal,sizeof(goal),"Reach %u%% to clear.",qb_difficulty_rules(view.difficulty).target);
  prv_center(ctx,goal,157,FONT_KEY_GOTHIC_18_BOLD,GColorYellow);
  prv_center(ctx,"Guard your line. Avoid sparks.",184,FONT_KEY_GOTHIC_14,GColorLightGray);
}

static void prv_help_touch(GContext *ctx) {
  prv_text(ctx,"RECOMMENDED",GRect(14,47,172,20),FONT_KEY_GOTHIC_14_BOLD,GColorYellow,GTextAlignmentLeft);
  prv_text(ctx,"SWIPE",GRect(14,75,172,20),FONT_KEY_GOTHIC_14_BOLD,GColorCyan,GTextAlignmentLeft);
  prv_text(ctx,"Steer in screen direction.",GRect(14,93,172,22),FONT_KEY_GOTHIC_14,GColorWhite,GTextAlignmentLeft);
  prv_rect(ctx,GRect(14,123,172,1),GColorDarkGray);
  prv_text(ctx,"TAP",GRect(14,133,172,20),FONT_KEY_GOTHIC_14_BOLD,GColorCyan,GTextAlignmentLeft);
  prv_text(ctx,"On an edge: turn back.",GRect(14,151,172,20),FONT_KEY_GOTHIC_14,GColorWhite,GTextAlignmentLeft);
  prv_text(ctx,"While drawing: Stop / Go.",GRect(14,169,172,20),FONT_KEY_GOTHIC_14,GColorWhite,GTextAlignmentLeft);
}

static void prv_help_buttons(GContext *ctx) {
  const char *keys[]={"UP","DOWN","SELECT"};
  const char *actions[]={"Left 90 deg","Right 90 deg","Stop / Go"};
  const GColor colors[]={GColorCyan,GColorYellow,GColorWhite};
  for(unsigned i=0;i<3;i++) {
    const int y=49+i*29;
    prv_text(ctx,keys[i],GRect(14,y,60,23),FONT_KEY_GOTHIC_14_BOLD,colors[i],GTextAlignmentLeft);
    prv_text(ctx,actions[i],GRect(82,y,108,23),FONT_KEY_GOTHIC_14,GColorWhite,GTextAlignmentLeft);
  }
  prv_text(ctx,"Blocked ahead? Go back.",GRect(14,136,176,20),FONT_KEY_GOTHIC_14,GColorLightGray,GTextAlignmentLeft);
  prv_rect(ctx,GRect(14,163,172,1),GColorDarkGray);
  prv_text(ctx,"HOLD SELECT",GRect(14,173,88,20),FONT_KEY_GOTHIC_14_BOLD,GColorOrange,GTextAlignmentLeft);
  prv_text(ctx,"Slow / Fast",GRect(108,173,82,20),FONT_KEY_GOTHIC_14,GColorWhite,GTextAlignmentLeft);
  prv_text(ctx,"BACK",GRect(14,191,60,18),FONT_KEY_GOTHIC_14_BOLD,GColorCyan,GTextAlignmentLeft);
  prv_text(ctx,"Menu / hold to exit",GRect(64,191,126,18),FONT_KEY_GOTHIC_14,GColorLightGray,GTextAlignmentLeft);
}

static void prv_help(GContext *ctx,QbGameView view) {
  const char *titles[]={"CLAIM THE FIELD","TOUCH","BUTTONS"};
  prv_center(ctx,titles[view.help_page],7,FONT_KEY_GOTHIC_24_BOLD,GColorCyan);
  for(int i=0;i<QB_GUIDE_PAGES;i++) prv_rect(ctx,GRect(14+i*59,35,54,2),
                                          i==view.help_page?GColorCyan:GColorDarkGray);
  if(view.help_page==0) prv_help_goal(ctx,view);
  else if(view.help_page==1) prv_help_touch(ctx);
  else prv_help_buttons(ctx);
  prv_text(ctx,"MENU",GRect(14,210,55,18),FONT_KEY_GOTHIC_14_BOLD,GColorCyan,GTextAlignmentLeft);
  prv_text(ctx,view.help_page+1==QB_GUIDE_PAGES?"SELECT PLAY":"SELECT NEXT",GRect(91,210,95,18),
           FONT_KEY_GOTHIC_14_BOLD,GColorWhite,GTextAlignmentRight);
}

static void prv_sound(GContext *ctx,QbGameView view) {
  prv_center(ctx,"SOUND",8,FONT_KEY_GOTHIC_24_BOLD,GColorCyan);
  const char *labels[5]={"Music","Effects","Haptics","Volume","Audio credits"};
  const bool values[3]={view.music_enabled,view.effects_enabled,view.haptics_enabled};
  for(int i=0;i<5;i++) {
    int y=i==4?174:44+i*29;
    if(view.sound_row==i) {
      graphics_context_set_stroke_color(ctx,GColorYellow);
      graphics_draw_line(ctx,GPoint(6,y+11),GPoint(10,y+15));
      graphics_draw_line(ctx,GPoint(10,y+15),GPoint(6,y+19));
    }
    prv_text(ctx,labels[i],GRect(17,y,121,26),FONT_KEY_GOTHIC_18_BOLD,
             view.sound_row==i?GColorWhite:GColorLightGray,GTextAlignmentLeft);
    if(i<3) {
      prv_rect(ctx,GRect(152,y+9,32,12),values[i]?GColorCyan:GColorDarkGray);
      prv_rect(ctx,GRect(values[i]?174:154,y+11,8,8),GColorWhite);
    } else if(i==3) {
      char volume[8];
      snprintf(volume,sizeof(volume),"%u%%",view.volume);
      prv_text(ctx,volume,GRect(138,y,47,24),FONT_KEY_GOTHIC_18_BOLD,GColorYellow,GTextAlignmentRight);
      prv_rect(ctx,GRect(18,y+25,166,2),GColorDarkGray);
      if(view.volume) prv_rect(ctx,GRect(18,y+25,166*view.volume/100,2),GColorYellow);
    } else prv_text(ctx,">",GRect(169,y,16,24),FONT_KEY_GOTHIC_18_BOLD,GColorCyan,GTextAlignmentRight);
  }
  prv_center(ctx,"BACK  MENU",207,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
}

static void prv_credits(GContext *ctx) {
  prv_center(ctx,"AUDIO CREDITS",9,FONT_KEY_GOTHIC_24_BOLD,GColorCyan);
  prv_center(ctx,"MUSIC",51,FONT_KEY_GOTHIC_14_BOLD,GColorLightGray);
  prv_center(ctx,"Juhani Junkala",70,FONT_KEY_GOTHIC_18_BOLD,GColorYellow);
  prv_center(ctx,"Retro Game Music Pack",91,FONT_KEY_GOTHIC_14_BOLD,GColorWhite);
  prv_center(ctx,"SOUND EFFECTS",108,FONT_KEY_GOTHIC_14_BOLD,GColorLightGray);
  prv_center(ctx,"Kenney - Digital SFX",127,FONT_KEY_GOTHIC_18_BOLD,GColorYellow);
  prv_center(ctx,"CC0 1.0",168,FONT_KEY_GOTHIC_14_BOLD,GColorWhite);
  prv_center(ctx,"BACK  SOUND",207,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
}

typedef struct { GContext *ctx; int scale; GColor color; } TileDraw;
typedef struct { TileDraw tile; int top; } ModeTileDraw;

static void prv_text_tile(int x,int y,bool settled,void *context) {
  TileDraw *tile=context;
  prv_rect(tile->ctx,GRect(x+1,y+2,tile->scale,tile->scale),GColorDarkGray);
  prv_rect(tile->ctx,GRect(x,y,tile->scale,tile->scale),settled?tile->color:GColorWhite);
}

static void prv_pixel_text(GContext *ctx,const char *text,int top,int scale,uint16_t tick,GColor color) {
  TileDraw tile={ctx,scale,color};
  pixel_text_visit(text,QB_SCREEN_W/2,top,scale,tick,prv_text_tile,&tile);
}

static void prv_mode_tile(int x,int y,bool settled,void *context) {
  ModeTileDraw *draw=context;
  if(x<20 || x+4>190 || y<draw->top-3 || y+5>draw->top+24) return;
  prv_text_tile(x,y,settled,&draw->tile);
}

static void prv_game_over(GContext *ctx,QbGameView view) {
  char score[18],best[32],level[16];
  prv_score(score,sizeof(score),view.score);
  snprintf(best,sizeof(best),"BEST %ld",(long)view.highscore);
  snprintf(level,sizeof(level),"LEVEL %u",view.level);
  prv_center(ctx,level,20,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
  prv_pixel_text(ctx,"GAME OVER",56,3,view.menu_ticks,GColorMagenta);
  prv_center(ctx,"SCORE",94,FONT_KEY_GOTHIC_14_BOLD,GColorLightGray);
  prv_center(ctx,score,109,FONT_KEY_BITHAM_30_BLACK,GColorYellow);
  prv_center(ctx,best,149,FONT_KEY_GOTHIC_14_BOLD,GColorWhite);
  prv_center(ctx,"SELECT  RETRY",177,FONT_KEY_GOTHIC_18_BOLD,GColorWhite);
  prv_center(ctx,"BACK  MENU",205,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
}

static void prv_transition(GContext *ctx,QbGameView view,bool clear) {
  prv_rect(ctx,GRect(QB_BOARD_X,QB_BOARD_Y,QB_BOARD_W,QB_BOARD_H),GColorWhite);
  prv_rect(ctx,GRect(QB_BOARD_X+QB_CELL_PX,QB_BOARD_Y+QB_CELL_PX,
                    QB_BOARD_W-2*QB_CELL_PX,QB_BOARD_H-2*QB_CELL_PX),GColorBlack);
  prv_pixel_text(ctx,clear?"CLEAR":"HIT",102,clear?4:5,view.transition_ticks,
                 clear?GColorGreen:GColorRed);
  char detail[32];
  if(clear) snprintf(detail,sizeof(detail),"LEVEL %u COMPLETE",view.level);
  else if(view.lives) snprintf(detail,sizeof(detail),"%u %s LEFT",view.lives,view.lives==1?"LIFE":"LIVES");
  else snprintf(detail,sizeof(detail),"FINAL LIFE LOST");
  prv_center(ctx,detail,148,FONT_KEY_GOTHIC_14_BOLD,GColorWhite);
  if(clear) {
    snprintf(detail,sizeof(detail),"%u%% CLAIMED",view.claimed_percent);
    prv_center(ctx,detail,170,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
  } else {
    const int lives=qb_difficulty_rules(view.difficulty).lives;
    for(int i=0;i<lives;i++) prv_heart(ctx,100-lives*6+i*12,177,i<view.lives?GColorRed:GColorDarkGray);
  }
}

static void prv_difficulty(GContext *ctx,QbGameView view) {
  prv_center(ctx,"DIFFICULTY",8,FONT_KEY_GOTHIC_24_BOLD,GColorCyan);
  const GColor colors[3]={GColorCyan,GColorYellow,GColorMagenta};
  for(int i=0;i<QB_DIFFICULTY_COUNT;i++) {
    int top=46+i*51;
    QbDifficultyRules rules=qb_difficulty_rules((QbDifficulty)i);
    bool selected=view.difficulty==(QbDifficulty)i;
    if(selected) {
      graphics_context_set_stroke_color(ctx,colors[i]);
      graphics_draw_line(ctx,GPoint(7,top+12),GPoint(12,top+17));
      graphics_draw_line(ctx,GPoint(12,top+17),GPoint(7,top+22));
    }
    ModeTileDraw tiles={{ctx,3,selected?colors[i]:GColorLightGray},top};
    pixel_text_visit(qb_difficulty_name((QbDifficulty)i),100,top,3,
                     selected?view.menu_ticks:120,prv_mode_tile,&tiles);
    char detail[40];
    snprintf(detail,sizeof(detail),"%u LIVES  /  %u%%  /  %us",rules.lives,rules.target,rules.pressure_seconds);
    prv_center(ctx,detail,top+24,FONT_KEY_GOTHIC_14_BOLD,
               selected?GColorWhite:GColorLightGray);
    if(selected) {
      int span=24+140*(view.menu_ticks<18?view.menu_ticks:18)/18;
      prv_rect(ctx,GRect(100-span/2,top+45,span,2),colors[i]);
    } else if(i<2) prv_rect(ctx,GRect(18,top+45,164,1),GColorDarkGray);
  }
  prv_center(ctx,"SELECT START / BACK MENU",207,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
}

static void prv_countdown(GContext *ctx,QbGameView view) {
  char digit[2]={(char)('0'+view.countdown),0};
  prv_rect(ctx,GRect(38,76,124,121),GColorBlack);
  prv_center(ctx,"READY",80,FONT_KEY_GOTHIC_14_BOLD,GColorCyan);
  prv_pixel_text(ctx,digit,105,8,view.transition_ticks,GColorYellow);
  for(int i=0;i<3;i++) prv_diamond(ctx,GPoint(80+i*20,181),3,
                                  i<view.countdown?GColorCyan:GColorDarkGray);
}

static void prv_update_proc(Layer *layer,GContext *ctx) {
  (void)layer;
  QbGameView v=game_view();
  prv_rect(ctx,GRect(0,0,QB_SCREEN_W,QB_SCREEN_H),GColorBlack);
  if(v.screen==GAME_SCREEN_TITLE) { prv_title(ctx,v); return; }
  if(v.screen==GAME_SCREEN_GAME_OVER) { prv_game_over(ctx,v); return; }
  if(v.screen==GAME_SCREEN_HELP) { prv_help(ctx,v); return; }
  if(v.screen==GAME_SCREEN_SOUND) { prv_sound(ctx,v); return; }
  if(v.screen==GAME_SCREEN_CREDITS) { prv_credits(ctx); return; }
  if(v.screen==GAME_SCREEN_DIFFICULTY) { prv_difficulty(ctx,v); return; }
  if(v.screen==GAME_SCREEN_DEATH || v.screen==GAME_SCREEN_LEVEL_CLEAR) {
    prv_transition(ctx,v,v.screen==GAME_SCREEN_LEVEL_CLEAR);
    prv_hud(ctx,v);
    return;
  }
  prv_draw_grid(ctx);
  prv_enemies(ctx);
  prv_player(ctx);
  if(v.screen==GAME_SCREEN_COUNTDOWN) prv_countdown(ctx,v);
  else if(v.screen==GAME_SCREEN_CLAIM_PAUSE) {
    GPoint p=prv_cell_center(game_player_cell());
    int radius=7+(game_anim()%9);
    prv_diamond(ctx,p,radius,GColorYellow);
  }
  prv_hud(ctx,v);
}

Layer *ui_layer_create(GRect bounds) {
  Layer *layer=layer_create(bounds);
  layer_set_update_proc(layer,prv_update_proc);
  return layer;
}

void ui_layer_destroy(Layer *layer) { layer_destroy(layer); }
void ui_mark_dirty(Layer *layer) { layer_mark_dirty(layer); }
