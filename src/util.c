
#include "util.h"

#include <glib.h>
#include <string.h>

void string_remove_at(gchar *str, int pos) {
  memmove(&str[pos], &str[pos + 1], strlen(str) - pos);
}

gchar *string_insert_chars_at(gchar *str, gchar *chars, int pos) {
  gchar *new_str;

  if (str && chars) {
    int n = strlen(str);
    int m = strlen(chars);
    int i = 0, j = 0;

    new_str = g_new(gchar, n + m + 1);

    while (j < n + m) {
      if (j == pos) {
        for (int k = 0; k < m; k++) {
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