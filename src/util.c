#include "util.h"

#include <glib.h>
#include <string.h>

gchar *string_remove_at(gchar *str, glong pos) {
  glong str_len = strlen(str);
  gchar *new_str = g_new0(gchar, MAX(str_len, 1));
  gchar *buffer_source = str;
  gchar *buffer_copy = new_str;
  glong i = 0;
  gint bytes;
  gunichar c;

  if (pos <= str_len && g_utf8_validate(str, -1, NULL)) {
    while (*buffer_source != '\0') {
      c = g_utf8_get_char(buffer_source);
      buffer_source = g_utf8_next_char(buffer_source);
      if (i != pos) {
        bytes = g_unichar_to_utf8(c, buffer_copy);
        buffer_copy += bytes;
      }
      i++;
    }
  }

  return new_str;
}

gchar *string_insert_chars_at(gchar *str, gchar *chars, glong pos) {
  gchar *new_str = NULL;

  if (g_utf8_validate(str, -1, NULL) && g_utf8_validate(chars, -1, NULL) &&
      pos >= 0 && pos <= g_utf8_strlen(str, -1)) {
    gchar *from = g_utf8_substring(str, 0, pos);
    gchar *end = g_utf8_offset_to_pointer(str, pos);

    new_str = g_strconcat(from, chars, end, NULL);

    g_free(from);

  } else {
    new_str = g_new0(gchar, 1);
  }

  return new_str;
}

glong string_get_nb_bytes_until(gchar *str, glong until) {
  glong ret = 0;
  if (str) {
    gchar *sub = g_utf8_substring(str, 0, until);
    ret = strlen(sub);
    g_free(sub);
  }

  return ret;
}

void pixel_data_print(guint32 pixel) {
  const guint32 r = pixel >> 24 & 0xff;
  const guint32 g = pixel >> 16 & 0xff;
  const guint32 b = pixel >> 8 & 0xff;
  const guint32 a = pixel >> 0 & 0xff;

  g_debug("rgba(%u, %d, %u, %u)", r, g, b, a);
}
