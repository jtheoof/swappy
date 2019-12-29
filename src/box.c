#include "box.h"

static int32_t lmax(int32_t a, int32_t b) { return a > b ? a : b; }

static int32_t lmin(int32_t a, int32_t b) { return a < b ? a : b; }

bool box_parse(struct swappy_box *box, const char *str) {
  char *end = NULL;
  box->x = (int32_t)strtol(str, &end, 10);
  if (end[0] != ',') {
    return false;
  }

  char *next = end + 1;
  box->y = (int32_t)strtol(next, &end, 10);
  if (end[0] != ' ') {
    return false;
  }

  next = end + 1;
  box->width = (int32_t)strtol(next, &end, 10);
  if (end[0] != 'x') {
    return false;
  }

  next = end + 1;
  box->height = (int32_t)strtol(next, &end, 10);
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

  int32_t x1 = lmax(a->x, b->x);
  int32_t y1 = lmax(a->y, b->y);
  int32_t x2 = lmin(a->x + a->width, b->x + b->width);
  int32_t y2 = lmin(a->y + a->height, b->y + b->height);

  struct swappy_box box = {
      .x = x1,
      .y = y1,
      .width = x2 - x1,
      .height = y2 - y1,
  };
  return !is_empty_box(&box);
}
