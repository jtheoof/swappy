#include <stdio.h>

#include <wayland-client.h>

#include "swappy.h"

void global_cb (void *data,
		       struct wl_registry *wl_registry,
		       uint32_t name,
		       const char *interface,
		       uint32_t version) {

               }

void global_remove_cb (void *data,
			      struct wl_registry *wl_registry,
			      uint32_t name) {

                  }

struct wl_registry_listener registry_listener = {
    .global = global_cb,
    .global_remove = global_remove_cb,
};

bool wayland_init(struct swappy_state *state) {
	state->display = wl_display_connect(NULL);
	if (state->display == NULL) {
		fprintf(stderr, "cannot connect to wayland display\n");
        return false;
	}

	printf("connected to wayland display\n");


	state->registry = wl_display_get_registry(state->display);
	wl_registry_add_listener(state->registry, &registry_listener, state);
	wl_display_roundtrip(state->display);

	if (state->compositor == NULL) {
		fprintf(stderr, "compositor doesn't support wl_compositor\n");
		return false;
	}
	if (state->shm == NULL) {
		fprintf(stderr, "compositor doesn't support wl_shm\n");
		return false;
	}
	if (state->layer_shell == NULL) {
		fprintf(stderr, "compositor doesn't support zwlr_layer_shell_v1\n");
		return false;
	}

    return true;
}

void wayland_finish(struct swappy_state *state) {
	wl_display_disconnect(state->display);
	printf("disconnected from wayland display\n");
}