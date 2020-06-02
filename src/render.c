#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <math.h>
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

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])

/*
 * This code was largely taken from Kristian Høgsberg and Chris Wilson from:
 * https://www.cairographics.org/cookbook/blur.c/
 */
static void blur_paint(cairo_t *cr, struct swappy_paint_blur *blur) {
  cairo_surface_t *tmp;
  int width, height;
  int src_stride, dst_stride;
  int x, y, z, w;
  uint8_t *src, *dst;
  uint32_t *s, *d, a, p;
  int i, j, k;
  uint8_t kernel[17];
  const int size = ARRAY_LENGTH(kernel);
  const int half = size / 2;
  double bluriness = blur->bluriness;
  struct swappy_point from = blur->from;
  struct swappy_point to = blur->to;

  cairo_surface_t *surface = cairo_get_target(cr);

  if (cairo_surface_status(surface)) return;

  width = cairo_image_surface_get_width(surface);
  height = cairo_image_surface_get_height(surface);

  switch (cairo_image_surface_get_format(surface)) {
    case CAIRO_FORMAT_A1:
    default:
      /* Don't even think about it! */
      return;

    case CAIRO_FORMAT_A8:
      /* Handle a8 surfaces by effectively unrolling the loops by a
       * factor of 4 - this is safe since we know that stride has to be a
       * multiple of uint32_t. */
      width /= 4;
      break;

    case CAIRO_FORMAT_RGB24:
    case CAIRO_FORMAT_ARGB32:
      break;
  }

  tmp = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  if (cairo_surface_status(tmp)) {
    return;
  }

  src = cairo_image_surface_get_data(surface);
  src_stride = cairo_image_surface_get_stride(surface);

  g_debug("sizeof(src): %lu", sizeof(src));
  g_debug("width*height*stride: %d", width * height * src_stride);

  dst = cairo_image_surface_get_data(tmp);
  dst_stride = cairo_image_surface_get_stride(tmp);

  a = 0;
  for (i = 0; i < size; i++) {
    double f = i - half;
    a += kernel[i] = exp(-f * f / bluriness) * 80;
  }

  int start_x = fmax(fmin(from.x, to.x), 0);
  int start_y = fmax(fmin(from.y, to.y), 0);

  int max_x = fmin(fmax(from.x, to.x), width);
  int max_y = fmin(fmax(from.y, to.y), height);

  for (i = 0; i < height; i++) {
    s = (uint32_t *)(src + i * src_stride);
    d = (uint32_t *)(dst + i * dst_stride);
    for (j = 0; j < width; j++) {
      d[j] = s[j];
    }
  }

  /* Horizontally blur from surface -> tmp */
  for (i = start_y; i < max_y; i++) {
    s = (uint32_t *)(src + i * src_stride);
    d = (uint32_t *)(dst + i * dst_stride);
    for (j = start_x; j < max_x; j++) {
      x = y = z = w = 0;
      for (k = 0; k < size; k++) {
        if (j - half + k < 0 || j - half + k >= width) continue;

        p = s[j - half + k];

        x += ((p >> 24) & 0xff) * kernel[k];
        y += ((p >> 16) & 0xff) * kernel[k];
        z += ((p >> 8) & 0xff) * kernel[k];
        w += ((p >> 0) & 0xff) * kernel[k];
      }
      d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
    }
  }

  /* Then vertically blur from tmp -> surface */
  for (i = start_y; i < max_y; i++) {
    s = (uint32_t *)(dst + i * dst_stride);
    d = (uint32_t *)(src + i * src_stride);
    for (j = start_x; j < max_x; j++) {
      x = y = z = w = 0;
      for (k = 0; k < size; k++) {
        if (i - half + k < 0 || i - half + k >= height) {
          continue;
        }

        s = (uint32_t *)(dst + (i - half + k) * dst_stride);
        p = s[j];

        x += ((p >> 24) & 0xff) * kernel[k];
        y += ((p >> 16) & 0xff) * kernel[k];
        z += ((p >> 8) & 0xff) * kernel[k];
        w += ((p >> 0) & 0xff) * kernel[k];
      }
      d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
    }
  }

  cairo_surface_destroy(tmp);
  cairo_surface_mark_dirty(surface);
}

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
  snprintf(pango_font, 255, "%s %d", text.font, (int)text.s);
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
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.3);
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

  double theta = copysign(1.0, fty) * acos(ftx / ftn);

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

  cairo_save(cr);

  double sx = (double)state->window->width / state->geometry->width;
  double sy = (double)state->window->height / state->geometry->height;

  cairo_scale(cr, sx, sy);

  for (GList *elem = state->patterns; elem; elem = elem->prev) {
    cairo_pattern_t *pattern = elem->data;
    cairo_set_source(cr, pattern);
    cairo_paint(cr);
  }

  cairo_restore(cr);
}

static void render_background(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);
}

static void render_blur(cairo_t *cr, struct swappy_paint_blur blur,
                        bool is_committed) {
  if (!is_committed) {
    // Blur not committed yet, draw bounding rectangle
    struct swappy_paint_shape rect = {
        .r = 0,
        .g = 0.5,
        .b = 1,
        .a = 0.5,
        .w = 5,
        .from = blur.from,
        .to = blur.to,
        .type = SWAPPY_PAINT_MODE_RECTANGLE,
    };
    render_shape_rectangle(cr, rect);
  }
  blur_paint(cr, &blur);
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
    case SWAPPY_PAINT_MODE_BLUR:
      render_blur(cr, paint->content.blur, paint->is_committed);
      break;
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
      g_info("unable to render paint with type: %d", paint->type);
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
