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
