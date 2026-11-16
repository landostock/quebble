#ifndef PIXEL_ART_H
#define PIXEL_ART_H

#include "quebble_config.h"

typedef void (*QbLogoVisitor)(int x, int y, bool settled, void *context);
void pixel_logo_visit(uint16_t tick, QbLogoVisitor visitor, void *context);
void pixel_text_visit(const char *text, int center_x, int top, int scale, uint16_t tick,
                      QbLogoVisitor visitor, void *context);

#endif
