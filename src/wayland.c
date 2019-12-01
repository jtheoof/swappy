#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "swappy.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"

static bool parse_box(struct swappy_box *box, const char *str) {
  char *end = NULL;
  box->x = strtol(str, &end, 10);
  if (end[0] != ',') {
    return false;
  }

  char *next = end + 1;
  box->y = strtol(next, &end, 10);
  if (end[0] != ' ') {
    return false;
  }

  next = end + 1;
  box->width = strtol(next, &end, 10);
  if (end[0] != 'x') {
    return false;
  }

  next = end + 1;
  box->height = strtol(next, &end, 10);
  if (end[0] != '\0') {
    return false;
  }

  return true;
}

bool wayland_screencopy_geometry(struct swappy_state *state) {
  struct swappy_box *geometry = g_new(struct swappy_box, 1);
  char *geometry_str = state->geometry_str;
  state->geometry = geometry;

  if (!parse_box(geometry, geometry_str)) {
    g_critical("%s is not a valid geometry, must follow the pattern \"%s",
               geometry_str, GEOMETRY_PATTERN);
    return false;
  }
  return true;
}

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t name, const char *interface,
                                    uint32_t version) {
  g_debug("got a registry event for interface: %s, name: %d", interface, name);

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
  } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) ==
             0) {
    state->zwlr_screencopy_manager = wl_registry_bind(
        registry, name, &zwlr_screencopy_manager_v1_interface, 1);
    bound = true;
  }

  if (bound) {
    g_debug("bound registry: %s", interface);
  }
}

static void global_registry_remove_handler(void *data,
                                           struct wl_registry *wl_registry,
                                           uint32_t name) {
  g_debug("got a registry remove event for name: %d", name);
}

static struct wl_registry_listener registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remove_handler,
};

bool wayland_init(struct swappy_state *state) {
  state->display = wl_display_connect(NULL);
  if (state->display == NULL) {
    g_error("cannot connect to wayland display");
    return false;
  }

  g_debug("connected to wayland display");

  state->registry = wl_display_get_registry(state->display);
  wl_registry_add_listener(state->registry, &registry_listener, state);
  wl_display_roundtrip(state->display);

  if (state->compositor == NULL) {
    g_error("compositor doesn't support wl_compositor");
    return false;
  }
  if (state->shm == NULL) {
    g_error("compositor doesn't support wl_shm");
    return false;
  }
  if (state->layer_shell == NULL) {
    g_error("compositor doesn't support zwlr_layer_shell_v1");
    return false;
  }
  if (state->zwlr_screencopy_manager == NULL) {
    g_error("compositor does not support zwlr_screencopy_v1");
  }

  return true;
}

void wayland_finish(struct swappy_state *state) {
  wl_display_disconnect(state->display);
  g_debug("disconnected from wayland display");
}