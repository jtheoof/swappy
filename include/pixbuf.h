#pragma once

#include "swappy.h"

GdkPixbuf *pixbuf_get_from_state(struct swappy_state *state);
void pixbuf_save_state_to_folder(GdkPixbuf *pixbuf, char *folder);
void pixbuf_save_to_file(GdkPixbuf *pixbuf, char *file);
void pixbuf_save_to_stdout(GdkPixbuf *pixbuf);