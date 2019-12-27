#pragma once

#include "swappy.h"

void paint_add_temporary(struct swappy_state *state, double x, double y,
                         enum swappy_paint_type type);
void paint_update_temporary(struct swappy_state *state, double x, double y);
void paint_commit_temporary(struct swappy_state *state);

void paint_free(gpointer data);
void paint_free_all(struct swappy_state *state);
void paint_free_list(GList **list);
