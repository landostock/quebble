#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "game.h"

void feedback_init(void);
void feedback_update(QbGameView view,bool focused);
void feedback_deinit(void);

#endif
