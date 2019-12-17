#pragma once

#include "swappy.h"

bool application_init(struct swappy_state *state);
int application_run(struct swappy_state *state);
void application_finish(struct swappy_state *state);

