#include <glib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <pango/pangocairo.h>

#include "algebra.h"
#include "swappy.h"

#define pango_layout_t PangoLayout
#define pango_font_description_t PangoFontDescription
#define pango_rectangle_t PangoRectangle

/*
 * This code was largely taken from Kristian Høgsberg and Chris Wilson from:
 * https://www.cairographics.org/cookbook/blur.c/
 */
static cairo_surface_t *blur_surface(cairo_surface_t *surface, double x,
                                     double y, double width, double height) {
  cairo_surface_t *dest_surface, *tmp_surface;
  cairo_t *cr;
  int src_width, src_height;
  int src_stride, dst_stride;
  guint u, v, w, z;
  uint8_t *dst, *tmp;
  uint32_t *s, *d, p;
  int i, j, k;
  const int radius = 4;
  const double sigma = 3.1;
  struct gaussian_kernel *gaussian = gaussian_kernel(radius, sigma);
  const int size = gaussian->size;
  const int half = (int)radius * 2;
  gdouble scale_x, scale_y;
  guint sum, pass, nb_passes;

  sum = (guint)gaussian->sum;

  if (cairo_surface_status(surface)) {
    return NULL;
  }

  cairo_surface_get_device_scale(surface, &scale_x, &scale_y);

  cairo_format_t src_format = cairo_image_surface_get_format(surface);
  switch (src_format) {
    case CAIRO_FORMAT_A1:
    case CAIRO_FORMAT_A8:
    case CAIRO_FORMAT_RGB24:
    default:
      g_warning("source surface format: %d is not supported", src_format);
      return NULL;
    case CAIRO_FORMAT_ARGB32:
      break;
  }

  src_stride = cairo_image_surface_get_stride(surface);
  src_width = cairo_image_surface_get_width(surface);
  src_height = cairo_image_surface_get_height(surface);

  g_assert(src_height >= height);
  g_assert(src_width >= width);

  dest_surface = cairo_image_surface_create(src_format, src_width, src_height);
  tmp_surface = cairo_image_surface_create(src_format, src_width, src_height);

  cairo_surface_set_device_scale(dest_surface, scale_x, scale_y);
  cairo_surface_set_device_scale(tmp_surface, scale_x, scale_y);

  if (cairo_surface_status(dest_surface) || cairo_surface_status(tmp_surface)) {
    return NULL;
  }

  cr = cairo_create(tmp_surface);
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  cr = cairo_create(dest_surface);
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  dst = cairo_image_surface_get_data(dest_surface);
  tmp = cairo_image_surface_get_data(tmp_surface);
  dst_stride = cairo_image_surface_get_stride(dest_surface);

  nb_passes = (guint)sqrt(scale_x * scale_y) + 1;

  int start_x = CLAMP(x * scale_x, 0, src_width);
  int start_y = CLAMP(y * scale_y, 0, src_height);

  int end_x = CLAMP((x + width) * scale_x, 0, src_width);
  int end_y = CLAMP((y + height) * scale_y, 0, src_height);

  for (pass = 0; pass < nb_passes; pass++) {
    /* Horizontally blur from surface -> tmp */
    for (i = start_y; i < end_y; i++) {
      s = (uint32_t *)(dst + i * src_stride);
      d = (uint32_t *)(tmp + i * dst_stride);
      for (j = start_x; j < end_x; j++) {
        u = v = w = z = 0;
        for (k = 0; k < size; k++) {
          gdouble multiplier = gaussian->kernel[k];

          if (j - half + k < 0 || j - half + k >= src_width) {
            continue;
          }

          p = s[j - half + k];

          u += ((p >> 24) & 0xff) * multiplier;
          v += ((p >> 16) & 0xff) * multiplier;
          w += ((p >> 8) & 0xff) * multiplier;
          z += ((p >> 0) & 0xff) * multiplier;
        }

        d[j] = (u / sum << 24) | (v / sum << 16) | (w / sum << 8) | z / sum;
      }
    }

    /* Then vertically blur from tmp -> surface */
    for (i = start_y; i < end_y; i++) {
      d = (uint32_t *)(dst + i * dst_stride);
      for (j = start_x; j < end_x; j++) {
        u = v = w = z = 0;
        for (k = 0; k < size; k++) {
          gdouble multiplier = gaussian->kernel[k];

          if (i - half + k < 0 || i - half + k >= src_height) {
            continue;
          }

          s = (uint32_t *)(tmp + (i - half + k) * dst_stride);
          p = s[j];

          u += ((p >> 24) & 0xff) * multiplier;
          v += ((p >> 16) & 0xff) * multiplier;
          w += ((p >> 8) & 0xff) * multiplier;
          z += ((p >> 0) & 0xff) * multiplier;
        }

        d[j] = (u / sum << 24) | (v / sum << 16) | (w / sum << 8) | z / sum;
      }
    }
  }

  // Mark destination surface as dirty since it was altered with custom data.
  cairo_surface_mark_dirty(dest_surface);
  cairo_surface_t *final = cairo_image_surface_create(
      src_format, (int)(width * scale_x), (int)(height * scale_y));
  cairo_surface_set_device_scale(final, scale_x, scale_y);
  cr = cairo_create(final);
  cairo_set_source_surface(cr, dest_surface, -x, -y);
  cairo_paint(cr);
  cairo_destroy(cr);
  cairo_surface_destroy(dest_surface);
  cairo_surface_destroy(tmp_surface);
  gaussian_kernel_free(gaussian);
  return final;
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

  double alpha = G_PI / 6;
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
  cairo_arc(cr, 0, 0, r, 0, 2 * G_PI);
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

  //  g_debug("scaling cairo context: (%.2lf, %.2lf)", sx, sy);

  cairo_scale(cr, sx, sy);

  for (GList *elem = state->patterns; elem; elem = elem->prev) {
    cairo_pattern_t *pattern = elem->data;
    cairo_set_source(cr, pattern);
    cairo_paint(cr);
  }

  cairo_restore(cr);
}

static void render_background(cairo_t *cr, struct swappy_state *state) {
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);
}

static void render_blur(cairo_t *cr, struct swappy_paint *paint) {
  struct swappy_paint_blur blur = paint->content.blur;

  cairo_surface_t *target = cairo_get_target(cr);

  double a, b;
  cairo_surface_get_device_scale(target, &a, &b);

  double x = MIN(blur.from.x, blur.to.x);
  double y = MIN(blur.from.y, blur.to.y);
  double w = ABS(blur.from.x - blur.to.x);
  double h = ABS(blur.from.y - blur.to.y);

  cairo_save(cr);

  if (!paint->is_committed) {
    cairo_surface_t *blurred = blur_surface(target, x, y, w, h);

    if (blurred && cairo_surface_status(blurred) == CAIRO_STATUS_SUCCESS) {
      cairo_set_source_surface(cr, blurred, x, y);
      cairo_paint(cr);
      if (blur.surface) {
        cairo_surface_destroy(blur.surface);
      }
      paint->content.blur.surface = blurred;
    }

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

  } else {
    cairo_surface_t *surface = blur.surface;
    if (surface && cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
      cairo_set_source_surface(cr, surface, x, y);
      cairo_paint(cr);
    }
  }

  cairo_restore(cr);
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
      render_blur(cr, paint);
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
  cairo_surface_t *surface = state->cairo_surface;
  cairo_t *cr = cairo_create(surface);

  render_background(cr, state);
  render_buffers(cr, state);
  render_paints(cr, state);

  cairo_destroy(cr);

  // Drawing is finished, notify the GtkDrawingArea it needs to be redrawn.
  gtk_widget_queue_draw(state->ui->area);
}
