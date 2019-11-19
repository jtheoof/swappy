#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>

#include "swappy.h"

static void swappy_overlay_clear(struct swappy_state *state) {
  if (state->brushes) {
    g_slist_free_full(state->brushes, g_free);
    state->brushes = NULL;
  }
}

void application_finish(struct swappy_state *state) {
  printf("calling application_finish\n");
  g_object_unref(state->app);
}

static void tools_menu_button_clear_clicked_handler(
    GtkWidget *widget, struct swappy_state *state) {
  swappy_overlay_clear(state);
  gtk_widget_queue_draw(state->area);
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

static gboolean draw_area_handler(GtkWidget *widget, cairo_t *cr,
                                  struct swappy_state *state) {
  guint width, height;
  GtkStyleContext *context;

  context = gtk_widget_get_style_context(widget);
  width = gtk_widget_get_allocated_width(widget);
  height = gtk_widget_get_allocated_height(widget);

  gtk_render_background(context, cr, 0, 0, width, height);

  // Drawing backgroound
  cairo_save(cr);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke_preserve(cr);
  cairo_fill(cr);
  cairo_restore(cr);

  // Drawing image
  cairo_surface_t *image;
  image = cairo_image_surface_create_from_png(state->image);
  cairo_save(cr);
  cairo_surface_flush(image);
  cairo_surface_mark_dirty(image);
  cairo_set_source_surface(cr, image, 0, 0);
  cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
  cairo_paint(cr);
  cairo_surface_destroy(image);
  cairo_restore(cr);

  for (GSList *point = state->brushes; point; point = point->next) {
    struct swappy_brush_point *bp = point->data;
    cairo_set_source_rgba(cr, bp->r, bp->g, bp->b, bp->a);
    cairo_rectangle(cr, bp->x, bp->y, 1, 1);
    cairo_set_line_width(cr, 1);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke(cr);
  }

  return true;
}

static void brush_add_point(struct swappy_state *state, double x, double y) {
  struct swappy_brush_point *point = g_new(struct swappy_brush_point, 1);
  point->x = x;
  point->y = y;
  point->r = 1;
  point->g = 0;
  point->b = 0;
  point->a = 1;

  state->brushes = g_slist_append(state->brushes, point);
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
    brush_add_point(state, event->x, event->y);
    gtk_widget_queue_draw(state->area);
  }
}

static void build_layout_drawing_area(struct swappy_state *state,
                                      GtkWidget *parent) {
  GtkWidget *area = gtk_drawing_area_new();
  state->area = area;

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
}

static void build_layout_tools_menu(struct swappy_state *state,
                                    GtkWidget *parent) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget *brush = gtk_radio_button_new_with_label(NULL, "Brush");
  GtkWidget *text = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(brush), "Text");
  GtkWidget *clear = gtk_button_new_with_label("Reset");

  gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

  gtk_container_add(GTK_CONTAINER(box), brush);
  gtk_container_add(GTK_CONTAINER(box), text);
  gtk_container_add(GTK_CONTAINER(box), clear);
  gtk_container_add(GTK_CONTAINER(parent), box);

  g_signal_connect(brush, "toggled",
                   G_CALLBACK(tools_menu_button_brush_toggle_handler), state);
  g_signal_connect(text, "toggled",
                   G_CALLBACK(tools_menu_button_text_toggle_handler), state);
  g_signal_connect(clear, "clicked",
                   G_CALLBACK(tools_menu_button_clear_clicked_handler), state);
}

static void build_layout(struct swappy_state *state) {
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  build_layout_tools_menu(state, vbox);
  build_layout_drawing_area(state, vbox);

  gtk_container_add(GTK_CONTAINER(state->window), vbox);
  gtk_container_set_border_width(GTK_CONTAINER(state->window), 2);

  g_signal_connect(G_OBJECT(state->window), "key_press_event",
                   G_CALLBACK(keypress_handler), state);

  gtk_widget_show_all(GTK_WIDGET(state->window));
}

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

  build_layout(state);
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
