#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>

#include "swappy.h"

void application_finish(struct swappy_state *state) {
  printf("calling application_finish\n");
  if (state->brushes) {
    for (GSList *point = state->brushes; point; point = point->next) {
      struct swappy_brush_point *bp = point->data;
      printf("freeing point with x: %lf, y: %lf\n", bp->x, bp->y);
    }
    g_slist_free_full(state->brushes, g_free);
  }
  g_object_unref(state->app);
}

static void tools_menu_button_brush_toggle_handler(GtkToggleButton *source,
                                                   struct swappy_state *state) {
  printf("brush toggled: %d\n", gtk_toggle_button_get_active(source));
  bool is_active = gtk_toggle_button_get_active(source) == 1;
  state->is_mode_brush = is_active;
  state->is_mode_text = !is_active;
}

static void tools_menu_button_text_toggle_handler(GtkToggleButton *source,
                                                  struct swappy_state *state) {
  printf("text toggled: %d\n", gtk_toggle_button_get_active(source));
  bool is_active = gtk_toggle_button_get_active(source) == 1;
  state->is_mode_brush = !is_active;
  state->is_mode_text = is_active;
}

static gboolean keypress_handler(GtkWidget *widget, GdkEventKey *event,
                                 gpointer data) {
  struct swappy_state *state = data;
  printf("keypress_handler key pressed: %d\n", event->keyval);
  if (event->keyval == GDK_KEY_Escape) {
    printf("keypress_handler: escape key pressed, ciao bye\n");
    state->should_exit = true;
    gtk_window_close(state->window);
    return TRUE;
  }
  return FALSE;
}

static void build_tools_menu(struct swappy_state *state, GtkWidget *parent) {
  // GtkWidget *grid = gtk_grid_new();
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_set_homogeneous(GTK_BOX(box), TRUE);
  GtkWidget *brush = gtk_radio_button_new_with_label(NULL, "Brush");
  GtkWidget *text = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(brush), "Text");

  g_signal_connect(brush, "toggled",
                   G_CALLBACK(tools_menu_button_brush_toggle_handler), state);
  g_signal_connect(text, "toggled",
                   G_CALLBACK(tools_menu_button_text_toggle_handler), state);

  gtk_container_add(GTK_CONTAINER(box), brush);
  gtk_container_add(GTK_CONTAINER(box), text);

  // gtk_grid_attach(GTK_GRID(grid), brush, 0, 0, 1, 1);
  // gtk_grid_attach(GTK_GRID(grid), text, 1, 0, 1, 1);
  gtk_container_add(GTK_CONTAINER(parent), box);
}

static void draw_area_handler(GtkWidget *widget, cairo_t *cr,
                              struct swappy_state *state) {
  guint width, height;
  GtkStyleContext *context;

  printf("drawing area\n");

  context = gtk_widget_get_style_context(widget);

  width = gtk_widget_get_allocated_width(widget);
  height = gtk_widget_get_allocated_height(widget);

  printf("found width: %d, height: %d\n", width, height);

  gtk_render_background(context, cr, 0, 0, width, height);

  // cairo_surface_t *image;
  // image = cairo_image_surface_create_from_png(state->image);
  // cairo_surface_flush(image);
  // cairo_surface_mark_dirty(image);
  // cairo_set_source_surface(cr, image, 0, 0);
  // cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
  // cairo_paint(cr);
  // cairo_surface_destroy(image);

  cairo_set_source_rgba(cr, 0, 1, 0, 1);
  cairo_rectangle(cr, 245, 244, 2, 2);
  cairo_set_line_width(cr, 1);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke(cr);

  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  for (double i = 30; i < 200; i++) {
    cairo_rectangle(cr, i, 25, 1, 1);
    cairo_set_line_width(cr, 1);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke(cr);
  }

  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  int length = g_slist_length(state->brushes);
  printf("drawing %d points\n", length);
  for (GSList *point = state->brushes; point; point = point->next) {
    // struct swappy_brush_point *bp = point->data;
    // printf("drawing rectancle: %lf, %lf, %lf, %lf, %lf, %lf\n", bp->x, bp->y,
    //  bp->r, bp->g, bp->b, bp->a);
    cairo_rectangle(cr, 200, 30, 10, 10);
    cairo_set_line_width(cr, 10);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke(cr);
  }

  if (state->brushes) {
    // GdkRGBA color;
    // cairo_arc(cr, width / 2.0, height / 2.0, MIN(width, height) / 2.0, 0,
    //           2 * G_PI);
    // gtk_style_context_get_color(context,
    // gtk_style_context_get_state(context),
    //                             &color);
    // gdk_cairo_set_source_rgba(cr, &color);

    // cairo_fill(cr);
  }
}

static void draw_area_button_press_handler(GtkWidget *widget, GdkEvent *event,
                                           struct swappy_state *state) {
  printf("button pressed event\n");
}

static void draw_area_button_release_handler(GtkWidget *widget, GdkEvent *event,
                                             struct swappy_state *state) {
  printf("button release event\n");
}

static void draw_area_motion_notify_handler(GtkWidget *widget,
                                            GdkEventMotion *event,
                                            struct swappy_state *state) {
  printf("motion notify event - x: %lf, y: %lf\n", event->x, event->y);

  if (state->is_mode_brush) {
    struct swappy_brush_point *point = g_new(struct swappy_brush_point, 1);
    point->x = event->x;
    point->y = event->y;
    point->r = 1;
    point->g = 1;
    point->b = 1;
    point->a = 1;

    state->brushes = g_slist_append(state->brushes, point);
  }
}

static void build_drawing_area(struct swappy_state *state, GtkWidget *parent) {
  GtkWidget *area = gtk_drawing_area_new();

  gtk_widget_set_size_request(area, state->width, state->height);

  gtk_container_add(GTK_CONTAINER(parent), area);
  gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                                  GDK_BUTTON1_MOTION_MASK);

  g_signal_connect(G_OBJECT(area), "draw", G_CALLBACK(draw_area_handler),
                   state);
  g_signal_connect(G_OBJECT(area), "button-press-event",
                   G_CALLBACK(draw_area_button_press_handler), state);
  g_signal_connect(G_OBJECT(area), "button-release-event",
                   G_CALLBACK(draw_area_button_release_handler), state);
  g_signal_connect(G_OBJECT(area), "motion-notify-event",
                   G_CALLBACK(draw_area_motion_notify_handler), state);
  // GdkCursor *cursor = gdk_cursor_new(GDK_CROSSHAIR);
  // gdk_window_set_cursor(state->window, cursor);
}

// static void destroy_handler(GtkWidget *widget, struct swappy_state *state) {
//   printf("received 'destroy' event\n");

//   application_finish(state);
// }

static void application_activate(GtkApplication *app, void *data) {
  struct swappy_state *state = data;

  // Create a normal GTK window however you like
  GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
  state->window = window;

  // Before the window is first realized, set it up to be a layer surface
  gtk_layer_init_for_window(window);

  // Order above normal windows
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_window_set_default_size(window, 300, 300);
  // gtk_window_resize(window, 400, 400);

  // Need to set keyboard interactivity for key bindings
  gtk_layer_set_keyboard_interactivity(window, true);

  // Push other windows out of the way
  gtk_layer_auto_exclusive_zone_enable(window);

  // The margins are the gaps around the window's edges
  // Margins and anchors can be set like this...
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, 40);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_RIGHT, 40);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, 20);

  gtk_window_get_size(window, &state->width, &state->height);

  printf("window has sizes %dx%d\n", state->width, state->height);

  // Set up a widget
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  // GtkWidget *label = gtk_label_new("");
  // gtk_label_set_markup(GTK_LABEL(label),
  //                      "<span font_desc=\"20.0\">"
  //                      "GTK Layer Shell example!"
  //                      "</span>");
  // GtkWidget *image = gtk_image_new_from_file(state->image);

  build_tools_menu(state, vbox);
  build_drawing_area(state, vbox);

  // gtk_container_add(GTK_CONTAINER(vbox), label);
  // gtk_container_add(GTK_CONTAINER(vbox), image);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width(GTK_CONTAINER(window), 2);
  g_signal_connect(G_OBJECT(window), "key_press_event",
                   G_CALLBACK(keypress_handler), state);
  // g_signal_connect(window, "destroy", G_CALLBACK(destroy_handler), state);
  gtk_widget_show_all(GTK_WIDGET(window));
}

bool application_init(struct swappy_state *state) {
  state->app =
      gtk_application_new("me.jtheoof.swappy", G_APPLICATION_FLAGS_NONE);

  if (state->app == NULL) {
    fprintf(stderr, "cannot create gtk application\n");
  }

  g_signal_connect(state->app, "activate", G_CALLBACK(application_activate),
                   state);

  return true;
}

int application_run(struct swappy_state *state) {
  return g_application_run(G_APPLICATION(state->app), state->argc, state->argv);
}
