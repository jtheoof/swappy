#pragma once

#include <gdk/gdk.h>

#include "swappy.h"

void paint_add_temporary(struct swappy_state *state, double x, double y,
                         enum swappy_paint_type type);
void paint_update_temporary_shape(struct swappy_state *state, double x,
                                  double y, gboolean is_control_pressed);
void paint_update_temporary_text(struct swappy_state *state,
                                 GdkEventKey *event);
void paint_update_temporary_text_clip(struct swappy_state *state, gdouble x,
                                      gdouble y);
void paint_commit_temporary(struct swappy_state *state);

void paint_get_crop_resize(enum swappy_resize *out_resize_x,
                           enum swappy_resize *out_resize_y,
                           const struct swappy_crop *crop,
                           double x, double y);
void paint_start_crop(struct swappy_crop *crop, double x, double y,
                      gboolean recreate);
void paint_update_crop(struct swappy_crop *crop, double x, double y);

void paint_free(gpointer data);
void paint_free_all(struct swappy_state *state);
void paint_free_list(GList **list);
