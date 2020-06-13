#define _POSIX_C_SOURCE 200809L

#include "application.h"
#include "config.h"

int main(int argc, char *argv[]) {
  struct swappy_state state = {0};
  int status;

  state.argc = argc;
  state.argv = argv;
  state.mode = SWAPPY_PAINT_MODE_BRUSH;

  if (!application_init(&state)) {
    g_critical("failed to initialize gtk application");
    exit(1);
  }

  status = application_run(&state);

  if (status == 0) {
    gtk_main();
  }

  application_finish(&state);

  return status;
}
