
#include "util.h"

#include <glib.h>
#include <string.h>

void string_remove_at(gchar *str, int pos) {
  memmove(&str[pos], &str[pos + 1], strlen(str) - pos);
}

gchar *string_insert_char_at(gchar *str, gchar c, int pos) {
  gchar *new_str;
  int n = strlen(str);

  if (str) {
    new_str = g_new(gchar, n + 1 + 1);  // one for the new char, one for the \0

    for (int i = 0, j = 0; j < n + 1 || i < n; i++, j++) {
      if (j == pos) {
        new_str[j] = c;
        j++;
      }
      new_str[j] = str[i];
    }

    new_str[n + 1] = str[n];
  } else {
    new_str = NULL;
  }

  return new_str;
}