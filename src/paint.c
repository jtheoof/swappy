#include "paint.h"

void paint_free(gpointer data) {
  struct swappy_paint *paint = (struct swappy_paint *)data;

  if (paint == NULL) {
    return;
  }

  switch (paint->type) {
    case SWAPPY_PAINT_MODE_BRUSH:
      g_list_free_full(paint->content.brush.points, g_free);
      break;
    default:
      g_free(paint);
  }
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

  double r = state->painting.r;
  double g = state->painting.g;
  double b = state->painting.b;
  double a = state->painting.a;
  double w = state->painting.w;

  paint->type = type;

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
    state->paints = g_list_prepend(state->paints, paint);
  }

  // Set the temporary paint to NULL but keep the content in memory
  // because it's now part of the GList.
  state->temp_paint = NULL;
}
