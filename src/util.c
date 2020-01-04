
#include "util.h"

#include <glib.h>
#include <string.h>

void string_remove_at(gchar *str, size_t pos) {
  if (str && strlen(str) > pos) {
    memmove(&str[pos], &str[pos + 1], strlen(str) - pos);
  }
}

gchar *string_insert_chars_at(gchar *str, gchar *chars, size_t pos) {
  gchar *new_str;

  if (str && chars) {
    size_t n = strlen(str);
    size_t m = strlen(chars);
    size_t i = 0, j = 0;

    new_str = g_new(gchar, n + m + 1);

    while (j < n + m) {
      if (j == pos) {
        for (size_t k = 0; k < m; k++) {
          new_str[j++] = chars[k];
        }
      } else {
        new_str[j++] = str[i++];
      }
    }

    new_str[j] = '\0';
  } else {
    new_str = NULL;
  }

  return new_str;
}