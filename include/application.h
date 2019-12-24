#pragma once

#include "swappy.h"

bool application_init(struct swappy_state *state);
int application_run(struct swappy_state *state);
void application_finish(struct swappy_state *state);

/* Glade signals */
void keypress_handler(GtkWidget *widget, GdkEventKey *event,
                      struct swappy_state *state);

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

void copy_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void save_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void clear_clicked_handler(GtkWidget *widget, struct swappy_state *state);
