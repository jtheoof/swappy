#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "swappy.h"

static void draw_buffer(cairo_t *cr, struct swappy_state *state) {
  // FIXME This is wrong, the geometry here is not quite valid
  // It must be based on output, but will work fine on single screen
  struct swappy_box *geometry = state->geometry;
  struct swappy_output *output;
  wl_list_for_each(output, &state->outputs, link) {
    struct swappy_buffer *buffer = output->buffer;
    cairo_surface_t *image;
    image = cairo_image_surface_create_for_data(
        buffer->data, CAIRO_FORMAT_ARGB32, geometry->height, geometry->width,
        buffer->stride);
    cairo_save(cr);
    cairo_surface_flush(image);
    cairo_surface_mark_dirty(image);
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
    cairo_paint(cr);
    cairo_surface_destroy(image);
    cairo_restore(cr);
  }
}

//static void draw_image(cairo_t *cr, struct swappy_state *state) {
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

void draw_area(GtkWidget *widget, cairo_t *cr, struct swappy_state *state) {
  guint width, height;
  GtkStyleContext *context;

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
}