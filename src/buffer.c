#include "buffer.h"

#include <cairo.h>

#include "box.h"
#include "swappy.h"

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

  g_info("size of image: %dx%d", width, height);

  state->geometry = geometry;

  cairo_pattern_t *output_pattern = cairo_pattern_create_for_surface(surface);
  state->patterns = g_list_append(state->patterns, output_pattern);
  state->original_image_surface = surface;

  return true;
}

static void scale_pattern(gpointer data, gpointer user_data) {
  struct swappy_state *state = (struct swappy_state *)user_data;
  cairo_pattern_t *pattern = (cairo_pattern_t *)data;
  int image_width, image_height;
  int rendered_width, rendered_height;

  image_width = state->geometry->width;
  image_height = state->geometry->height;

  rendered_width = state->drawing_area_rect->width;
  rendered_height = state->drawing_area_rect->height;

  cairo_surface_t *scaled = cairo_surface_create_similar(
      state->rendered_surface, CAIRO_CONTENT_COLOR_ALPHA, rendered_width,
      rendered_height);
  cairo_t *cr = cairo_create(scaled);

  double sx = (double)rendered_width / image_width;
  double sy = (double)rendered_height / image_height;

  cairo_matrix_t matrix;
  cairo_matrix_init_scale(&matrix, 1.0 / sx, 1.0 / sy);
  cairo_pattern_set_matrix(pattern, &matrix);
  cairo_set_source_surface(cr, state->original_image_surface, 0, 0);
  cairo_set_source(cr, pattern);
  cairo_paint(cr);

  cairo_destroy(cr);

  if (state->scaled_image_surface) {
    cairo_surface_destroy(state->scaled_image_surface);
  }

  state->scaled_image_surface = scaled;
}

void buffer_resize_patterns(struct swappy_state *state) {
  g_list_foreach(state->patterns, scale_pattern, state);
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