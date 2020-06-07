#include "algebra.h"

#include <glib.h>
#include <math.h>

struct gaussian_kernel *gaussian_kernel(int width, double sigma) {
  double sum = 0;
  gint size = width * width + 1;
  double *kernel = g_new(double, size);
  struct gaussian_kernel *gaussian = g_new(struct gaussian_kernel, 1);
  for (gint y = 0; y < width; y++) {
    for (gint x = 0; x < width; x++) {
      double j = y - width;
      double i = x - width;
      double cell = ((1.0 / (2.0 * G_PI * sigma)) *
                     exp((-(i * i + j * j)) / (2.0 * sigma * sigma))) *
                    0xff;
      kernel[y * width + x] = cell;
      sum += cell;
    }
  }

  gaussian->kernel = kernel;
  gaussian->size = size;
  gaussian->sigma = sigma;
  gaussian->sum = sum;

  return gaussian;
}

void gaussian_kernel_free(gpointer data) {
  struct gaussian_kernel *gaussian = (struct gaussian_kernel *)data;
  if (gaussian != NULL) {
    g_free(gaussian->kernel);
    g_free(gaussian);
  }
}
