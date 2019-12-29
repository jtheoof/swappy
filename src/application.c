#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <time.h>

#include "clipboard.h"
#include "file.h"
#include "notification.h"
#include "paint.h"
#include "render.h"
#include "screencopy.h"
#include "swappy.h"
#include "wayland.h"

static void update_ui_undo_redo(struct swappy_state *state) {
  GtkWidget *undo = GTK_WIDGET(state->ui->undo);
  GtkWidget *redo = GTK_WIDGET(state->ui->redo);
  gboolean undo_sensitive = g_list_length(state->paints) > 0;
  gboolean redo_sensitive = g_list_length(state->redo_paints) > 0;
  gtk_widget_set_sensitive(undo, undo_sensitive);
  gtk_widget_set_sensitive(redo, redo_sensitive);
}

static void update_ui_stroke_size_widget(struct swappy_state *state) {
  GtkButton *button = GTK_BUTTON(state->ui->stroke_size);
  char label[255];
  snprintf(label, 255, "%.0lf", state->painting.w);
  gtk_button_set_label(button, label);
}

static void action_undo(struct swappy_state *state) {
  GList *first = state->paints;

  if (first) {
    state->paints = g_list_remove_link(state->paints, first);
    state->redo_paints = g_list_prepend(state->redo_paints, first->data);

    render_state(state);
    update_ui_undo_redo(state);
  }
}

static void action_redo(struct swappy_state *state) {
  GList *first = state->redo_paints;

  if (first) {
    state->redo_paints = g_list_remove_link(state->redo_paints, first);
    state->paints = g_list_prepend(state->paints, first->data);

    render_state(state);
    update_ui_undo_redo(state);
  }
}

static void action_clear(struct swappy_state *state) {
  paint_free_all(state);
  render_state(state);
  update_ui_undo_redo(state);
}

static void action_toggle_painting_pane(struct swappy_state *state) {
  GtkWidget *painting_box = GTK_WIDGET(state->ui->painting_box);
  gboolean is_visible = gtk_widget_get_visible(painting_box);
  gtk_widget_set_visible(painting_box, !is_visible);
}

static void action_update_color_state(struct swappy_state *state, double r,
                                      double g, double b, double a,
                                      gboolean custom) {
  state->painting.r = r;
  state->painting.g = g;
  state->painting.b = b;
  state->painting.a = a;

  gtk_widget_set_sensitive(GTK_WIDGET(state->ui->color), custom);
}

static void action_set_color_from_custom(struct swappy_state *state) {
  GdkRGBA color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(state->ui->color), &color);

  action_update_color_state(state, color.red, color.green, color.blue,
                            color.alpha, true);
}

static void switch_mode_to_brush(struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_BRUSH;
}

static void switch_mode_to_text(struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_TEXT;
}

static void switch_mode_to_rectangle(struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_RECTANGLE;
}

static void switch_mode_to_ellipse(struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_ELLIPSE;
}

static void switch_mode_to_arrow(struct swappy_state *state) {
  state->mode = SWAPPY_PAINT_MODE_ARROW;
}

static void action_stroke_size_decrease(struct swappy_state *state) {
  guint step = state->painting.w <= 10 ? 1 : 5;

  state->painting.w -= step;

  if (state->painting.w < SWAPPY_STROKE_SIZE_MIN) {
    state->painting.w = SWAPPY_STROKE_SIZE_MIN;
  }

  update_ui_stroke_size_widget(state);
}

static void action_stroke_size_reset(struct swappy_state *state) {
  state->painting.w = SWAPPY_STROKE_SIZE_DEFAULT;

  update_ui_stroke_size_widget(state);
}

static void action_stroke_size_increase(struct swappy_state *state) {
  guint step = state->painting.w >= 10 ? 5 : 1;
  state->painting.w += step;

  if (state->painting.w > SWAPPY_STROKE_SIZE_MAX) {
    state->painting.w = SWAPPY_STROKE_SIZE_MAX;
  }

  update_ui_stroke_size_widget(state);
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
  paint_free_all(state);
  cairo_surface_destroy(state->cairo_surface);
  g_free(state->storage_path);
  g_free(state->geometry_str);
  g_free(state->geometry);
  g_free(state->ui);
  g_object_unref(state->app);
}

static void action_save_area_to_file(struct swappy_state *state) {
  guint width = gtk_widget_get_allocated_width(state->ui->area);
  guint height = gtk_widget_get_allocated_height(state->ui->area);
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
  gdk_pixbuf_savev(pixbuf, path, "png", NULL, NULL, &error);

  if (error != NULL) {
    g_critical("unable to save drawing area to pixbuf: %s", error->message);
    g_error_free(error);
  }

  char *msg = "Saved Swappshot to: ";
  size_t len = strlen(msg) + strlen(path) + 1;
  char *message = g_new(char, len);
  snprintf(message, len, "%s%s", msg, path);
  notification_send("Swappy", message);
  g_free(message);
}

void save_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_save_area_to_file(state);
}

void clear_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_clear(state);
}

void copy_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  clipboard_copy_drawing_area_to_selection(state);
}

void window_keypress_handler(GtkWidget *widget, GdkEventKey *event,
                             struct swappy_state *state) {
  if (event->state & GDK_CONTROL_MASK) {
    switch (event->keyval) {
      case GDK_KEY_c:
        clipboard_copy_drawing_area_to_selection(state);
        break;
      case GDK_KEY_s:
        action_save_area_to_file(state);
        break;
      case GDK_KEY_b:
        action_toggle_painting_pane(state);
        break;
      case GDK_KEY_z:
        action_undo(state);
        break;
      case GDK_KEY_Z:
      case GDK_KEY_y:
        action_redo(state);
        break;
      default:
        break;
    }
  } else {
    switch (event->keyval) {
      case GDK_KEY_Escape:
      case GDK_KEY_q:
        gtk_main_quit();
        break;
      case GDK_KEY_b:
        switch_mode_to_brush(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->brush), true);
        break;
      case GDK_KEY_t:
        switch_mode_to_text(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->text), true);
        break;
      case GDK_KEY_r:
        switch_mode_to_rectangle(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->rectangle),
                                     true);
        break;
      case GDK_KEY_o:
        switch_mode_to_ellipse(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->ellipse),
                                     true);
        break;
      case GDK_KEY_a:
        switch_mode_to_arrow(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->arrow), true);
        break;
      case GDK_KEY_k:
        action_clear(state);
        break;
      case GDK_KEY_R:
        action_update_color_state(state, 1, 0, 0, 1, false);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->red), true);
        break;
      case GDK_KEY_G:
        action_update_color_state(state, 0, 1, 0, 1, false);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->green), true);
        break;
      case GDK_KEY_B:
        action_update_color_state(state, 0, 0, 1, 1, false);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->blue), true);
        break;
      case GDK_KEY_C:
        action_set_color_from_custom(state);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->ui->custom),
                                     true);
        break;
      case GDK_KEY_minus:
        action_stroke_size_decrease(state);
        break;
      case GDK_KEY_equal:
        action_stroke_size_reset(state);
        break;
      case GDK_KEY_plus:
        action_stroke_size_increase(state);
        break;
      default:
        break;
    }
  }
}

void undo_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_undo(state);
}

void redo_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_redo(state);
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
  if (!(event->state & GDK_BUTTON1_MASK)) {
    return;
  }

  switch (state->mode) {
    case SWAPPY_PAINT_MODE_BRUSH:
    case SWAPPY_PAINT_MODE_RECTANGLE:
    case SWAPPY_PAINT_MODE_ELLIPSE:
    case SWAPPY_PAINT_MODE_ARROW:
      paint_commit_temporary(state);
      paint_free_list(&state->redo_paints);
      render_state(state);
      update_ui_undo_redo(state);
      break;
    default:
      return;
  }
}

void color_red_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_update_color_state(state, 1, 0, 0, 1, false);
}

void color_green_clicked_handler(GtkWidget *widget,
                                 struct swappy_state *state) {
  action_update_color_state(state, 0, 1, 0, 1, false);
}

void color_blue_clicked_handler(GtkWidget *widget, struct swappy_state *state) {
  action_update_color_state(state, 0, 0, 1, 1, false);
}

void color_custom_clicked_handler(GtkWidget *widget,
                                  struct swappy_state *state) {
  action_set_color_from_custom(state);
}

void color_custom_color_set_handler(GtkWidget *widget,
                                    struct swappy_state *state) {
  action_set_color_from_custom(state);
}

void stroke_size_decrease_handler(GtkWidget *widget,
                                  struct swappy_state *state) {
  action_stroke_size_decrease(state);
}

void stroke_size_reset_handler(GtkWidget *widget, struct swappy_state *state) {
  action_stroke_size_reset(state);
}
void stroke_size_increase_handler(GtkWidget *widget,
                                  struct swappy_state *state) {
  action_stroke_size_increase(state);
}

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider) {
  gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider,
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if (GTK_IS_CONTAINER(widget)) {
    gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)apply_css,
                         provider);
  }
}

static bool load_css(struct swappy_state *state) {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider,
                                      "/me/jtheoof/swappy/style/swappy.css");
  apply_css(GTK_WIDGET(state->ui->window), GTK_STYLE_PROVIDER(provider));
  g_object_unref(provider);
  return true;
}

static bool load_layout(struct swappy_state *state) {
  struct swappy_box *geometry = state->geometry;
  GError *error = NULL;

  /* Construct a GtkBuilder instance and load our UI description */
  GtkBuilder *builder = gtk_builder_new();
  if (gtk_builder_add_from_resource(builder, "/me/jtheoof/swappy/swappy.ui",
                                    &error) == 0) {
    g_printerr("Error loading file: %s", error->message);
    g_clear_error(&error);
    return false;
  }

  gtk_builder_connect_signals(builder, state);

  GtkWindow *window =
      GTK_WINDOW(gtk_builder_get_object(builder, "paint-window"));

  state->ui->undo = GTK_BUTTON(gtk_builder_get_object(builder, "undo-button"));
  state->ui->redo = GTK_BUTTON(gtk_builder_get_object(builder, "redo-button"));

  GtkWidget *area =
      GTK_WIDGET(gtk_builder_get_object(builder, "painting-area"));

  state->ui->painting_box =
      GTK_BOX(gtk_builder_get_object(builder, "painting-box"));
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

  state->ui->red =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "color-red-button"));
  state->ui->green =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "color-green-button"));
  state->ui->blue =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "color-blue-button"));
  state->ui->custom =
      GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "color-custom-button"));
  state->ui->color =
      GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "custom-color-button"));

  state->ui->stroke_size =
      GTK_BUTTON(gtk_builder_get_object(builder, "stroke-size-button"));

  //  gtk_popover_set_relative_to(ui, area);
  gtk_widget_set_size_request(area, geometry->width, geometry->height);

  state->ui->brush = brush;
  state->ui->text = text;
  state->ui->rectangle = rectangle;
  state->ui->ellipse = ellipse;
  state->ui->arrow = arrow;
  state->ui->area = area;
  state->ui->window = window;

  g_object_unref(G_OBJECT(builder));

  return true;
}

static bool init_gtk_window(struct swappy_state *state) {
  g_info("activating application ----------");

  if (!load_layout(state)) {
    return false;
  }

  if (!load_css(state)) {
    return false;
  }

  update_ui_stroke_size_widget(state);
  update_ui_undo_redo(state);

  return true;
}

static gboolean has_option_geometry(struct swappy_state *state) {
  return (state->geometry_str != NULL);
}

static gboolean has_option_file(struct swappy_state *state) {
  return (state->file_str != NULL);
}

static gboolean is_file_valid(const char *file) {
  cairo_surface_t *surface = cairo_image_surface_create_from_png(file);
  cairo_status_t status = cairo_surface_status(surface);

  if (status) {
    g_warning("error while loading: %s - cairo status: %s", file,
              cairo_status_to_string(status));
    return false;
  }

  return true;
}

static gint command_line_handler(GtkApplication *app,
                                 GApplicationCommandLine *cmdline,
                                 struct swappy_state *state) {
  if (has_option_geometry(state)) {
    if (!screencopy_parse_geometry(state)) {
      return EXIT_FAILURE;
    }

    if (!screencopy_init(state)) {
      return EXIT_FAILURE;
    }
  }

  if (has_option_file(state)) {
    if (!is_file_valid(state->file_str)) {
      return EXIT_FAILURE;
    }
  }

  if (!init_gtk_window(state)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

bool application_init(struct swappy_state *state) {
  const GOptionEntry cli_options[] = {
      {
          .long_name = "geometry",
          .short_name = 'g',
          .arg = G_OPTION_ARG_STRING,
          .arg_data = &state->geometry_str,
          .description =
              "Set the region to capture. (Can be an output of slurp)",
      },
      {
          .long_name = "file",
          .short_name = 'f',
          .arg = G_OPTION_ARG_STRING,
          .arg_data = &state->file_str,
          .description = "Load a file at a specific path.",
      },
      {NULL}};

  state->app = gtk_application_new("me.jtheoof.swappy",
                                   G_APPLICATION_HANDLES_COMMAND_LINE);

  if (state->app == NULL) {
    g_critical("cannot create gtk application");
    return false;
  }

  g_application_add_main_option_entries(G_APPLICATION(state->app), cli_options);

  state->ui = g_new(struct swappy_state_ui, 1);

  g_signal_connect(state->app, "command-line", G_CALLBACK(command_line_handler),
                   state);

  state->painting.r = 1;
  state->painting.g = 0;
  state->painting.b = 0;
  state->painting.a = 1;
  state->painting.w = SWAPPY_STROKE_SIZE_DEFAULT;

  return true;
}

int application_run(struct swappy_state *state) {
  return g_application_run(G_APPLICATION(state->app), state->argc, state->argv);
}
