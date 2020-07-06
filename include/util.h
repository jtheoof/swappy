
#pragma once

#include <glib.h>

glong string_get_nb_bytes_until(gchar *str, glong until);
gchar *string_remove_at(char *str, glong pos);
gchar *string_insert_chars_at(gchar *str, gchar *chars, glong pos);
void pixel_data_print(guint32 pixel);
