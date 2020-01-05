#define _POSIX_C_SOURCE 2000809L

#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "buffer.h"
#include "swappy.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"

#ifdef HAVE_WAYLAND_PROTOCOLS
#include "xdg-output-unstable-v1-client-protocol.h"
#endif

static bool guess_output_logical_geometry(struct swappy_output *output) {
  // TODO Implement
  g_warning("guessing output is not yet implemented");
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

#ifdef HAVE_WAYLAND_PROTOCOLS
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
  struct swappy_output *output = data;
  output->name = strdup(name);
}

static void xdg_output_handle_description(void *data,
                                          struct zxdg_output_v1 *xdg_output,
                                          const char *name) {}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};
#endif

static void seat_capabilities(void *data, struct wl_seat *wl_seat,
                              uint32_t capabilities) {}

static void seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
  g_debug("received seat name: %s", name);
}

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

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t name, const char *interface,
                                    uint32_t version) {
  g_debug("got a registry event for interface: %s, name: %d, version: %d",
          interface, name, version);

  struct swappy_state *state = data;
  bool bound = false;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    state->wl->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
    bound = true;
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    state->wl->shm =
        wl_registry_bind(registry, name, &wl_shm_interface, version);
    bound = true;
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct swappy_output *output = calloc(1, sizeof(struct swappy_output));
    output->state = state;
    output->scale = 1;
    output->wl_output =
        wl_registry_bind(registry, name, &wl_output_interface, 3);
    wl_output_add_listener(output->wl_output, &output_listener, output);
    wl_list_insert(&state->wl->outputs, &output->link);
    bound = true;
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    struct swappy_wl_seat *seat = calloc(1, sizeof(struct swappy_wl_seat));
    seat->state = state;
    seat->wl_seat =
        wl_registry_bind(registry, name, &wl_seat_interface, version);
    wl_seat_add_listener(seat->wl_seat, &seat_listener, seat);
    wl_list_insert(&state->wl->seats, &seat->link);
    bound = true;
  } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) ==
             0) {
    state->wl->zwlr_screencopy_manager = wl_registry_bind(
        registry, name, &zwlr_screencopy_manager_v1_interface, version);
    bound = true;
  } else if (strcmp(interface, zwlr_data_control_manager_v1_interface.name) ==
             0) {
    state->wl->zwlr_data_control_manager = wl_registry_bind(
        registry, name, &zwlr_data_control_manager_v1_interface, version);
    bound = true;
  } else {
#ifdef HAVE_WAYLAND_PROTOCOLS
    if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
      state->wl->xdg_output_manager = wl_registry_bind(
          registry, name, &zxdg_output_manager_v1_interface, version);
      bound = true;
    }
#endif
  }

  if (bound) {
    g_debug("bound registry: %s", interface);
  }
}

static void global_registry_remove_handler(void *data,
                                           struct wl_registry *wl_registry,
                                           uint32_t name) {}

static struct wl_registry_listener registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remove_handler,
};

bool wayland_init(struct swappy_state *state) {
  state->wl = g_new(struct swappy_wayland, 1);

  state->wl->display = wl_display_connect(NULL);
  state->wl->n_done = 0;
  if (state->wl->display == NULL) {
    g_warning("cannot connect to wayland display");
    return false;
  }

  wl_list_init(&state->wl->outputs);
  wl_list_init(&state->wl->seats);
  state->wl->registry = wl_display_get_registry(state->wl->display);
  wl_registry_add_listener(state->wl->registry, &registry_listener, state);
  wl_display_roundtrip(state->wl->display);

  if (state->wl->compositor == NULL) {
    g_warning("compositor doesn't support wl_compositor");
    return false;
  }
  if (state->wl->shm == NULL) {
    g_warning("compositor doesn't support wl_shm");
    return false;
  }

  if (wl_list_empty(&state->wl->outputs)) {
    g_warning("no wl_output found");
    return false;
  }

  if (wl_list_empty(&state->wl->seats)) {
    g_warning("no wl_seats found");
    return false;
  }

  bool found_output_layout = false;
#ifdef HAVE_WAYLAND_PROTOCOLS
  if (state->wl->xdg_output_manager != NULL) {
    struct swappy_output *output;
    wl_list_for_each(output, &state->wl->outputs, link) {
      output->xdg_output = zxdg_output_manager_v1_get_xdg_output(
          state->wl->xdg_output_manager, output->wl_output);
      zxdg_output_v1_add_listener(output->xdg_output, &xdg_output_listener,
                                  output);
    }

    wl_display_dispatch(state->wl->display);
    wl_display_roundtrip(state->wl->display);
    found_output_layout = true;
  }
#endif
  if (!found_output_layout) {
    g_warning(
        "zxdg_output_manager_v1 isn't available, guessing the output layout");

    struct swappy_output *output;
    bool output_guessed = false;
    wl_list_for_each(output, &state->wl->outputs, link) {
      output_guessed = guess_output_logical_geometry(output);
    }
    if (!output_guessed) {
      g_warning("could not guess output logical geometry");
      return false;
    }
  }

  if (state->wl->zwlr_screencopy_manager == NULL) {
    g_warning(
        "compositor does not support zwlr_screencopy_v1, -g option will not "
        "work");
  } else if (state->wl->zwlr_data_control_manager == NULL) {
    g_warning(
        "compositor does not support zwlr_data_control_v1, copy will fallback "
        "to gdk clipboard");
  }

  return true;
}

void wayland_finish(struct swappy_state *state) {
  struct swappy_output *output;
  struct swappy_output *output_tmp;
  struct swappy_wl_seat *seat;
  struct swappy_wl_seat *seat_tmp;

  if (!state->wl) {
    return;
  }

  wl_list_for_each_safe(output, output_tmp, &state->wl->outputs, link) {
    wl_list_remove(&output->link);
    free(output->name);
    if (output->screencopy_frame != NULL) {
      zwlr_screencopy_frame_v1_destroy(output->screencopy_frame);
    }
#ifdef HAVE_WAYLAND_PROTOCOLS
    if (output->xdg_output != NULL) {
      zxdg_output_v1_destroy(output->xdg_output);
    }
#endif
    wl_output_release(output->wl_output);
    buffer_wayland_destroy(output->buffer);
    free(output);
  }

  wl_list_for_each_safe(seat, seat_tmp, &state->wl->seats, link) {
    wl_list_remove(&seat->link);
    wl_seat_release(seat->wl_seat);
    free(seat);
  }

  if (state->wl->compositor != NULL) {
    wl_compositor_destroy(state->wl->compositor);
  }

  if (state->wl->zwlr_screencopy_manager != NULL) {
    zwlr_screencopy_manager_v1_destroy(state->wl->zwlr_screencopy_manager);
  }

  if (state->wl->zwlr_data_control_manager != NULL) {
    zwlr_data_control_manager_v1_destroy(state->wl->zwlr_data_control_manager);
  }

#ifdef HAVE_WAYLAND_PROTOCOLS
  if (state->wl->xdg_output_manager != NULL) {
    zxdg_output_manager_v1_destroy(state->wl->xdg_output_manager);
  }
#endif

  if (state->wl->shm != NULL) {
    wl_shm_destroy(state->wl->shm);
  }

  if (state->wl->registry != NULL) {
    wl_registry_destroy(state->wl->registry);
  }

  if (state->wl->display) {
    wl_display_disconnect(state->wl->display);
  }

  g_free(state->wl);
  state->wl = NULL;
}