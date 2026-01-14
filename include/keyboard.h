#pragma once

#include <gtk/gtk.h>

#include "swappy.h"

guint keyboard_keysym_for_shortcuts(enum swappy_keyboard_shortcuts mode,
                                    GdkEventKey *event);
