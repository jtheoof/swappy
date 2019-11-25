#define _POSIX_C_SOURCE 2
#include <errno.h>
#include <getopt.h>
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

// static const char usage[] =
//     "Usage: swappy [OPTIONS...]\n"
//     "\n"
//     "Options:\n"
//     "  -h, --help                   Show help message and quit.\n"
//     "  -g, --geometry=\"x,y wxh\"   Set the region to capture.\n"
//     "                               (Can be an output of slurp)\n";

// static bool parse_options(struct swappy_state *state) {
//   int argc = state->argc;
//   char **argv = state->argv;

//   int c;

//   while (1) {
//     int option_index = 0;

//     static struct option long_options[] = {
//         {"help", optional_argument, NULL, 'h'},
//         {"geometry", required_argument, NULL, 'g'},
//         {0, 0, 0, 0}};

//     c = getopt_long(argc, argv, "hg:", long_options, &option_index);
//     if (c == -1) break;

//     switch (c) {
//       case 'h':
//         printf(usage);
//         break;
//       case 'g':
//         g_debug("geometry is: %s", optarg);
//         break;
//       default:
//         printf("waaaaaaaaaaaa\n");
//     }
//   }

//   if (optind < argc) {
//     g_error("invalid usage");
//     printf(usage);
//     return false;
//   }

//   return true;
// }

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

  if (!wayland_init(&state)) {
    g_critical("failed to initialize wayland");
    exit(1);
  }

  if (!application_init(&state)) {
    g_critical("failed to initialize gtk application");
    exit(1);
  }

  application_run(&state);

  application_finish(&state);
  wayland_finish(&state);
}
