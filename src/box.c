#include "box.h"
#include "swappy.h"

bool box_parse(struct swappy_box *box, const char *str) {
  char *end = NULL;
  box->x = strtol(str, &end, 10);
  if (end[0] != ',') {
    return false;
  }

  char *next = end + 1;
  box->y = strtol(next, &end, 10);
  if (end[0] != ' ') {
    return false;
  }

  next = end + 1;
  box->width = strtol(next, &end, 10);
  if (end[0] != 'x') {
    return false;
  }

  next = end + 1;
  box->height = strtol(next, &end, 10);
  if (end[0] != '\0') {
    return false;
  }

  return true;
}

bool is_empty_box(struct swappy_box *box) {
  return box->width <= 0 || box->height <= 0;
}

bool intersect_box(struct swappy_box *a, struct swappy_box *b) {
  if (is_empty_box(a) || is_empty_box(b)) {
    return false;
  }

  int x1 = fmax(a->x, b->x);
  int y1 = fmax(a->y, b->y);
  int x2 = fmin(a->x + a->width, b->x + b->width);
  int y2 = fmin(a->y + a->height, b->y + b->height);

  struct swappy_box box = {
      .x = x1,
      .y = y1,
      .width = x2 - x1,
      .height = y2 - y1,
  };
  return !is_empty_box(&box);
}
