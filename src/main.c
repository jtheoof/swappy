#define _POSIX_C_SOURCE 2
#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-cursor.h>
#include <wlr/util/log.h>

#include "application.h"
#include "config.h"
#include "swappy.h"
#include "wayland.h"

int main(int argc, char *argv[]) {
  struct swappy_state state = {0};

  state.argc = argc;
  state.argv = argv;
  state.mode = SWAPPY_PAINT_MODE_BRUSH;

  char *path = "/home/jattali/Desktop/sway.png";

  g_debug("Loading: %s", path);
  sprintf(state.image, "%s", path);

  if (!config_get_storage_path(&state)) {
    g_critical("could not find a valid pictures path in your env variables");
    exit(1);
  }

  if (!application_init(&state)) {
    g_critical("failed to init application");
    exit(1);
  }

  application_run(&state);

  application_finish(&state);
}
