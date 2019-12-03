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

enum swappy_brush_point_kind {
  SWAPPY_BRUSH_POINT_FIRST = 0, /* A first point of new brush batch */
  SWAPPY_BRUSH_POINT_WITHIN,    /* A point within a brush batch */
  SWAPPY_BRUSH_POINT_LAST,      /* A point at the end of brush batch */
};

enum swappy_paint_mode_type {
  SWAPPY_PAINT_MODE_BRUSH = 0, /* Brush mode to draw arbitrary shapes */
  SWAPPY_PAINT_MODE_TEXT,      /* Mode to draw texts */
  SWAPPY_PAINT_MODE_RECTANGLE, /* Rectangle shapes */
};

struct swappy_brush_point {
  double x;
  double y;
  double r;
  double g;
  double b;
  double a;
  enum swappy_brush_point_kind kind;
};

struct swappy_box {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

struct swappy_state {
  GtkApplication *app;
  GtkWindow *window;
  GtkWidget *area;
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_shm *shm;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zwlr_screencopy_manager_v1 *zwlr_screencopy_manager;
  struct wl_list outputs;  // mako_output::link
  struct wl_list seats;    // mako_seat::link

  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;

  size_t n_done;

  char *storage_path;

  enum swappy_paint_mode_type mode;

  int width;
  int height;
  char image[255];

  /* Options */
  char *geometry_str;

  struct swappy_box *geometry;

  GSList *brushes;

  int argc;
  char **argv;
};

struct swappy_buffer;

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
