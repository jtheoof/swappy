#define _POSIX_C_SOURCE 2
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-cursor.h>
#include <linux/input-event-codes.h>

#include "swappy.h"
#include "wayland.h"

int main(int argc, char *argv[]) {
	struct swappy_state state = {0};

	state.argc = argc;
	state.argv = argv;

	if (!wayland_init(&state)) {
		exit(1);
	}

	wayland_finish(&state);
}
