#ifndef _SWAPPY_H
#define _SWAPPY_H

#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>

struct swappy_brush_point {
  double x;
  double y;
  double r;
  double g;
  double b;
  double a;
};

struct swappy_state {
  GtkApplication *app;
  GtkWindow *window;
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_shm *shm;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct wl_list outputs;  // mako_output::link
  struct wl_list seats;    // mako_seat::link

  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;

  bool should_exit;
  bool is_mode_brush;
  bool is_mode_text;

  int width;
  int height;
  char image[255];

  GSList *brushes;

  int argc;
  char **argv;
};

#endif
