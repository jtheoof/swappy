#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>
#include <time.h>

#include "clipboard.h"
#include "notification.h"
#include "paint.h"
#include "render.h"
#include "screencopy.h"
#include "swappy.h"
#include "wayland.h"

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
  paint_free_all(state);
  cairo_surface_destroy(state->cairo_surface);
  g_free(state->storage_path);
  g_free(state->geometry_str);
  g_free(state->geometry);
  g_resources_unregister(state->resource);
  g_free(state->popover);
  g_object_unref(state->app);
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
  snprintf(path, MAX_PATH, "%s/%s %s", state->storage_path, "Swappshot",
           c_time_string);
  g_info("saving swappshot to: \"%s\"", path);
  gdk_pixbuf_savev(pixbuf, path, "png", NULL, NULL, &error);

  if (error != NULL) {
    g_error("unable to save drawing area to pixbuf: %s", error->message);
    g_error_free(error);
  }

  char message[MAX_PATH];
  snprintf(message, MAX_PATH, "Saved Swappshot to: %s\n", path);
  notification_send("Swappy", message);
}

void save_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_save_area_to_file(state);
}

void clear_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  paint_free_all(state);
  render_state(state);
}

void copy_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  clipboard_copy_drawing_area_to_selection(state);
}

void keypress_handler(GtkWidget *widget, GdkEventKey *event,
                      struct swappy_state *state) {
  g_debug("keypress_handler key pressed: keyval: %d - state: %d\n",
          event->keyval, event->state);

  switch (event->state) {
    case 0:
      switch (event->keyval) {
        case GDK_KEY_Escape:
          g_debug("keypress_handler: escape key pressed, ciao bye\n");
          gtk_main_quit();
          break;
        case GDK_KEY_b:
          switch_mode_to_brush(state);
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->brush),
                                       true);
          break;
        case GDK_KEY_t:
          switch_mode_to_text(state);
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->text),
                                       true);
          break;
        case GDK_KEY_r:
          switch_mode_to_rectangle(state);
          gtk_toggle_button_set_active(
              GTK_TOGGLE_BUTTON(state->popover->rectangle), true);
          break;
        case GDK_KEY_o:
          switch_mode_to_ellipse(state);
          gtk_toggle_button_set_active(
              GTK_TOGGLE_BUTTON(state->popover->ellipse), true);
          break;
        case GDK_KEY_a:
          switch_mode_to_arrow(state);
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->popover->arrow),
                                       true);
          break;
        default:
          break;
      }
      break;
    case GDK_CONTROL_MASK:
      switch (event->keyval) {
        case GDK_KEY_c:
          clipboard_copy_drawing_area_to_selection(state);
          break;
        case GDK_KEY_s:
          action_save_area_to_file(state);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

gboolean draw_area_handler(GtkWidget *widget, cairo_t *cr,
                           struct swappy_state *state) {
  cairo_set_source_surface(cr, state->cairo_surface, 0, 0);
  cairo_paint(cr);

  return FALSE;
}
gboolean draw_area_configure_handler(GtkWidget *widget,
                                     GdkEventConfigure *event,
                                     struct swappy_state *state) {
  g_debug("received configure_event handler");
  cairo_surface_destroy(state->cairo_surface);

  state->cairo_surface = gdk_window_create_similar_surface(
      gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
      gtk_widget_get_allocated_width(widget),
      gtk_widget_get_allocated_height(widget));

  render_state(state);

  return TRUE;
}
void draw_area_button_press_handler(GtkWidget *widget, GdkEventButton *event,
                                    struct swappy_state *state) {
  g_debug("press event: state: %d, button: %d", event->state, event->button);

  if (event->button == 1) {
    switch (state->mode) {
      case SWAPPY_PAINT_MODE_BRUSH:
      case SWAPPY_PAINT_MODE_RECTANGLE:
      case SWAPPY_PAINT_MODE_ELLIPSE:
      case SWAPPY_PAINT_MODE_ARROW:
        paint_add_temporary(state, event->x, event->y, state->mode);
        render_state(state);
        break;
      default:
        return;
    }
  }
}
void draw_area_motion_notify_handler(GtkWidget *widget, GdkEventMotion *event,
                                     struct swappy_state *state) {
  GdkDisplay *display = gdk_display_get_default();
  GdkWindow *window = event->window;
  GdkCursor *crosshair = gdk_cursor_new_for_display(display, GDK_CROSSHAIR);
  gdk_window_set_cursor(window, crosshair);

  gboolean is_button1_pressed = event->state & GDK_BUTTON1_MASK;

  switch (state->mode) {
    case SWAPPY_PAINT_MODE_BRUSH:
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      if (is_button1_pressed) {
        paint_update_temporary(state, event->x, event->y);
        render_state(state);
      }
      break;
    default:
      return;
  }
  g_object_unref(crosshair);
}
void draw_area_button_release_handler(GtkWidget *widget, GdkEventButton *event,
                                      struct swappy_state *state) {
  g_debug("releasing button in state: %d", event->state);
  if (!(event->state & GDK_BUTTON1_MASK)) {
    return;
  }

  switch (state->mode) {
    case SWAPPY_PAINT_MODE_BRUSH:
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      paint_commit_temporary(state);
      render_state(state);
      break;
    default:
      return;
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
  apply_css(GTK_WIDGET(state->window), GTK_STYLE_PROVIDER(provider));
  g_object_unref(provider);
}

static bool load_layout(struct swappy_state *state) {
  struct swappy_box *geometry = state->geometry;
  GError *error = NULL;

  /* Construct a GtkBuilder instance and load our UI description */
  GtkBuilder *builder = gtk_builder_new();
  if (gtk_builder_add_from_resource(builder, "/swappy/swappy.ui", &error) ==
      0) {
    g_printerr("Error loading file: %s", error->message);
    g_clear_error(&error);
    return false;
  }

  gtk_builder_connect_signals(builder, state);

  GtkWindow *window =
      GTK_WINDOW(gtk_builder_get_object(builder, "paint-window"));

  GtkWidget *area = GTK_WIDGET(gtk_builder_get_object(builder, "paint_area"));

  GtkRadioButton *brush =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "brush"));
  GtkRadioButton *text =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "text"));
  GtkRadioButton *rectangle =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "rectangle"));
  GtkRadioButton *ellipse =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "ellipse"));
  GtkRadioButton *arrow =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "arrow"));

  //  gtk_popover_set_relative_to(popover, area);
  gtk_widget_set_size_request(area, geometry->width, geometry->height);

  state->popover->brush = brush;
  state->popover->text = text;
  state->popover->rectangle = rectangle;
  state->popover->ellipse = ellipse;
  state->popover->arrow = arrow;
  state->area = area;
  state->window = window;

  g_object_unref(G_OBJECT(builder));

  return true;
}

static void init_gtk_window(struct swappy_state *state) {
  g_info("activating application ----------");

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

  init_gtk_window(state);

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
    g_critical("unable to load swappy resource file: %s", error->message);
    g_error_free(error);
  }

  state->popover = g_new(struct swappy_state_ui_painting, 1);

  g_resources_register(state->resource);

  g_signal_connect(state->app, "command-line", G_CALLBACK(command_line_handler),
                   state);

  return true;
}

int application_run(struct swappy_state *state) {
  return g_application_run(G_APPLICATION(state->app), state->argc, state->argv);
}
