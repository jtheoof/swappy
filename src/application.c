#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>
#include <time.h>

#include "draw.h"
#include "notification.h"
#include "screencopy.h"
#include "swappy.h"
#include "wayland.h"

static void swappy_overlay_clear(struct swappy_state *state) {
  if (state->brushes) {
    g_slist_free_full(state->brushes, g_free);
    state->brushes = NULL;
  }
}

void application_finish(struct swappy_state *state) {
  g_debug("application is shutting down");
  swappy_overlay_clear(state);
  g_free(state->storage_path);
  g_free(state->geometry_str);
  g_free(state->geometry);
  g_object_unref(state->app);
}

static void action_save_area_to_file(struct swappy_state *state) {
  GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(state->area));
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_window(window, 0, 0, state->width, state->height);
  GError *error = NULL;

  time_t current_time;
  char *c_time_string;

  time(&current_time);

  c_time_string = ctime(&current_time);
  c_time_string[strlen(c_time_string) - 1] = '\0';
  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "%s/%s %s", state->storage_path, "Swapp Shot",
           c_time_string);
  g_info("saving swapp shot to: \"%s\"", path);
  gdk_pixbuf_savev(pixbuf, path, "png", NULL, NULL, &error);

  if (error != NULL) {
    g_error("unable to save drawing area to pixbuf: %s", error->message);
    g_error_free(error);
  }

  char message[MAX_PATH];
  snprintf(message, MAX_PATH, "Saved Swapp Shot to: %s\n", path);
  notification_send("Swappy", message);
}

static void tools_menu_button_save_clicked_handler(GtkButton *button,
                                                   struct swappy_state *state) {
  action_save_area_to_file(state);
}

static void tools_menu_button_clear_clicked_handler(
    GtkWidget *widget, struct swappy_state *state) {
  swappy_overlay_clear(state);
  gtk_widget_queue_draw(state->area);
}

static void tools_menu_button_brush_toggle_handler(GtkToggleButton *source,
                                                   struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_BRUSH;
}

static void tools_menu_button_text_toggle_handler(GtkToggleButton *source,
                                                  struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_TEXT;
}

static void tools_menu_button_shape_toggle_handler(GtkToggleButton *source,
                                                   struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_RECTANGLE;
}

static void tools_menu_button_copy_clicked_handler(GtkToggleButton *source,
                                                   struct swappy_state *state) {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  char text[255] = "This is a nice text";
  gtk_clipboard_set_text(clipboard, text, -1);
  gtk_clipboard_store(clipboard);
  g_debug("clipboard stored\n");
}

static void keypress_handler(GtkWidget *widget, GdkEventKey *event,
                             gpointer data) {
  struct swappy_state *state = data;
  g_debug("keypress_handler key pressed: %d\n", event->keyval);
  if (event->keyval == GDK_KEY_Escape) {
    g_debug("keypress_handler: escape key pressed, ciao bye\n");
    gtk_window_close(state->window);
  }
}

static void brush_add_point(struct swappy_state *state, double x, double y,
                            enum swappy_brush_point_kind kind) {
  struct swappy_brush_point *point = g_new(struct swappy_brush_point, 1);
  point->x = x;
  point->y = y;
  point->r = 1;
  point->g = 0;
  point->b = 0;
  point->a = 1;
  point->kind = kind;

  state->brushes = g_slist_append(state->brushes, point);
}

static void draw_area_button_press_handler(GtkWidget *widget,
                                           GdkEventButton *event,
                                           struct swappy_state *state) {
  if (!(event->state & GDK_BUTTON1_MASK)) {
    return;
  }

  if (state->mode == SWAPPY_PAINT_MODE_BRUSH) {
    brush_add_point(state, event->x, event->y, SWAPPY_BRUSH_POINT_FIRST);
    gtk_widget_queue_draw(state->area);
  }
}

static void draw_area_button_release_handler(GtkWidget *widget,
                                             GdkEventButton *event,
                                             struct swappy_state *state) {
  if (!(event->state & GDK_BUTTON1_MASK)) {
    return;
  }

  if (state->mode == SWAPPY_PAINT_MODE_BRUSH) {
    brush_add_point(state, event->x, event->y, SWAPPY_BRUSH_POINT_LAST);
    gtk_widget_queue_draw(state->area);
  }
}

static void draw_area_motion_notify_handler(GtkWidget *widget,
                                            GdkEventMotion *event,
                                            struct swappy_state *state) {
  GdkDisplay *display = gdk_display_get_default();
  GdkWindow *window = event->window;

  if (state->mode == SWAPPY_PAINT_MODE_BRUSH) {
    GdkCursor *crosshair = gdk_cursor_new_for_display(display, GDK_CROSSHAIR);
    gdk_window_set_cursor(window, crosshair);

    if (event->state & GDK_BUTTON1_MASK) {
      brush_add_point(state, event->x, event->y, SWAPPY_BRUSH_POINT_WITHIN);
      gtk_widget_queue_draw(state->area);
    }
  } else {
    gdk_window_set_cursor(window, NULL);
  }
}

static bool build_ui(struct swappy_state *state) {
  GtkBuilder *builder;
  GObject *container;
  GObject *brush;
  GObject *text;
  GObject *shape;
  GtkWidget *area;
  GObject *copy;
  GObject *save;
  GObject *clear;
  GError *error = NULL;

  /* Construct a GtkBuilder instance and load our UI description */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "swappy.ui", &error) == 0) {
    g_printerr("Error loading file: %s", error->message);
    g_clear_error(&error);
    return false;
  }

  /* Connect signal handlers to the constructed widgets. */
  container = gtk_builder_get_object(builder, "container");
  brush = gtk_builder_get_object(builder, "brush");
  text = gtk_builder_get_object(builder, "text");
  shape = gtk_builder_get_object(builder, "shape");
  copy = gtk_builder_get_object(builder, "copy");
  save = gtk_builder_get_object(builder, "save");
  clear = gtk_builder_get_object(builder, "clear");
  area = GTK_WIDGET(gtk_builder_get_object(builder, "paint_area"));

  gtk_container_add(GTK_CONTAINER(state->window), GTK_WIDGET(container));

  g_signal_connect(G_OBJECT(state->window), "key_press_event",
                   G_CALLBACK(keypress_handler), state);
  g_signal_connect(brush, "toggled",
                   G_CALLBACK(tools_menu_button_brush_toggle_handler), state);
  g_signal_connect(text, "toggled",
                   G_CALLBACK(tools_menu_button_text_toggle_handler), state);
  g_signal_connect(shape, "toggled",
                   G_CALLBACK(tools_menu_button_shape_toggle_handler), state);
  g_signal_connect(copy, "clicked",
                   G_CALLBACK(tools_menu_button_copy_clicked_handler), state);
  g_signal_connect(save, "clicked",
                   G_CALLBACK(tools_menu_button_save_clicked_handler), state);
  g_signal_connect(clear, "clicked",
                   G_CALLBACK(tools_menu_button_clear_clicked_handler), state);

  gtk_widget_set_size_request(area, state->width, state->height);
  gtk_widget_add_events(area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK |
                                  GDK_BUTTON1_MOTION_MASK);
  g_signal_connect(area, "draw", G_CALLBACK(draw_area), state);
  g_signal_connect(area, "button-press-event",
                   G_CALLBACK(draw_area_button_press_handler), state);
  g_signal_connect(area, "button-release-event",
                   G_CALLBACK(draw_area_button_release_handler), state);
  g_signal_connect(area, "motion-notify-event",
                   G_CALLBACK(draw_area_motion_notify_handler), state);

  state->area = area;
  g_object_unref(G_OBJECT(builder));
  gtk_widget_show_all(GTK_WIDGET(state->window));

  return true;
}

static void init_layer_shell(GtkApplication *app, struct swappy_state *state) {
  g_info("activating application ----------");

  // Create a normal GTK vbox however you like
  GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
  struct swappy_box *geometry = state->geometry;
  state->window = window;

  // Before the window is first realized, set it up to be a layer surface
  gtk_layer_init_for_window(window);

  // Order above normal windows
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_window_set_default_size(window, geometry->width, geometry->height);

  // Need to set keyboard interactivity for key bindings
  gtk_layer_set_keyboard_interactivity(window, true);

  // Push other windows out of the way
  gtk_layer_auto_exclusive_zone_enable(window);

  // The margins are the gaps around the window's edges
  // Margins and anchors can be set like this...
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, 1);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_RIGHT, 1);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, 1);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_BOTTOM, 1);

  gtk_window_get_size(window, &state->width, &state->height);
  gtk_window_move(window, geometry->x, geometry->y);

  g_debug("window has sizes %dx%d", state->width, state->height);

  build_ui(state);
}

static gboolean is_geometry_valid(struct swappy_state *state) {
  return (state->geometry_str != NULL);
}

static gint command_line_handler(GtkApplication *app,
                                 GApplicationCommandLine *cmdline,
                                 struct swappy_state *state) {
  if (!is_geometry_valid(state)) {
    g_warning("geometry parameter is missing");
    return EXIT_FAILURE;
  }

  g_debug("geometry is: %s", state->geometry_str);

  if (!screencopy_parse_geometry(state)) {
    return EXIT_FAILURE;
  }

  if (!screencopy_init(state)) {
    g_warning("unable to initialize zwlr_screencopy_v1");
    return false;
  }

  init_layer_shell(app, state);

  return EXIT_SUCCESS;
}

bool application_init(struct swappy_state *state) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
  const GOptionEntry cli_options[] = {
      {
          .long_name = "geometry",
          .short_name = 'g',
          .arg = G_OPTION_ARG_STRING,
          .arg_data = &state->geometry_str,
          .description =
              "Set the region to capture. (Can be an output of slurp)",
      },
      {NULL}};
#pragma clang diagnostic pop

  state->app = gtk_application_new("me.jtheoof.swappy",
                                   G_APPLICATION_HANDLES_COMMAND_LINE);

  if (state->app == NULL) {
    g_critical("cannot create gtk application");
    return false;
  }

  g_application_add_main_option_entries(G_APPLICATION(state->app), cli_options);

  g_signal_connect(state->app, "command-line", G_CALLBACK(command_line_handler),
                   state);

  return true;
}

int application_run(struct swappy_state *state) {
  return g_application_run(G_APPLICATION(state->app), state->argc, state->argv);
}
