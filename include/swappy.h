#pragma once

#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_PATH 4096

#define SWAPPY_LINE_SIZE_MIN 1
#define SWAPPY_LINE_SIZE_MAX 50

#define SWAPPY_TEXT_SIZE_MIN 10
#define SWAPPY_TEXT_SIZE_MAX 50

#define SWAPPY_TRANSPARENCY_MIN 5
#define SWAPPY_TRANSPARENCY_MAX 95

enum swappy_paint_type {
  SWAPPY_PAINT_MODE_BRUSH = 0, /* Brush mode to draw arbitrary shapes */
  SWAPPY_PAINT_MODE_TEXT,      /* Mode to draw texts */
  SWAPPY_PAINT_MODE_RECTANGLE, /* Rectangle shapes */
  SWAPPY_PAINT_MODE_ELLIPSE,   /* Ellipse shapes */
  SWAPPY_PAINT_MODE_ARROW,     /* Arrow shapes */
  SWAPPY_PAINT_MODE_BLUR,      /* Blur mode */
};

enum swappy_paint_shape_operation {
  SWAPPY_PAINT_SHAPE_OPERATION_STROKE = 0, /* Used to stroke the shape */
  SWAPPY_PAINT_SHAPE_OPERATION_FILL,       /* Used to fill the shape */
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
  glong cursor;
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
  bool should_center_at_from;
  struct swappy_point from;
  struct swappy_point to;
  enum swappy_paint_type type;
  enum swappy_paint_shape_operation operation;
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
  struct swappy_point from;
  struct swappy_point to;
  cairo_surface_t *surface;
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
  int32_t tr;
};

struct swappy_state_ui {
  gboolean panel_toggled;

  GtkWindow *window;
  GtkIMContext *im_context;

  GtkWidget *area;

  GtkToggleButton *panel_toggle_button;

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

  GtkButton *line_size;
  GtkButton *text_size;
  GtkButton *transparency;
  GtkButton *transparency_plus;
  GtkButton *transparency_minus;

  GtkToggleButton *fill_shape;
  GtkToggleButton *transparent;
};

struct swappy_config {
  char *config_file;
  char *save_dir;
  char *save_filename_format;
  gint8 paint_mode;
  gboolean fill_shape;
  gboolean transparent;
  gboolean show_panel;
  guint32 line_size;
  guint32 text_size;
  guint32 transparency;
  char *text_font;
  gboolean early_exit;
  gboolean auto_save;
  char *custom_color;
};

struct swappy_state {
  GtkApplication *app;

  struct swappy_state_ui *ui;
  struct swappy_config *config;

  GdkPixbuf *original_image;
  cairo_surface_t *original_image_surface;
  cairo_surface_t *rendering_surface;

  gdouble scaling_factor;

  enum swappy_paint_type mode;

  /* Options */
  char *file_str;
  char *output_file;

  char *temp_file_str;

  struct swappy_box *window;
  struct swappy_box *geometry;

  GList *paints;
  GList *redo_paints;
  struct swappy_paint *temp_paint;

  struct swappy_state_settings settings;

  int argc;
  char **argv;
};
