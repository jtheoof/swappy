#define _POSIX_C_SOURCE 2

#include "application.h"
#include "config.h"
#include "wayland.h"

int main(int argc, char *argv[]) {
  struct swappy_state state = {0};
  int status;

  state.argc = argc;
  state.argv = argv;
  state.mode = SWAPPY_PAINT_MODE_BRUSH;

  if (!config_get_storage_path(&state)) {
    g_critical("could not find a valid pictures path in your env variables");
    exit(1);
  }

  if (!application_init(&state)) {
    g_critical("failed to initialize gtk application");
    exit(1);
  }

  if (!wayland_init(&state)) {
    g_critical("failed to initialize wayland");
    exit(1);
  }

  status = application_run(&state);

  if (status == 0) {
    gtk_main();
  }

  application_finish(&state);
  wayland_finish(&state);

  return status;
}
