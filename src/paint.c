#include "paint.h"

#include <glib.h>
#include <stdio.h>

#include "gtk/gtk.h"
#include "util.h"

static void cursor_move_backward(struct swappy_paint_text *text) {
  if (text->cursor > 0) {
    text->cursor--;
  }
}

static void cursor_move_forward(struct swappy_paint_text *text) {
  if (text->cursor < g_utf8_strlen(text->text, -1)) {
    text->cursor++;
  }
}

void paint_free(gpointer data) {
  struct swappy_paint *paint = (struct swappy_paint *)data;

  if (paint == NULL) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BLUR:
      if (paint->content.blur.surface) {
        cairo_surface_destroy(paint->content.blur.surface);
      }
      break;
    case SWAPPY_PAINT_MODE_BRUSH:
      g_list_free_full(paint->content.brush.points, g_free);
      break;
    case SWAPPY_PAINT_MODE_TEXT:
      g_free(paint->content.text.text);
      g_free(paint->content.text.font);
      break;
    default:
      break;
  }
  g_free(paint);
}

void paint_free_list(GList **list) {
  if (*list) {
    g_list_free_full(*list, paint_free);
    *list = NULL;
  }
}

void paint_free_all(struct swappy_state *state) {
  paint_free_list(&state->paints);
  paint_free_list(&state->redo_paints);
  paint_free(state->temp_paint);
  state->temp_paint = NULL;
}

void paint_add_temporary(struct swappy_state *state, double x, double y,
                         enum swappy_paint_type type) {
  struct swappy_paint *paint = g_new(struct swappy_paint, 1);
  struct swappy_point *point;

  double r = state->settings.r;
  double g = state->settings.g;
  double b = state->settings.b;
  double a = state->settings.a;
  double w = state->settings.w;
  double t = state->settings.t;

  if (state->config->transparent) a *= (1 - state->settings.tr / 100.0);

  paint->type = type;
  paint->is_committed = false;

  g_debug("adding temporary paint at: %.2lfx%.2lf", x, y);

  if (state->temp_paint) {
    if (type == SWAPPY_PAINT_MODE_TEXT) {
      paint_commit_temporary(state);
    } else {
      paint_free(state->temp_paint);
      state->temp_paint = NULL;
    }
  }

  switch (type) {
    case SWAPPY_PAINT_MODE_BLUR:
      paint->can_draw = false;

      paint->content.blur.from.x = x;
      paint->content.blur.from.y = y;
      paint->content.blur.surface = NULL;
      break;
    case SWAPPY_PAINT_MODE_BRUSH:
      paint->can_draw = true;

      paint->content.brush.r = r;
      paint->content.brush.g = g;
      paint->content.brush.b = b;
      paint->content.brush.a = a;
      paint->content.brush.w = w;

      point = g_new(struct swappy_point, 1);
      point->x = x;
      point->y = y;

      paint->content.brush.points = g_list_prepend(NULL, point);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      paint->can_draw = false;  // need `to` vector

      paint->content.shape.from.x = x;
      paint->content.shape.from.y = y;
      paint->content.shape.r = r;
      paint->content.shape.g = g;
      paint->content.shape.b = b;
      paint->content.shape.a = a;
      paint->content.shape.w = w;
      paint->content.shape.type = type;
      if (state->config->fill_shape)
        paint->content.shape.operation = SWAPPY_PAINT_SHAPE_OPERATION_FILL;
      else
        paint->content.shape.operation = SWAPPY_PAINT_SHAPE_OPERATION_STROKE;
      break;
    case SWAPPY_PAINT_MODE_TEXT:
      paint->can_draw = false;

      paint->content.text.from.x = x;
      paint->content.text.from.y = y;
      paint->content.text.r = r;
      paint->content.text.g = g;
      paint->content.text.b = b;
      paint->content.text.a = a;
      paint->content.text.s = t;
      paint->content.text.font = g_strdup(state->config->text_font);
      paint->content.text.cursor = 0;
      paint->content.text.mode = SWAPPY_TEXT_MODE_EDIT;
      paint->content.text.text = g_new(gchar, 1);
      paint->content.text.text[0] = '\0';
      break;

    default:
      g_info("unable to add temporary paint: %d", type);
      break;
  }

  state->temp_paint = paint;
}

void paint_update_temporary_shape(struct swappy_state *state, double x,
                                  double y, gboolean is_control_pressed) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_point *point;
  GList *points;

  if (!paint) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BLUR:
      paint->can_draw = true;
      paint->content.blur.to.x = x;
      paint->content.blur.to.y = y;
      break;
    case SWAPPY_PAINT_MODE_BRUSH:
      points = paint->content.brush.points;
      point = g_new(struct swappy_point, 1);
      point->x = x;
      point->y = y;

      paint->content.brush.points = g_list_prepend(points, point);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
      paint->can_draw = true;  // all set

      paint->content.shape.should_center_at_from = is_control_pressed;
      paint->content.shape.to.x = x;
      paint->content.shape.to.y = y;
      break;
    case SWAPPY_PAINT_MODE_ARROW:
      paint->can_draw = true;  // all set

      paint->content.shape.to.x = x;
      paint->content.shape.to.y = y;
      break;
    default:
      g_info("unable to update temporary paint when type is: %d", paint->type);
      break;
  }
}

void paint_update_temporary_str(struct swappy_state *state, char *str) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_paint_text *text;
  char *new_text;
  if (!paint || paint->type != SWAPPY_PAINT_MODE_TEXT) {
    g_warning("trying to update text but not in text mode");
    return;
  }

  text = &paint->content.text;
  new_text = string_insert_chars_at(text->text, str, text->cursor);
  g_free(text->text);
  text->text = new_text;
  text->cursor += g_utf8_strlen(str, -1);
}

void paint_update_temporary_text(struct swappy_state *state,
                                 GdkEventKey *event) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_paint_text *text;
  char *new_text;
  char buffer[32];
  guint32 unicode;

  if (!paint || paint->type != SWAPPY_PAINT_MODE_TEXT) {
    g_warning("trying to update text but not in text mode");
    return;
  }

  text = &paint->content.text;

  switch (event->keyval) {
    case GDK_KEY_Escape:
      paint_commit_temporary(state);
      break;
    case GDK_KEY_BackSpace:
      if (g_utf8_strlen(text->text, -1) > 0) {
        new_text = string_remove_at(text->text, text->cursor - 1);
        g_free(text->text);
        text->text = new_text;
        cursor_move_backward(text);
      }
      break;
    case GDK_KEY_Delete:
      if (g_utf8_strlen(text->text, -1) > 0) {
        new_text = string_remove_at(text->text, text->cursor);
        g_free(text->text);
        text->text = new_text;
      }
      break;
    case GDK_KEY_Left:
      cursor_move_backward(text);
      break;
    case GDK_KEY_Right:
      cursor_move_forward(text);
      break;
    case GDK_KEY_V:
      cursor_move_forward(text);
      break;
    default:
      unicode = gdk_keyval_to_unicode(event->keyval);
      if (unicode != 0) {
        int ll = g_unichar_to_utf8(unicode, buffer);
        buffer[ll] = '\0';
        char *new_text =
            string_insert_chars_at(text->text, buffer, text->cursor);
        g_free(text->text);
        text->text = new_text;
        text->cursor++;
      }
      break;
  }
}

void paint_update_temporary_text_clip(struct swappy_state *state, gdouble x,
                                      gdouble y) {
  struct swappy_paint *paint = state->temp_paint;

  if (!paint) {
    return;
  }

  g_assert(paint->type == SWAPPY_PAINT_MODE_TEXT);

  paint->can_draw = true;
  paint->content.text.to.x = x;
  paint->content.text.to.y = y;
  gtk_im_context_focus_in(state->ui->im_context);
}

void paint_commit_temporary(struct swappy_state *state) {
  struct swappy_paint *paint = state->temp_paint;

  if (!paint) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_TEXT:
      if (g_utf8_strlen(paint->content.text.text, -1) == 0) {
        paint->can_draw = false;
      }
      paint->content.text.mode = SWAPPY_TEXT_MODE_DONE;
      break;
    default:
      break;
  }

  if (!paint->can_draw) {
    paint_free(paint);
  } else {
    paint->is_committed = true;
    state->paints = g_list_prepend(state->paints, paint);
  }

  gtk_im_context_focus_out(state->ui->im_context);
  // Set the temporary paint to NULL but keep the content in memory
  // because it's now part of the GList.
  state->temp_paint = NULL;
}

void paint_get_crop_resize(enum swappy_resize *out_resize_x,
                           enum swappy_resize *out_resize_y,
                           const struct swappy_state *state, double x,
                           double y) {
  const struct swappy_crop *crop = &state->crop;
  const double part_size = 30 / state->scaling_factor;

  if (x < crop->left_x - part_size || x > crop->right_x + part_size ||
      y < crop->top_y - part_size || y > crop->bottom_y + part_size) {
    *out_resize_x = SWAPPY_RESIZE_NONE;
    *out_resize_y = SWAPPY_RESIZE_NONE;
    return;
  }

  if (x < crop->left_x + part_size)
    *out_resize_x = SWAPPY_RESIZE_LOW;
  else if (x > crop->right_x - part_size)
    *out_resize_x = SWAPPY_RESIZE_HIGH;
  else if (x >= crop->left_x && x <= crop->right_x)
    *out_resize_x = SWAPPY_RESIZE_BOTH;
  else
    *out_resize_x = SWAPPY_RESIZE_NONE;

  if (y < crop->top_y + part_size)
    *out_resize_y = SWAPPY_RESIZE_LOW;
  else if (y > crop->bottom_y - part_size)
    *out_resize_y = SWAPPY_RESIZE_HIGH;
  else if (y >= crop->top_y && y <= crop->bottom_y)
    *out_resize_y = SWAPPY_RESIZE_BOTH;
  else
    *out_resize_y = SWAPPY_RESIZE_NONE;

  if (*out_resize_x == SWAPPY_RESIZE_BOTH &&
      *out_resize_y != SWAPPY_RESIZE_BOTH)
    *out_resize_x = SWAPPY_RESIZE_NONE;
  if (*out_resize_y == SWAPPY_RESIZE_BOTH &&
      *out_resize_x != SWAPPY_RESIZE_BOTH)
    *out_resize_y = SWAPPY_RESIZE_NONE;
}

void paint_start_crop(struct swappy_state *state, double x, double y,
                      gboolean recreate_requested) {
  if (!recreate_requested) {
    paint_get_crop_resize(&state->crop.resize_x, &state->crop.resize_y, state,
                          x, y);

    if (state->crop.resize_x || state->crop.resize_y) return;
  }

  state->crop.left_x = x;
  state->crop.right_x = x;
  state->crop.top_y = y;
  state->crop.bottom_y = y;

  state->crop.resize_x = SWAPPY_RESIZE_HIGH;
  state->crop.resize_y = SWAPPY_RESIZE_HIGH;
}

static inline bool u32_add_clamped(uint32_t *val, double to_add, uint32_t max) {
  if (*val + to_add > max) {
    *val = max;
    return true;
  } else if (to_add < 0 && *val < -to_add) {
    *val = 0;
    return true;
  } else {
    *val += to_add;
    return false;
  }
}

void paint_update_crop(struct swappy_state *state, double delta_x,
                       double delta_y) {
  struct swappy_crop *crop = &state->crop;
  double iw = cairo_image_surface_get_width(state->rendering_surface);
  double ih = cairo_image_surface_get_height(state->rendering_surface);

  switch (crop->resize_x) {
    case SWAPPY_RESIZE_NONE:
      break;
    case SWAPPY_RESIZE_LOW:
      u32_add_clamped(&crop->left_x, delta_x, iw);
      break;
    case SWAPPY_RESIZE_HIGH:
      u32_add_clamped(&crop->right_x, delta_x, iw);
      break;
    case SWAPPY_RESIZE_BOTH: {
      uint32_t width = crop->right_x - crop->left_x;
      if (u32_add_clamped(&crop->left_x, delta_x, iw)) {
        crop->right_x = crop->left_x + width;
      } else if (u32_add_clamped(&crop->right_x, delta_x, iw)) {
        crop->left_x = crop->right_x - width;
      }
      break;
    }
  }
  switch (crop->resize_y) {
    case SWAPPY_RESIZE_NONE:
      break;
    case SWAPPY_RESIZE_LOW:
      u32_add_clamped(&crop->top_y, delta_y, ih);
      break;
    case SWAPPY_RESIZE_HIGH:
      u32_add_clamped(&crop->bottom_y, delta_y, ih);
      break;
    case SWAPPY_RESIZE_BOTH: {
      uint32_t height = crop->bottom_y - crop->top_y;
      if (u32_add_clamped(&crop->top_y, delta_y, ih)) {
        crop->bottom_y = crop->top_y + height;
      } else if (u32_add_clamped(&crop->bottom_y, delta_y, ih)) {
        crop->top_y = crop->bottom_y - height;
      }
      break;
    }
  }

  uint32_t k;
  if (crop->left_x > crop->right_x) {
    k = crop->left_x;
    crop->left_x = crop->right_x;
    crop->right_x = k;
    crop->resize_x = -crop->resize_x;
  }
  if (crop->top_y > crop->bottom_y) {
    k = crop->top_y;
    crop->top_y = crop->bottom_y;
    crop->bottom_y = k;
    crop->resize_y = -crop->resize_y;
  }
}
