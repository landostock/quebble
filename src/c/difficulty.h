#ifndef QUEBBLE_DIFFICULTY_H
#define QUEBBLE_DIFFICULTY_H

#include <stdint.h>

typedef enum { QB_EASY, QB_STANDARD, QB_EXPERT, QB_DIFFICULTY_COUNT } QbDifficulty;
typedef struct {
  uint8_t lives, target, pressure_seconds, fuse_ticks;
  int8_t stage_offset;
} QbDifficultyRules;

static inline QbDifficultyRules qb_difficulty_rules(QbDifficulty mode) {
  switch(mode) {
    case QB_EASY: return (QbDifficultyRules){4,65,45,33,-2};
    case QB_EXPERT: return (QbDifficultyRules){2,80,22,15,2};
    default: return (QbDifficultyRules){3,75,30,21,0};
  }
}

static inline const char *qb_difficulty_name(QbDifficulty mode) {
  return mode==QB_EASY?"EASY":mode==QB_EXPERT?"EXPERT":"STANDARD";
}

#endif
