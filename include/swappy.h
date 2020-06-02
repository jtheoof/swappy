#pragma once

#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-client.h>
#ifdef HAVE_WAYLAND_PROTOCOLS
#include <xdg-output-unstable-v1-client-protocol.h>
#endif

#include "wlr-screencopy-unstable-v1-client-protocol.h"

#define MAX_PATH 4096

#define GEOMETRY_PATTERN "xx,yy wwxhh"

#define SWAPPY_LINE_SIZE_MIN 1
#define SWAPPY_LINE_SIZE_MAX 50

#define SWAPPY_BLUR_LEVEL_MIN 1
#define SWAPPY_BLUR_LEVEL_MAX 500

#define SWAPPY_TEXT_SIZE_MIN 10
#define SWAPPY_TEXT_SIZE_MAX 50

enum swappy_paint_type {
  SWAPPY_PAINT_MODE_BRUSH = 0, /* Brush mode to draw arbitrary shapes */
  SWAPPY_PAINT_MODE_TEXT,      /* Mode to draw texts */
  SWAPPY_PAINT_MODE_RECTANGLE, /* Rectangle shapes */
  SWAPPY_PAINT_MODE_ELLIPSE,   /* Ellipse shapes */
  SWAPPY_PAINT_MODE_ARROW,     /* Arrow shapes */
  SWAPPY_PAINT_MODE_BLUR,      /* Blur mode */
};

enum swappy_text_mode {
  SWAPPY_TEXT_MODE_EDIT = 0,
  SWAPPY_TEXT_MODE_DONE,
};

struct swappy_point {
  gdouble x;
  gdouble y;
};

struct swappy_paint_text {
  double r;
  double g;
  double b;
  double a;
  double s;
  gchar *font;
  gchar *text;
  size_t cursor;
  struct swappy_point from;
  struct swappy_point to;
  enum swappy_text_mode mode;
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
  GList *points;
};

struct swappy_paint_blur {
  double bluriness;
  struct swappy_point from;
  struct swappy_point to;
};

struct swappy_paint {
  enum swappy_paint_type type;
  bool can_draw;
  bool is_committed;
  union {
    struct swappy_paint_brush brush;
    struct swappy_paint_shape shape;
    struct swappy_paint_text text;
    struct swappy_paint_blur blur;
  } content;
};

struct swappy_box {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

struct swappy_state_settings {
  double r;
  double g;
  double b;
  double a;
  double w;
  double t;
  guint32 blur_level;
};

struct swappy_state_ui {
  GtkWindow *window;
  GtkWidget *area;

  // Undo / Redo
  GtkButton *undo;
  GtkButton *redo;

  // Painting Area
  GtkBox *painting_box;
  GtkRadioButton *brush;
  GtkRadioButton *text;
  GtkRadioButton *rectangle;
  GtkRadioButton *ellipse;
  GtkRadioButton *arrow;
  GtkRadioButton *blur;

  GtkRadioButton *red;
  GtkRadioButton *green;
  GtkRadioButton *blue;
  GtkRadioButton *custom;
  GtkColorButton *color;

  GtkButton *blur_level;
  GtkButton *line_size;
  GtkButton *text_size;
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
  struct swappy_box geometry;
  struct swappy_box logical_geometry;
  struct wl_output *wl_output;
  struct wl_list link;
  int32_t scale;
  struct swappy_buffer *buffer;

  double logical_scale;  // guessed from the logical size
  char *name;

  enum wl_output_transform transform;
  struct zwlr_screencopy_frame_v1 *screencopy_frame;
  uint32_t screencopy_frame_flags;  // enum zwlr_screencopy_frame_v1_flags

#ifdef HAVE_WAYLAND_PROTOCOLS
  struct zxdg_output_v1 *xdg_output;
#endif
};

struct swappy_wayland {
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_shm *shm;
  struct wl_list outputs;
  struct zwlr_screencopy_manager_v1 *zwlr_screencopy_manager;
  size_t n_done;
#ifdef HAVE_WAYLAND_PROTOCOLS
  struct zxdg_output_manager_v1 *xdg_output_manager;
#endif
};

struct swappy_config {
  char *config_file;
  char *save_dir;
  guint32 line_size;
  guint32 text_size;
  guint32 blur_level;
  char *text_font;
};

struct swappy_state {
  GtkApplication *app;

  struct swappy_state_ui *ui;
  struct swappy_config *config;
  struct swappy_wayland *wl;

  cairo_surface_t *cairo_surface;
  GList *patterns;  // List of cairo_pattern_t

  enum swappy_paint_type mode;

  /* Options */
  char *geometry_str;
  char *file_str;
  char *output_file;

  struct swappy_box *window;
  struct swappy_box *geometry;

  GList *paints;
  GList *redo_paints;
  struct swappy_paint *temp_paint;

  struct swappy_state_settings settings;

  int argc;
  char **argv;
};
