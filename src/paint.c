#include "paint.h"

void paint_free(gpointer data) {
  struct swappy_paint *paint = (struct swappy_paint *)data;

  if (paint == NULL) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      g_slist_free_full(paint->content.brush.points, g_free);
      break;
    default:
      g_free(paint);
  }
}

void paint_free_all(struct swappy_state *state) {
  if (state->paints) {
    g_slist_free_full(state->paints, paint_free);
    state->paints = NULL;
  }

  paint_free(state->temp_paint);
  state->temp_paint = NULL;
}

void paint_add_temporary(struct swappy_state *state, double x, double y,
                         enum swappy_paint_type type) {
  struct swappy_paint *paint = g_new(struct swappy_paint, 1);
  struct swappy_point *brush;

  paint->type = type;

  switch (type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      paint->can_draw = true;

      paint->content.brush.r = 1;
      paint->content.brush.g = 0;
      paint->content.brush.b = 0;
      paint->content.brush.a = 1;
      paint->content.brush.w = 2;

      brush = g_new(struct swappy_point, 1);
      brush->x = x;
      brush->y = y;

      paint->content.brush.points = g_slist_append(NULL, brush);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      paint->can_draw = false;  // need `to` vector

      paint->content.shape.from.x = x;
      paint->content.shape.from.y = y;
      paint->content.shape.r = 1;
      paint->content.shape.g = 0;
      paint->content.shape.b = 0;
      paint->content.shape.a = 1;
      paint->content.shape.w = 2;
      paint->content.shape.type = type;
      break;
    default:
      g_info("unable to add temporary paint: %d", type);
      break;
  }

  if (state->temp_paint) {
    g_free(state->temp_paint);
  }

  state->temp_paint = paint;
}

void paint_update_temporary(struct swappy_state *state, double x, double y) {
  struct swappy_paint *paint = state->temp_paint;
  struct swappy_point *brush;
  GSList *points;

  if (!paint) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      points = paint->content.brush.points;
      brush = g_new(struct swappy_point, 1);
      brush->x = x;
      brush->y = y;

      paint->content.brush.points = g_slist_append(points, brush);
      break;
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      paint->content.shape.to.x = x;
      paint->content.shape.to.y = y;
      paint->can_draw = true;  // all set
      break;
    default:
      g_info("unable to update temporary paint: %d", paint->type);
      break;
  }
}

void paint_commit_temporary(struct swappy_state *state) {
  struct swappy_paint *paint = state->temp_paint;

  if (!paint) {
    return;
  }

  if (!paint->can_draw) {
    paint_free(paint);
  } else {
    state->paints = g_slist_append(state->paints, paint);
  }

  // Set the temporary paint to NULL but keep the content in memory
  // because it's now part of the GSList.
  state->temp_paint = NULL;
}
