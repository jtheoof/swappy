#define _POSIX_C_SOURCE 200112L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wordexp.h>

#include "swappy.h"

static bool folder_exists(const char *path) {
  struct stat sb;

  stat(path, &sb);

  return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
}

bool config_get_storage_path(struct swappy_state *state) {
  static const char *storage_paths[] = {
      "$XDG_DESKTOP_DIR",
      "$XDG_CONFIG_HOME/Desktop",
      "$HOME/Desktop",
  };

  for (size_t i = 0; i < sizeof(storage_paths) / sizeof(char *); ++i) {
    wordexp_t p;
    if (wordexp(storage_paths[i], &p, 0) == 0) {
      char *path = g_strdup(p.we_wordv[0]);
      wordfree(&p);
      if (folder_exists(path)) {
        g_info("storage path is: %s", path);
        state->storage_path = path;
        return true;
      }
      g_free(path);
    }
  }

  return false;
}