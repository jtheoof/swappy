#pragma once

#include <gtk/gtk.h>

#include "swappy.h"

guint keyboard_keysym_for_shortcuts(struct swappy_state *state,
                                    GdkEventKey *event);
