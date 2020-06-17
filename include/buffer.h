#pragma once

#include "swappy.h"

bool buffer_init_from_file(struct swappy_state *state);
void buffer_resize_patterns(struct swappy_state *state);
void buffer_free_all(struct swappy_state *state);
