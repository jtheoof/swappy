#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>
#include <time.h>

#include "clipboard.h"
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

  if (state->shapes) {
    g_slist_free_full(state->shapes, g_free);
    state->shapes = NULL;
  }

  if (state->temp_shape) {
    g_free(state->temp_shape);
    state->temp_shape = NULL;
  }
}

static void switch_mode_to_brush(struct swappy_state *state) {
  g_debug("switching mode to brush");
  state->mode = SWAPPY_PAINT_MODE_BRUSH;
}

static void switch_mode_to_text(struct swappy_state *state) {
  g_debug("switching mode to arrow");
  state->mode = SWAPPY_PAINT_MODE_TEXT;
}

static void switch_mode_to_rectangle(struct swappy_state *state) {
  g_debug("switching mode to rectangle");
  state->mode = SWAPPY_PAINT_MODE_RECTANGLE;
}

static void switch_mode_to_ellipse(struct swappy_state *state) {
  g_debug("switching mode to ellipse");
  state->mode = SWAPPY_PAINT_MODE_ELLIPSE;
}

static void switch_mode_to_arrow(struct swappy_state *state) {
  g_debug("switching mode to arrow");
  state->mode = SWAPPY_PAINT_MODE_ARROW;
}

void brush_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  switch_mode_to_brush(state);
}

void text_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  switch_mode_to_text(state);
}

void rectangle_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  switch_mode_to_rectangle(state);
}

void ellipse_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  switch_mode_to_ellipse(state);
}

void arrow_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  switch_mode_to_arrow(state);
}

void application_finish(struct swappy_state *state) {
  g_debug("application is shutting down");
  swappy_overlay_clear(state);
  cairo_surface_destroy(state->cairo_surface);
  g_free(state->temp_shape);
  g_free(state->storage_path);
  g_free(state->geometry_str);
  g_free(state->geometry);
  g_resources_unregister(state->resource);
  g_free(state->popover);
  g_object_unref(state->app);
}

static gboolean draw_area_handler(GtkWidget *widget, cairo_t *cr,
                                  struct swappy_state *state) {
  cairo_set_source_surface(cr, state->cairo_surface, 0, 0);
  cairo_paint(cr);

  return FALSE;
}

static gboolean configure_event_handler(GtkWidget *area,
                                        GdkEventConfigure *event,
                                        struct swappy_state *state) {
  g_debug("received configure_event handler");
  cairo_surface_destroy(state->cairo_surface);

  state->cairo_surface = gdk_window_create_similar_surface(
      gtk_widget_get_window(area), CAIRO_CONTENT_COLOR,
      gtk_widget_get_allocated_width(area),
      gtk_widget_get_allocated_height(area));

  draw_state(state);

  return TRUE;
}

static void action_save_area_to_file(struct swappy_state *state) {
  g_debug("saving area to file");

  guint width = gtk_widget_get_allocated_width(state->area);
  guint height = gtk_widget_get_allocated_height(state->area);
  // GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(state->area));
  g_debug("generating pixbuf from area");
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_surface(state->cairo_surface, 0, 0, width, height);
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
  draw_state(state);
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
  clipboard_copy_drawing_area_to_selection(state);
}

static void keypress_handler(GtkWidget *widget, GdkEventKey *event,
                             struct swappy_state *state) {
  g_debug("keypress_handler key pressed: %d\n", event->keyval);
  if (event->keyval == GDK_KEY_Escape) {
    g_debug("keypress_handler: escape key pressed, ciao bye\n");
    gtk_window_close(state->window);
  } else if (event->keyval == GDK_KEY_B || event->keyval == GDK_KEY_b) {
    switch_mode_to_brush(state);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->brush),
                                 true);
  } else if (event->keyval == GDK_KEY_T || event->keyval == GDK_KEY_t) {
    switch_mode_to_text(state);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->text), true);
  } else if (event->keyval == GDK_KEY_R || event->keyval == GDK_KEY_r) {
    switch_mode_to_rectangle(state);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->rectangle),
                                 true);
  } else if (event->keyval == GDK_KEY_O || event->keyval == GDK_KEY_o) {
    switch_mode_to_ellipse(state);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->ellipse),
                                 true);
  } else if (event->keyval == GDK_KEY_A || event->keyval == GDK_KEY_a) {
    switch_mode_to_arrow(state);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->arrow),
                                 true);
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

static void paint_add_temporary(struct swappy_state *state, double x, double y,
                                enum swappy_paint_mode_type type) {
  struct swappy_shape *temp = g_new(struct swappy_shape, 1);

  temp->from.x = x;
  temp->from.y = y;
  temp->type = type;

  if (state->temp_shape) {
    g_free(state->temp_shape);
    state->temp_shape = NULL;
  }

  state->temp_shape = temp;
}

static void paint_update_temporary(struct swappy_state *state, double x,
                                   double y) {
  if (!state->temp_shape) {
    return;
  }

  state->temp_shape->to.x = x;
  state->temp_shape->to.y = y;
}

static void paint_commit_temporary(struct swappy_state *state, double x,
                                   double y) {
  if (!state->temp_shape) {
    return;
  }

  state->temp_shape->to.x = x;
  state->temp_shape->to.y = y;

  g_debug("commit temp paint: from: %lf,%lf to: %lf,%lf",
          state->temp_shape->from.x, state->temp_shape->from.y,
          state->temp_shape->to.x, state->temp_shape->to.y);

  struct swappy_shape *shape =
      g_memdup(state->temp_shape, sizeof(struct swappy_shape));

  state->shapes = g_slist_append(state->shapes, shape);
}

static void draw_area_button_press_handler(GtkWidget *widget,
                                           GdkEventButton *event,
                                           struct swappy_state *state) {
  g_debug("press event: state: %d, button: %d", event->state, event->button);
  if (event->button == 3) {
    gtk_popover_popup(state->popover->container);
  }

  if (event->button == 1) {
    if (state->mode == SWAPPY_PAINT_MODE_RECTANGLE) {
      paint_add_temporary(state, event->x, event->y,
                          SWAPPY_PAINT_MODE_RECTANGLE);
    }
  }
}

static void draw_area_button_release_handler(GtkWidget *widget,
                                             GdkEventButton *event,
                                             struct swappy_state *state) {
  g_debug("releasing button in state: %d", event->state);
  if (!(event->state & GDK_BUTTON1_MASK)) {
    return;
  }

  switch (state->mode) {
    case SWAPPY_PAINT_MODE_BRUSH:
      brush_add_point(state, event->x, event->y, SWAPPY_BRUSH_POINT_LAST);
      draw_state(state);
      break;

    case SWAPPY_PAINT_MODE_RECTANGLE:
      paint_commit_temporary(state, event->x, event->y);
      draw_state(state);
      break;
    default:
      return;
  }
}

static void draw_area_motion_notify_handler(GtkWidget *widget,
                                            GdkEventMotion *event,
                                            struct swappy_state *state) {
  GdkDisplay *display = gdk_display_get_default();
  GdkWindow *window = event->window;
  GdkCursor *crosshair = gdk_cursor_new_for_display(display, GDK_CROSSHAIR);
  gdk_window_set_cursor(window, crosshair);

  if (state->mode == SWAPPY_PAINT_MODE_BRUSH) {
    if (event->state & GDK_BUTTON1_MASK) {
      brush_add_point(state, event->x, event->y, SWAPPY_BRUSH_POINT_WITHIN);
      draw_state(state);
    }
  } else if (state->mode == SWAPPY_PAINT_MODE_RECTANGLE) {
    if (event->state & GDK_BUTTON1_MASK) {
      paint_update_temporary(state, event->x, event->y);
      draw_state(state);
    }
  } else {
    gdk_window_set_cursor(window, NULL);
  }
}

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider) {
  gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider,
                                 1);
  if (GTK_IS_CONTAINER(widget)) {
    gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)apply_css,
                         provider);
  }
}

static void load_css(struct swappy_state *state) {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/swappy/style/popover.css");
  apply_css(GTK_WIDGET(state->popover->container),
            GTK_STYLE_PROVIDER(provider));
  g_object_unref(provider);
}

static bool load_layout(struct swappy_state *state) {
  GtkBuilder *builder;
  GObject *container;
  GtkWidget *area;
  GObject *copy;
  GObject *save;
  GObject *clear;
  GtkPopover *popover;
  GtkRadioButton *brush;
  GtkRadioButton *text;
  GtkRadioButton *rectangle;
  GtkRadioButton *ellipse;
  GtkRadioButton *arrow;
  GError *error = NULL;

  /* Construct a GtkBuilder instance and load our UI description */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_resource(builder, "/swappy/swappy.ui", &error) ==
      0) {
    g_printerr("Error loading file: %s", error->message);
    g_clear_error(&error);
    return false;
  }

  /* Connect signal handlers to the constructed widgets. */
  container = gtk_builder_get_object(builder, "container");

  copy = gtk_builder_get_object(builder, "copy");
  save = gtk_builder_get_object(builder, "save");
  clear = gtk_builder_get_object(builder, "clear");
  area = GTK_WIDGET(gtk_builder_get_object(builder, "paint_area"));

  popover = GTK_POPOVER(gtk_builder_get_object(builder, "popover"));
  brush = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "brush"));
  text = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "text"));
  rectangle = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "rectangle"));
  ellipse = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "ellipse"));
  arrow = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "arrow"));

  gtk_builder_connect_signals(builder, state);

  g_signal_connect(G_OBJECT(state->window), "key_press_event",
                   G_CALLBACK(keypress_handler), state);
  g_signal_connect(brush, "toggled",
                   G_CALLBACK(tools_menu_button_brush_toggle_handler), state);
  g_signal_connect(text, "toggled",
                   G_CALLBACK(tools_menu_button_text_toggle_handler), state);
  g_signal_connect(rectangle, "toggled",
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
  g_signal_connect(area, "draw", G_CALLBACK(draw_area_handler), state);
  g_signal_connect(area, "configure-event", G_CALLBACK(configure_event_handler),
                   state);
  g_signal_connect(area, "button-press-event",
                   G_CALLBACK(draw_area_button_press_handler), state);
  g_signal_connect(area, "button-release-event",
                   G_CALLBACK(draw_area_button_release_handler), state);
  g_signal_connect(area, "motion-notify-event",
                   G_CALLBACK(draw_area_motion_notify_handler), state);

  gtk_container_add(GTK_CONTAINER(state->window), GTK_WIDGET(container));
  gtk_popover_set_relative_to(popover, area);

  state->popover->container = popover;
  state->popover->brush = brush;
  state->popover->text = text;
  state->popover->rectangle = rectangle;
  state->popover->ellipse = ellipse;
  state->popover->arrow = arrow;
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
  // gtk_layer_init_for_window(window);

  // Order above normal windows
  // gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_window_set_default_size(window, geometry->width, geometry->height);

  // Need to set keyboard interactivity for key bindings
  // gtk_layer_set_keyboard_interactivity(window, true);

  // Push other windows out of the way
  // gtk_layer_auto_exclusive_zone_enable(window);

  // The margins are the gaps around the window's edges
  // Margins and anchors can be set like this...
  // gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, 1);
  // gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_RIGHT, 1);
  // gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, 1);
  // gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_BOTTOM, 1);

  gtk_window_get_size(window, &state->width, &state->height);
  gtk_window_move(window, geometry->x, geometry->y);

  g_debug("window has sizes %dx%d", state->width, state->height);

  load_layout(state);
  load_css(state);
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
  GError *error = NULL;
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

  state->resource = g_resource_load("build/meson-out/swappy.gresource", &error);

  if (error != NULL) {
    g_error("unable to load swappy resource file: %s", error->message);
    g_error_free(error);
  }

  state->popover = g_new(struct swappy_state_ui_popover, 1);

  g_resources_register(state->resource);

  g_signal_connect(state->app, "command-line", G_CALLBACK(command_line_handler),
                   state);

  return true;
}

int application_run(struct swappy_state *state) {
  return g_application_run(G_APPLICATION(state->app), state->argc, state->argv);
}
