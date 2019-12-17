#pragma once

#include "swappy.h"

bool application_init(struct swappy_state *state);
int application_run(struct swappy_state *state);
void application_finish(struct swappy_state *state);

void brush_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void text_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void rectangle_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void ellipse_clicked_handler(GtkWidget *widget, struct swappy_state *state);
void arrow_clicked_handler(GtkWidget *widget, struct swappy_state *state);