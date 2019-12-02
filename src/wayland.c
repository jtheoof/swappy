#define _POSIX_C_SOURCE 2000809L

#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "box.h"
#include "swappy.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

static bool guess_output_logical_geometry(struct swappy_output *output) {
  // TODO Implement
  g_critical("guessing output is not yet implemented");
  return false;
}

void apply_output_transform(enum wl_output_transform transform, int32_t *width,
                            int32_t *height) {
  if (transform & WL_OUTPUT_TRANSFORM_90) {
    int32_t tmp = *width;
    *width = *height;
    *height = tmp;
  }
}

static void xdg_output_handle_logical_position(
    void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {
  struct swappy_output *output = data;

  output->logical_geometry.x = x;
  output->logical_geometry.y = y;
}

static void xdg_output_handle_logical_size(void *data,
                                           struct zxdg_output_v1 *xdg_output,
                                           int32_t width, int32_t height) {
  struct swappy_output *output = data;

  output->logical_geometry.width = width;
  output->logical_geometry.height = height;
}

static void xdg_output_handle_done(void *data,
                                   struct zxdg_output_v1 *xdg_output) {
  struct swappy_output *output = data;

  // Guess the output scale from the logical size
  int32_t width = output->geometry.width;
  int32_t height = output->geometry.height;
  apply_output_transform(output->transform, &width, &height);
  output->logical_scale = (double)width / output->logical_geometry.width;
}

static void xdg_output_handle_name(void *data,
                                   struct zxdg_output_v1 *xdg_output,
                                   const char *name) {
  g_debug("xdg_output_v1: name: %s", name);
  struct swappy_output *output = data;
  output->name = strdup(name);
}

static void xdg_output_handle_description(void *data,
                                          struct zxdg_output_v1 *xdg_output,
                                          const char *name) {
  g_debug("xdg_output_v1: description: %s", name);
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};

static void output_handle_geometry(void *data, struct wl_output *wl_output,
                                   int32_t x, int32_t y, int32_t physical_width,
                                   int32_t physical_height, int32_t subpixel,
                                   const char *make, const char *model,
                                   int32_t transform) {
  struct swappy_output *output = data;

  output->geometry.x = x;
  output->geometry.y = y;
  output->transform = transform;
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
                               uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh) {
  struct swappy_output *output = data;

  if ((flags & WL_OUTPUT_MODE_CURRENT) != 0) {
    output->geometry.width = width;
    output->geometry.height = height;
  }
}

static void output_handle_done(void *data, struct wl_output *wl_output) {
  // No-op
}

static void output_handle_scale(void *data, struct wl_output *wl_output,
                                int32_t factor) {
  struct swappy_output *output = data;
  output->scale = factor;
}

static const struct wl_output_listener output_listener = {
    .geometry = output_handle_geometry,
    .mode = output_handle_mode,
    .done = output_handle_done,
    .scale = output_handle_scale,
};

bool wayland_screencopy_geometry(struct swappy_state *state) {
  struct swappy_box *geometry = g_new(struct swappy_box, 1);
  char *geometry_str = state->geometry_str;
  state->geometry = geometry;

  if (!box_parse(geometry, geometry_str)) {
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
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct swappy_output *output = calloc(1, sizeof(struct swappy_output));
    output->state = state;
    output->scale = 1;
    output->wl_output =
        wl_registry_bind(registry, name, &wl_output_interface, 3);
    wl_output_add_listener(output->wl_output, &output_listener, output);
    wl_list_insert(&state->outputs, &output->link);
    bound = true;
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    state->layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version);
    bound = true;
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    state->xdg_output_manager = wl_registry_bind(
        registry, name, &zxdg_output_manager_v1_interface, version);
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
    g_warning("cannot connect to wayland display");
    return false;
  }

  g_debug("connected to wayland display");

  wl_list_init(&state->outputs);
  state->registry = wl_display_get_registry(state->display);
  wl_registry_add_listener(state->registry, &registry_listener, state);
  wl_display_roundtrip(state->display);

  if (state->compositor == NULL) {
    g_warning("compositor doesn't support wl_compositor");
    return false;
  }
  if (state->shm == NULL) {
    g_warning("compositor doesn't support wl_shm");
    return false;
  }

  if (wl_list_empty(&state->outputs)) {
    g_warning("no wl_output found");
    return false;
  }

  if (state->xdg_output_manager != NULL) {
    struct swappy_output *output;
    wl_list_for_each(output, &state->outputs, link) {
      output->xdg_output = zxdg_output_manager_v1_get_xdg_output(
          state->xdg_output_manager, output->wl_output);
      zxdg_output_v1_add_listener(output->xdg_output, &xdg_output_listener,
                                  output);
    }

    wl_display_dispatch(state->display);
    wl_display_roundtrip(state->display);
  } else {
    g_warning(
        "zxdg_output_manager_v1 isn't available, guessing the output layout");

    struct swappy_output *output;
    bool output_guessed = false;
    wl_list_for_each(output, &state->outputs, link) {
      output_guessed = guess_output_logical_geometry(output);
    }
    if (!output_guessed) {
      g_warning("could not guess output logical geometry");
      return false;
    }
  }
  if (state->layer_shell == NULL) {
    g_warning("compositor doesn't support zwlr_layer_shell_v1");
    return false;
  }

  if (state->zwlr_screencopy_manager == NULL) {
    g_warning("compositor does not support zwlr_screencopy_v1");
    return false;
  }

  return true;
}

void wayland_finish(struct swappy_state *state) {
  g_debug("cleaning up wayland resources");

  struct swappy_output *output;
  struct swappy_output *output_tmp;
  wl_list_for_each_safe(output, output_tmp, &state->outputs, link) {
    wl_list_remove(&output->link);
    free(output->name);
    if (output->screencopy_frame != NULL) {
      zwlr_screencopy_frame_v1_destroy(output->screencopy_frame);
    }
    // destroy_buffer(output->buffer);
    if (output->xdg_output != NULL) {
      zxdg_output_v1_destroy(output->xdg_output);
    }
    wl_output_release(output->wl_output);
    free(output);
  }

  if (state->compositor != NULL) {
    wl_compositor_destroy(state->compositor);
  }

  if (state->layer_shell != NULL) {
    zwlr_layer_shell_v1_destroy(state->layer_shell);
  }

  if (state->zwlr_screencopy_manager != NULL) {
    zwlr_screencopy_manager_v1_destroy(state->zwlr_screencopy_manager);
  }

  if (state->xdg_output_manager != NULL) {
    zxdg_output_manager_v1_destroy(state->xdg_output_manager);
  }

  if (state->shm != NULL) {
    wl_shm_destroy(state->shm);
  }

  if (state->registry != NULL) {
    wl_registry_destroy(state->registry);
  }

  if (state->display) {
    wl_display_disconnect(state->display);
  }
}