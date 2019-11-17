#ifndef _SWAPPY_H
#define _SWAPPY_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>

struct swappy_state {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zxdg_output_manager_v1 *xdg_output_manager;
	struct wl_list outputs; // mako_output::link
	struct wl_list seats; // mako_seat::link

	struct wl_surface *surface;
	struct mako_output *surface_output;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct mako_output *layer_surface_output;

	int argc;
	char **argv;
};

#endif
