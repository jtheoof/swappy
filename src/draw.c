#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "swappy.h"

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

static void draw_buffer(cairo_t *cr, struct swappy_state *state) {
  // FIXME This is wrong, the geometry here is not quite valid
  // It must be based on output, but will work fine on single screen
  struct swappy_box *geometry = state->geometry;
  struct swappy_output *output;
  wl_list_for_each(output, &state->outputs, link) {
    struct swappy_buffer *buffer = output->buffer;
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

    cairo_set_source(cr, output_pattern);
    cairo_pattern_destroy(output_pattern);

    cairo_paint(cr);

    cairo_surface_destroy(output_surface);
  }
}

// static void draw_image(cairo_t *cr, struct swappy_state *state) {
//  cairo_surface_t *image;
//  image = cairo_image_surface_create_from_png(state->image);
//  cairo_save(cr);
//  cairo_surface_flush(image);
//  cairo_surface_mark_dirty(image);
//  cairo_set_source_surface(cr, image, 0, 0);
//  cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
//  cairo_paint(cr);
//  cairo_surface_destroy(image);
//  cairo_restore(cr);
//}

static void draw_brushes(cairo_t *cr, struct swappy_state *state) {
  for (GSList *brush = state->brushes; brush; brush = brush->next) {
    GSList *brush_next = brush->next;
    struct swappy_brush_point *point = brush->data;

    if (brush_next && point->kind == SWAPPY_BRUSH_POINT_WITHIN) {
      struct swappy_brush_point *next = brush_next->data;
      cairo_set_source_rgba(cr, point->r, point->g, point->b, point->a);
      cairo_set_line_width(cr, 2);
      cairo_move_to(cr, point->x, point->y);
      cairo_line_to(cr, next->x, next->y);
      cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
      cairo_stroke(cr);
    } else {
      cairo_set_source_rgba(cr, point->r, point->g, point->b, point->a);
      cairo_set_line_width(cr, 2);
      cairo_rectangle(cr, point->x, point->y, 1, 1);
      cairo_stroke(cr);
    }
  }
}

gboolean draw_area(GtkWidget *widget, cairo_t *cr, struct swappy_state *state) {
  g_debug("received draw callback");
  guint width, height;
  GtkStyleContext *context;

  cairo_set_source_surface(cr, state->cairo_surface, 0, 0);

  context = gtk_widget_get_style_context(widget);

  width = gtk_widget_get_allocated_width(widget);
  height = gtk_widget_get_allocated_height(widget);

  gtk_render_background(context, cr, 0, 0, width, height);

  // Drawing backgroound
  cairo_save(cr);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke_preserve(cr);
  cairo_fill(cr);
  cairo_restore(cr);

  draw_buffer(cr, state);
  //  draw_image(cr, state);
  draw_brushes(cr, state);

  return FALSE;

  // state->cairo_surface = cairo_get_target(cr);
  // cairo_surface_reference(state->cairo_surface);
}

void draw_clear_surface(cairo_surface_t *surface) {
  cairo_t *cr;

  cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);

  cairo_destroy(cr);
}
