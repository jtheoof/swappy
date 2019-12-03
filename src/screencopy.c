#include "screencopy.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <wayland-client.h>

#include "box.h"
#include "swappy.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"

static void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

static int anonymous_shm_open(void) {
  char name[] = "/swappy-XXXXXX";
  int retries = 100;

  do {
    randname(name + strlen(name) - 6);

    --retries;
    // shm_open guarantees that O_CLOEXEC is set
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);

  return -1;
}

static int create_shm_file(off_t size) {
  int fd = anonymous_shm_open();
  if (fd < 0) {
    return fd;
  }

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

static struct swappy_buffer *create_buffer(struct wl_shm *shm,
                                           enum wl_shm_format format,
                                           int32_t width, int32_t height,
                                           int32_t stride) {
  size_t size = stride * height;
  g_debug("creating buffer with dimensions: %dx%d", width, height);

  int fd = create_shm_file(size);
  if (fd == -1) {
    return NULL;
  }

  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer *wl_buffer =
      wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool);

  close(fd);

  struct swappy_buffer *buffer = calloc(1, sizeof(struct swappy_buffer));
  buffer->wl_buffer = wl_buffer;
  buffer->data = data;
  buffer->width = width;
  buffer->height = height;
  buffer->stride = stride;
  buffer->size = size;
  buffer->format = format;

  return buffer;
}

void screencopy_destroy_buffer(struct swappy_buffer *buffer) {
  if (buffer == NULL) {
    return;
  }
  munmap(buffer->data, buffer->size);
  wl_buffer_destroy(buffer->wl_buffer);
  free(buffer);
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

    zwlr_screencopy_frame_v1_copy(frame, output->buffer->wl_buffer);
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
