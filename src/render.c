#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include "swappy.h"
#include "util.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define RENDER_PANGO_FONT SWAPPY_TEXT_FONT_DEFAULT SWAPPY_TEXT_SIZE_DEFAULT

#define pango_layout_t PangoLayout
#define pango_font_description_t PangoFontDescription
#define pango_rectangle_t PangoRectangle

static void convert_pango_rectangle_to_swappy_box(pango_rectangle_t rectangle,
                                                  struct swappy_box *box) {
  if (!box) {
    return;
  }

  box->x = pango_units_to_double(rectangle.x);
  box->y = pango_units_to_double(rectangle.y);
  box->width = pango_units_to_double(rectangle.width);
  box->height = pango_units_to_double(rectangle.height);
}

static void render_text(cairo_t *cr, struct swappy_paint_text text) {
  char pango_font[255];
  double x = fmin(text.from.x, text.to.x);
  double y = fmin(text.from.y, text.to.y);
  double w = fabs(text.from.x - text.to.x);
  double h = fabs(text.from.y - text.to.y);

  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cairo_t *crt = cairo_create(surface);

  pango_layout_t *layout = pango_cairo_create_layout(crt);
  pango_layout_set_text(layout, text.text, -1);
  snprintf(pango_font, 255, "%s %d", SWAPPY_TEXT_FONT_DEFAULT, (int)text.s);
  pango_font_description_t *desc =
      pango_font_description_from_string(pango_font);
  pango_layout_set_width(layout, pango_units_from_double(w));
  pango_layout_set_font_description(layout, desc);
  pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
  pango_font_description_free(desc);

  if (text.mode == SWAPPY_TEXT_MODE_EDIT) {
    pango_rectangle_t strong_pos;
    pango_rectangle_t weak_pos;
    struct swappy_box cursor_box;
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 5);
    cairo_rectangle(cr, x, y, w, h);
    cairo_stroke(cr);
    pango_layout_get_cursor_pos(layout, text.cursor, &strong_pos, &weak_pos);
    convert_pango_rectangle_to_swappy_box(strong_pos, &cursor_box);
    cairo_move_to(crt, cursor_box.x, cursor_box.y);
    cairo_set_source_rgba(crt, 0.3, 0.3, 0.3, 1);
    cairo_line_to(crt, cursor_box.x, cursor_box.y + cursor_box.height);
    cairo_stroke(crt);
  }

  cairo_rectangle(crt, 0, 0, w, h);
  cairo_set_source_rgba(crt, text.r, text.g, text.b, text.a);
  cairo_move_to(crt, 0, 0);
  pango_cairo_show_layout(crt, layout);

  cairo_set_source_surface(cr, surface, x, y);
  cairo_paint(cr);

  cairo_destroy(crt);
  cairo_surface_destroy(surface);
  g_object_unref(layout);
}

static void render_shape_arrow(cairo_t *cr, struct swappy_paint_shape shape) {
  cairo_set_source_rgba(cr, shape.r, shape.g, shape.b, shape.a);
  cairo_set_line_width(cr, shape.w);

  double ftx = shape.to.x - shape.from.x;
  double fty = shape.to.y - shape.from.y;
  double ftn = sqrt(ftx * ftx + fty * fty);

  double r = 20;
  double scaling_factor = shape.w / 4;

  double alpha = M_PI / 6;
  double ta = 5 * alpha;
  double tb = 7 * alpha;
  double xa = r * cos(ta);
  double ya = r * sin(ta);
  double xb = r * cos(tb);
  double yb = r * sin(tb);
  double xc = ftn - fabs(xa) * scaling_factor;

  if (xc < DBL_EPSILON) {
    xc = 0;
  }

  if (ftn < DBL_EPSILON) {
    return;
  }

  double theta = atan(fty / ftx);

  if (ftx < DBL_EPSILON) {
    theta = M_PI + theta;
  }

  // Draw line
  cairo_save(cr);
  cairo_translate(cr, shape.from.x, shape.from.y);
  cairo_rotate(cr, theta);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, xc, 0);
  cairo_stroke(cr);
  cairo_restore(cr);

  // Draw arrow
  cairo_save(cr);
  cairo_translate(cr, shape.to.x, shape.to.y);
  cairo_rotate(cr, theta);
  cairo_scale(cr, scaling_factor, scaling_factor);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, xa, ya);
  cairo_line_to(cr, xb, yb);
  cairo_line_to(cr, 0, 0);
  cairo_fill(cr);
  cairo_restore(cr);
}

static void render_shape_ellipse(cairo_t *cr, struct swappy_paint_shape shape) {
  double x = fabs(shape.from.x - shape.to.x);
  double y = fabs(shape.from.y - shape.to.y);
  double xc = shape.from.x + ((shape.to.x - shape.from.x) / 2);
  double yc = shape.from.y + ((shape.to.y - shape.from.y) / 2);

  double n = sqrt(x * x + y * y);
  double r = n / 2;

  cairo_set_source_rgba(cr, shape.r, shape.g, shape.b, shape.a);
  cairo_set_line_width(cr, shape.w);

  cairo_matrix_t save_matrix;
  cairo_get_matrix(cr, &save_matrix);
  cairo_translate(cr, xc, yc);
  cairo_scale(cr, x / n, y / n);
  cairo_arc(cr, 0, 0, r, 0, 2 * M_PI);
  cairo_set_matrix(cr, &save_matrix);
  cairo_stroke(cr);
  cairo_close_path(cr);
}

static void render_shape_rectangle(cairo_t *cr,
                                   struct swappy_paint_shape shape) {
  double x = fmin(shape.from.x, shape.to.x);
  double y = fmin(shape.from.y, shape.to.y);
  double w = fabs(shape.from.x - shape.to.x);
  double h = fabs(shape.from.y - shape.to.y);

  cairo_set_source_rgba(cr, shape.r, shape.g, shape.b, shape.a);
  cairo_set_line_width(cr, shape.w);

  cairo_rectangle(cr, x, y, w, h);
  cairo_close_path(cr);
  cairo_stroke(cr);
}

static void render_shape(cairo_t *cr, struct swappy_paint_shape shape) {
  cairo_save(cr);
  switch (shape.type) {
    case SWAPPY_PAINT_MODE_RECTANGLE:
      render_shape_rectangle(cr, shape);
      break;
    case SWAPPY_PAINT_MODE_ELLIPSE:
      render_shape_ellipse(cr, shape);
      break;
    case SWAPPY_PAINT_MODE_ARROW:
      render_shape_arrow(cr, shape);
      break;
    default:
      break;
  }
  cairo_restore(cr);
}

static void render_buffers(cairo_t *cr, struct swappy_state *state) {
  if (!state->patterns) {
    return;
  }

  cairo_paint(cr);

  for (GList *elem = state->patterns; elem; elem = elem->prev) {
    cairo_pattern_t *pattern = elem->data;
    cairo_set_source(cr, pattern);
    cairo_paint(cr);
  }
}

static void render_background(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);
}

static void render_brush(cairo_t *cr, struct swappy_paint_brush brush) {
  cairo_set_source_rgba(cr, brush.r, brush.g, brush.b, brush.a);
  cairo_set_line_width(cr, brush.w);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

  guint l = g_list_length(brush.points);

  if (l == 1) {
    struct swappy_point *point = g_list_nth_data(brush.points, 0);
    cairo_rectangle(cr, point->x, point->y, brush.w, brush.w);
    cairo_fill(cr);
  } else {
    for (GList *elem = brush.points; elem; elem = elem->next) {
      struct swappy_point *point = elem->data;
      cairo_line_to(cr, point->x, point->y);
    }
    cairo_stroke(cr);
  }
}

static void render_paint(cairo_t *cr, struct swappy_paint *paint) {
  if (!paint->can_draw) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      render_brush(cr, paint->content.brush);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      render_shape(cr, paint->content.shape);
      break;
    case SWAPPY_PAINT_MODE_TEXT:
      render_text(cr, paint->content.text);
      break;
    default:
      g_info("unable to draw paint with type: %d", paint->type);
      break;
  }
}

static void render_paints(cairo_t *cr, struct swappy_state *state) {
  for (GList *elem = g_list_last(state->paints); elem; elem = elem->prev) {
    struct swappy_paint *paint = elem->data;
    render_paint(cr, paint);
  }

  if (state->temp_paint) {
    render_paint(cr, state->temp_paint);
  }
}

void render_state(struct swappy_state *state) {
  cairo_t *cr = cairo_create(state->cairo_surface);

  render_background(cr);
  render_buffers(cr, state);
  render_paints(cr, state);

  // Drawing is finished, notify the GtkDrawingArea it needs to be redrawn.
  gtk_widget_queue_draw(state->ui->area);

  cairo_destroy(cr);
}
