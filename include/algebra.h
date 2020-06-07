#pragma once

#include <glib.h>

struct gaussian_kernel {
  gdouble *kernel;
  gint size;
  gdouble sigma;
  gdouble sum;
};

struct gaussian_kernel *gaussian_kernel(gint width, gdouble sigma);
void gaussian_kernel_free(gpointer data);
