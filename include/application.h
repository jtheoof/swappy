#pragma once

#include "swappy.h"

bool application_init(struct swappy_state *state);
int application_run(struct swappy_state *state);
void application_finish(struct swappy_state *state);

/* Glade signals */
void window_keypress_handler(GtkWidget *widget, GdkEventKey *event,
                             struct swappy_state *state);
gboolean window_delete_handler(GtkWidget *widget, GdkEvent *event,
                               struct swappy_state *state);

void undo_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void redo_clicked_handler(GtkWidget *widget, struct swappy_state *state);

gboolean draw_area_handler(GtkWidget *widget, cairo_t *cr,
                           struct swappy_state *state);
gboolean draw_area_configure_handler(GtkWidget *widget,
                                     GdkEventConfigure *event,
                                     struct swappy_state *state);
void draw_area_button_press_handler(GtkWidget *widget, GdkEventButton *event,
                                    struct swappy_state *state);
void draw_area_button_release_handler(GtkWidget *widget, GdkEventButton *event,
                                      struct swappy_state *state);
void draw_area_motion_notify_handler(GtkWidget *widget, GdkEventMotion *event,
                                     struct swappy_state *state);

void brush_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void text_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void rectangle_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void ellipse_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void arrow_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void blur_clicked_handler(GtkWidget *widget, struct swappy_state *state);

void copy_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void save_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void clear_clicked_handler(GtkWidget *widget, struct swappy_state *state);

void color_red_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void color_green_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void color_blue_clicked_handler(GtkWidget *widget, struct swappy_state *state);

void color_custom_clicked_handler(GtkWidget *widget,
                                  struct swappy_state *state);
void color_custom_color_set_handler(GtkWidget *widget,
                                    struct swappy_state *state);

void stroke_size_decrease_handler(GtkWidget *widget,
                                  struct swappy_state *state);
void stroke_size_reset_handler(GtkWidget *widget, struct swappy_state *state);
void stroke_size_increase_handler(GtkWidget *widget,
                                  struct swappy_state *state);

void text_size_decrease_handler(GtkWidget *widget, struct swappy_state *state);
void text_size_reset_handler(GtkWidget *widget, struct swappy_state *state);
void text_size_increase_handler(GtkWidget *widget, struct swappy_state *state);