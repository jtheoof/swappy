#include "buffer.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "box.h"
#include "wayland.h"

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

static cairo_format_t get_cairo_format(enum wl_shm_format wl_fmt) {
  switch (wl_fmt) {
    case WL_SHM_FORMAT_ARGB8888:
      return CAIRO_FORMAT_ARGB32;
    case WL_SHM_FORMAT_XRGB8888:
      return CAIRO_FORMAT_RGB24;
    default:
      return CAIRO_FORMAT_INVALID;
  }
}

static int get_output_flipped(enum wl_output_transform transform) {
  return transform & WL_OUTPUT_TRANSFORM_FLIPPED ? -1 : 1;
}

static void apply_output_transform(enum wl_output_transform transform,
                                   int32_t *width, int32_t *height) {
  if (transform & WL_OUTPUT_TRANSFORM_90) {
    int32_t tmp = *width;
    *width = *height;
    *height = tmp;
  }
}

static struct swappy_buffer *create_buffer(struct wl_shm *shm,
                                           enum wl_shm_format format,
                                           int32_t width, int32_t height,
                                           int32_t stride) {
  size_t size = stride * height;

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

static void screencopy_frame_handle_buffer(
    void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t format,
    uint32_t width, uint32_t height, uint32_t stride) {
  struct swappy_output *output = data;

  output->buffer =
      create_buffer(output->state->wl->shm, format, width, height, stride);
  if (output->buffer == NULL) {
    g_warning("failed to create buffer");
    exit(EXIT_FAILURE);
  }

  zwlr_screencopy_frame_v1_copy(frame, output->buffer->wl_buffer);
}

static void screencopy_frame_handle_flags(
    void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t flags) {
  struct swappy_output *output = data;
  output->screencopy_frame_flags = flags;
}

static void screencopy_frame_handle_ready(
    void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t tv_sec_hi,
    uint32_t tv_sec_lo, uint32_t tv_nsec) {
  struct swappy_output *output = data;
  ++output->state->wl->n_done;
}

static void screencopy_frame_handle_failed(
    void *data, struct zwlr_screencopy_frame_v1 *frame) {
  struct swappy_output *output = data;
  g_warning("screencopy: failed to copy output %s", output->name);
  exit(EXIT_FAILURE);
}

bool buffer_init_from_screencopy(struct swappy_state *state) {
  struct swappy_box *geometry = state->geometry;
  int32_t with_cursor = 0;
  size_t n_pending = 0;
  struct swappy_output *output;

  const struct zwlr_screencopy_frame_v1_listener screencopy_frame_listener = {
      .buffer = screencopy_frame_handle_buffer,
      .flags = screencopy_frame_handle_flags,
      .ready = screencopy_frame_handle_ready,
      .failed = screencopy_frame_handle_failed,
  };

  wl_list_for_each(output, &state->wl->outputs, link) {
    if (state->geometry != NULL &&
        !intersect_box(state->geometry, &output->logical_geometry)) {
      continue;
    }

    output->screencopy_frame = zwlr_screencopy_manager_v1_capture_output(
        state->wl->zwlr_screencopy_manager, with_cursor, output->wl_output);
    zwlr_screencopy_frame_v1_add_listener(output->screencopy_frame,
                                          &screencopy_frame_listener, output);

    ++n_pending;
  }
  if (n_pending == 0) {
    g_warning("screencopy: region is empty");
    return EXIT_FAILURE;
  }

  bool done = false;
  while (!done && wl_display_dispatch(state->wl->display) != -1) {
    done = (state->wl->n_done == n_pending);
  }
  if (!done) {
    g_warning("failed to screenshot all outputs");
    return EXIT_FAILURE;
  }

  wl_list_for_each(output, &state->wl->outputs, link) {
    struct swappy_buffer *buffer = output->buffer;

    if (output->buffer == NULL) {
      // screencopy buffer is empty, cannot draw it onto the paint area"
      continue;
    }

    cairo_format_t format = get_cairo_format(buffer->format);

    g_assert(format != CAIRO_FORMAT_INVALID);

    int32_t output_x = output->logical_geometry.x - geometry->x;
    int32_t output_y = output->logical_geometry.y - geometry->y;
    int32_t output_width = output->logical_geometry.width;
    int32_t output_height = output->logical_geometry.height;
    int32_t scale = output->scale;

    int32_t raw_output_width = output->geometry.width;
    int32_t raw_output_height = output->geometry.height;
    apply_output_transform(output->transform, &raw_output_width,
                           &raw_output_height);

    int output_flipped_x = get_output_flipped(output->transform);
    int output_flipped_y =
        output->screencopy_frame_flags & ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT
            ? -1
            : 1;

    cairo_surface_t *output_surface = cairo_image_surface_create_for_data(
        buffer->data, format, buffer->width, buffer->height, buffer->stride);
    cairo_pattern_t *output_pattern =
        cairo_pattern_create_for_surface(output_surface);

    // All transformations are in pattern-local coordinates
    cairo_matrix_t matrix;
    cairo_matrix_init_identity(&matrix);
    cairo_matrix_translate(&matrix, (double)output->geometry.width / 2,
                           (double)output->geometry.height / 2);
    //    cairo_matrix_rotate(&matrix, -get_output_rotation(output->transform));
    cairo_matrix_scale(
        &matrix, (double)raw_output_width / output_width * output_flipped_x,
        (double)raw_output_height / output_height * output_flipped_y);
    cairo_matrix_translate(&matrix, -(double)output_width / 2,
                           -(double)output_height / 2);
    cairo_matrix_translate(&matrix, -output_x, -output_y);
    cairo_matrix_scale(&matrix, 1 / scale, 1 / scale);
    cairo_pattern_set_matrix(output_pattern, &matrix);

    cairo_pattern_set_filter(output_pattern, CAIRO_FILTER_BEST);

    state->patterns = g_list_append(state->patterns, output_pattern);

    cairo_surface_destroy(output_surface);
  }

  return true;
}

bool buffer_init_from_file(struct swappy_state *state) {
  char *file = state->file_str;

  cairo_surface_t *surface = cairo_image_surface_create_from_png(file);
  cairo_status_t status = cairo_surface_status(surface);

  if (status) {
    g_warning("error while loading png file: %s - cairo status: %s", file,
              cairo_status_to_string(status));
    return false;
  }

  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);

  struct swappy_box *geometry = g_new(struct swappy_box, 1);

  geometry->x = 0;
  geometry->y = 0;
  geometry->width = (int32_t)width;
  geometry->height = (int32_t)height;

  state->geometry = geometry;

  cairo_pattern_t *output_pattern = cairo_pattern_create_for_surface(surface);
  state->patterns = g_list_append(state->patterns, output_pattern);

  cairo_surface_destroy(surface);

  return true;
}

bool buffer_parse_geometry(struct swappy_state *state) {
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

void buffer_wayland_destroy(struct swappy_buffer *buffer) {
  if (buffer == NULL) {
    return;
  }
  munmap(buffer->data, buffer->size);
  wl_buffer_destroy(buffer->wl_buffer);
  free(buffer);
}

static void free_pattern(gpointer data) {
  cairo_pattern_t *pattern = data;
  cairo_pattern_destroy(pattern);
}

void buffer_free_all(struct swappy_state *state) {
  if (state->patterns) {
    g_list_free_full(state->patterns, free_pattern);
    state->patterns = NULL;
  }
}