#include "screencopy.h"

#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "box.h"
#include "swappy.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"

static struct swappy_buffer *create_buffer(struct wl_shm *shm,
                                           enum wl_shm_format format,
                                           int32_t width, int32_t height,
                                           int32_t stride) {
  return NULL;
}

void screencopy_frame_handle_buffer(void *data,
                                    struct zwlr_screencopy_frame_v1 *frame,
                                    uint32_t format, uint32_t width,
                                    uint32_t height, uint32_t stride) {
  g_debug("screencopy: frame handler buffer: %dx%d", width, height);
  struct swappy_output *output = data;

  output->buffer =
      create_buffer(output->state->shm, format, width, height, stride);
  if (output->buffer == NULL) {
    g_warning("failed to create buffer");
    exit(EXIT_FAILURE);
  }

  //  zwlr_screencopy_frame_v1_copy(frame, output->buffer->wl_buffer);
}

void screencopy_frame_handle_flags(void *data,
                                   struct zwlr_screencopy_frame_v1 *frame,
                                   uint32_t flags) {
  g_debug("screencopy: frame flags: %d", flags);
  struct swappy_output *output = data;
  output->screencopy_frame_flags = flags;
}

void screencopy_frame_handle_ready(void *data,
                                   struct zwlr_screencopy_frame_v1 *frame,
                                   uint32_t tv_sec_hi, uint32_t tv_sec_lo,
                                   uint32_t tv_nsec) {
  struct swappy_output *output = data;
  g_debug("screencopy: frame handler is ready");
  ++output->state->n_done;
}

void screencopy_frame_handle_failed(void *data,
                                    struct zwlr_screencopy_frame_v1 *frame) {
  struct swappy_output *output = data;
  g_warning("screencopy: failed to copy output %s", output->name);
  exit(EXIT_FAILURE);
}

bool screencopy_init(struct swappy_state *state) {
  int32_t with_cursor = 0;
  size_t n_pending = 0;
  struct swappy_output *output;

  const struct zwlr_screencopy_frame_v1_listener screencopy_frame_listener = {
      .buffer = screencopy_frame_handle_buffer,
      .flags = screencopy_frame_handle_flags,
      .ready = screencopy_frame_handle_ready,
      .failed = screencopy_frame_handle_failed,
  };

  wl_list_for_each(output, &state->outputs, link) {
    if (state->geometry != NULL &&
        !intersect_box(state->geometry, &output->logical_geometry)) {
      continue;
    }

    output->screencopy_frame = zwlr_screencopy_manager_v1_capture_output(
        state->zwlr_screencopy_manager, with_cursor, output->wl_output);
    zwlr_screencopy_frame_v1_add_listener(output->screencopy_frame,
                                          &screencopy_frame_listener, output);

    ++n_pending;
  }
  if (n_pending == 0) {
    g_warning("screencopy: region is empty");
    return EXIT_FAILURE;
  }

  bool done = false;
  while (!done && wl_display_dispatch(state->display) != -1) {
    done = (state->n_done == n_pending);
  }
  if (!done) {
    g_warning("failed to screenshoot all outputs");
    return EXIT_FAILURE;
  }

  return true;
}

bool screencopy_parse_geometry(struct swappy_state *state) {
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
