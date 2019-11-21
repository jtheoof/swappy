#define _POSIX_C_SOURCE 2
#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-cursor.h>

#include "application.h"
#include "swappy.h"
#include "wayland.h"

int main(int argc, char *argv[]) {
  struct swappy_state state = {0};

  state.argc = argc;
  state.argv = argv;
  state.mode = SWAPPY_PAINT_MODE_BRUSH;

  sprintf(state.image, "%s", "/home/jattali/Desktop/sway.png");

  if (!application_init(&state)) {
    exit(1);
  }

  application_run(&state);

  application_finish(&state);
}
