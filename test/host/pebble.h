#ifndef QUEBBLE_HOST_PEBBLE_H
#define QUEBBLE_HOST_PEBBLE_H
#ifdef QUEBBLE_FEEDBACK_TEST
#include "feedback_pebble.h"
#elif defined(QUEBBLE_INPUT_TEST)
#include "input_pebble.h"
#else
#include <stdbool.h>
#include <stdint.h>
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))
typedef struct { const uint32_t *durations; unsigned num_segments; } VibePattern;
static inline void vibes_short_pulse(void) {}
static inline void vibes_double_pulse(void) {}
static inline void vibes_enqueue_custom_pattern(VibePattern pattern) { (void)pattern; }
#ifdef QUEBBLE_PERSIST_TEST
bool persist_exists(int key);
int32_t persist_read_int(int key);
void persist_write_int(int key,int32_t value);
#else
static inline bool persist_exists(int key) { (void)key; return false; }
static inline int32_t persist_read_int(int key) { (void)key; return 0; }
static inline void persist_write_int(int key, int32_t value) { (void)key; (void)value; }
#endif
#endif
#endif
