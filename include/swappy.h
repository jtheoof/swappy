#pragma once

#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-client.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

#define MAX_PATH 4096

#define GEOMETRY_PATTERN "xx,yy wwxhh"

enum swappy_paint_type {
  SWAPPY_PAINT_MODE_BRUSH = 0, /* Brush mode to draw arbitrary shapes */
  SWAPPY_PAINT_MODE_TEXT,      /* Mode to draw texts */
  SWAPPY_PAINT_MODE_RECTANGLE, /* Rectangle shapes */
  SWAPPY_PAINT_MODE_ELLIPSE,   /* Ellipse shapes */
  SWAPPY_PAINT_MODE_ARROW,     /* Arrow shapes */
};

struct swappy_point {
  gdouble x;
  gdouble y;
};

struct swappy_paint_shape {
  double r;
  double g;
  double b;
  double a;
  double w;
  struct swappy_point from;
  struct swappy_point to;
  enum swappy_paint_type type;
};

struct swappy_paint_brush {
  double r;
  double g;
  double b;
  double a;
  double w;
  GSList *points;
};

struct swappy_paint {
  enum swappy_paint_type type;
  bool can_draw;
  union {
    struct swappy_paint_brush brush;
    struct swappy_paint_shape shape;
  } content;
};

struct swappy_box {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

struct swappy_state_ui_popover {
  GtkPopover *container;
  GtkRadioButton *brush;
  GtkRadioButton *text;
  GtkRadioButton *rectangle;
  GtkRadioButton *ellipse;
  GtkRadioButton *arrow;
};

struct swappy_state {
  GResource *resource;
  GtkApplication *app;

  GtkWindow *window;
  GtkWindow *settings_window;
  GtkWidget *area;

  struct swappy_state_ui_popover *popover;

  cairo_surface_t *cairo_surface;

  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_shm *shm;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zwlr_screencopy_manager_v1 *zwlr_screencopy_manager;
  struct wl_list outputs;  // mako_output::link
  struct wl_list seats;    // mako_seat::link

  size_t n_done;

  char *storage_path;

  enum swappy_paint_type mode;

  /* Options */
  char *geometry_str;

  struct swappy_box *geometry;

  GSList *paints;
  struct swappy_paint *temp_paint;

  int argc;
  char **argv;
};

struct swappy_buffer {
  struct wl_buffer *wl_buffer;
  void *data;
  int32_t width, height, stride;
  size_t size;
  enum wl_shm_format format;
};

struct swappy_output {
  struct swappy_state *state;
  struct wl_output *wl_output;
  struct zxdg_output_v1 *xdg_output;
  struct wl_list link;

  struct swappy_box geometry;
  enum wl_output_transform transform;
  int32_t scale;

  struct swappy_box logical_geometry;
  double logical_scale;  // guessed from the logical size
  char *name;

  struct swappy_buffer *buffer;
  struct zwlr_screencopy_frame_v1 *screencopy_frame;
  uint32_t screencopy_frame_flags;  // enum zwlr_screencopy_frame_v1_flags
};
