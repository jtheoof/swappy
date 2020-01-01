#include "paint.h"

#include "util.h"

void paint_free(gpointer data) {
  struct swappy_paint *paint = (struct swappy_paint *)data;

  if (paint == NULL) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      g_list_free_full(paint->content.brush.points, g_free);
      break;
    case SWAPPY_PAINT_MODE_TEXT:
      g_free(paint->content.text.text);
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
  struct swappy_point *brush;

  double r = state->settings.r;
  double g = state->settings.g;
  double b = state->settings.b;
  double a = state->settings.a;
  double w = state->settings.w;
  double t = state->settings.t;

  paint->type = type;

  if (state->temp_paint) {
    if (type == SWAPPY_PAINT_MODE_TEXT) {
      paint_commit_temporary(state);
    } else {
      g_free(state->temp_paint);
      state->temp_paint = NULL;
    }
  }

  switch (type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      paint->can_draw = true;

      paint->content.brush.r = r;
      paint->content.brush.g = g;
      paint->content.brush.b = b;
      paint->content.brush.a = a;
      paint->content.brush.w = w;

      brush = g_new(struct swappy_point, 1);
      brush->x = x;
      brush->y = y;

      paint->content.brush.points = g_list_prepend(NULL, brush);
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
                                  double y) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_point *brush;
  GList *points;

  if (!paint) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      points = paint->content.brush.points;
      brush = g_new(struct swappy_point, 1);
      brush->x = x;
      brush->y = y;

      paint->content.brush.points = g_list_prepend(points, brush);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
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

void paint_update_temporary_text(struct swappy_state *state,
                                 GdkEventKey *event) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_paint_text *text;
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
      if (strlen(text->text) > 0) {
        string_remove_at(text->text, text->cursor - 1);
        text->cursor--;
      }
      break;
    case GDK_KEY_Delete:
      if (strlen(text->text) > 0) {
        string_remove_at(text->text, text->cursor);
      }
      break;
    case GDK_KEY_Left:
      text->cursor--;
      break;
    case GDK_KEY_Right:
      text->cursor++;
      break;
    default:
      unicode = gdk_keyval_to_unicode(event->keyval);
      if (unicode != 0) {
        int ll = g_unichar_to_utf8(unicode, buffer);
        buffer[ll] = '\0';
        g_debug("received unicode: %d - utf8: %s (%d)", unicode, buffer, ll);
        g_debug("text before: %s - cursor: %d", text->text, text->cursor);
        char *new_text =
            string_insert_char_at(text->text, buffer[0], text->cursor);
        g_free(text->text);
        text->text = new_text;
        text->cursor++;
      }
      break;
  }

  if (text->cursor < 0) {
    text->cursor = 0;
  } else if (text->cursor > (int)strlen(text->text)) {
    text->cursor = (int)strlen(text->text);
  }

  g_debug("text is now: %s", text->text);
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
}

void paint_commit_temporary(struct swappy_state *state) {
  struct swappy_paint *paint = state->temp_paint;

  if (!paint) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_TEXT:
      if (strlen(paint->content.text.text) == 0) {
        paint->can_draw = false;
      }
      paint->content.text.mode = SWAPPY_TEXT_MODE_DONE;
      break;
    default:
      g_info("unable to update temporary text when type is: %d", paint->type);
      break;
  }

  if (!paint->can_draw) {
    paint_free(paint);
  } else {
    state->paints = g_list_prepend(state->paints, paint);
  }

  // Set the temporary paint to NULL but keep the content in memory
  // because it's now part of the GList.
  state->temp_paint = NULL;
}
