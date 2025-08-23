#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// replace %s in str with file
static char *replace_string(const char *str, const char *file) {
  GString *result = g_string_new(NULL);
  const char *p = str;
  const char *found;
  while ((found = strstr(p, "%s")) != NULL) {
    g_string_append_len(result, p, found - p);
    g_string_append(result, file);
    p = found + 2;
  }
  g_string_append(result, p);
  return g_string_free(result, FALSE);
}

char *execute_ocr_command(char **ocr_cmd, gsize ocr_cmd_len, char *file) {
  GError *error = NULL;
  char *standard_output = NULL;
  char *standard_error = NULL;
  int exit_status = 0;
  GPtrArray *args_array = g_ptr_array_new();

  for (size_t i = 0; i < ocr_cmd_len; i++) {
    const char *arg = ocr_cmd[i];
    if (strstr(arg, "%s") != NULL) {
      char *new_arg = replace_string(arg, file);
      g_ptr_array_add(args_array, new_arg);
    } else {
      g_ptr_array_add(args_array, g_strdup(arg));
    }
  }
  g_ptr_array_add(args_array, NULL);

  gboolean success = g_spawn_sync(
      NULL, (char **)args_array->pdata, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
      &standard_output, &standard_error, &exit_status, &error);

  for (size_t i = 0; i < args_array->len - 1; i++) {
    g_free(g_ptr_array_index(args_array, i));
  }
  g_ptr_array_free(args_array, TRUE);

  if (!success) {
    g_printerr("execute_ocr_command: %s\n", error->message);
    g_error_free(error);
    g_free(standard_output);
    g_free(standard_error);
    return NULL;
  }

  if (exit_status != 0) {
    g_printerr("execute_ocr_command: exit_status:%d\n", exit_status);
    if (standard_error != NULL) {
      g_printerr("stderr: %s\n", standard_error);
    }
    g_free(standard_output);
    g_free(standard_error);
    return NULL;
  }
  g_free(standard_error);
  return standard_output;  // you should free this outside
}
