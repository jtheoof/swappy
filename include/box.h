#pragma once

#include "swappy.h"

bool box_parse(struct swappy_box *box, const char *str);
bool is_empty_box(struct swappy_box *box);
bool intersect_box(struct swappy_box *a, struct swappy_box *b);
