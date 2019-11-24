#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "swappy.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

void global_registry_handler(void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  printf("got a registry event for interface: %s, name: %d\n", interface, name);

  struct swappy_state *state = data;
  bool bound = false;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    state->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
    bound = true;
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    state->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
    bound = true;
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    state->layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version);
    bound = true;
  }

  if (bound) {
    printf("bound registry: %s\n", interface);
  }
}

void global_registry_remove_handler(void *data, struct wl_registry *wl_registry,
                                    uint32_t name) {
  printf("got a registry remove event for name: %d\n", name);
}

struct wl_registry_listener registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remove_handler,
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