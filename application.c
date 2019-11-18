#include <gtk-layer-shell/gtk-layer-shell.h>
#include <gtk/gtk.h>

#include "swappy.h"

void tools_menu_button_brush_toggle_handler(GtkToggleButton *source,
                                            gpointer user_data) {
  printf("brush toggled: %d\n", gtk_toggle_button_get_active(source));
}

void tools_menu_button_text_toggle_handler(GtkToggleButton *source,
                                           gpointer user_data) {
  printf("text toggled: %d\n", gtk_toggle_button_get_active(source));
}

gboolean keypress_handler(GtkWidget *widget, GdkEventKey *event,
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

void build_tools_menu(struct swappy_state *state, GtkWidget *parent) {
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

void application_activate(GtkApplication *app, void *data) {
  struct swappy_state *state = data;

  // Create a normal GTK window however you like
  GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
  state->window = window;

  // Before the window is first realized, set it up to be a layer surface
  gtk_layer_init_for_window(window);

  // Order above normal windows
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);

  // Need to set keyboard interactivity for key bindings
  gtk_layer_set_keyboard_interactivity(window, true);

  // Push other windows out of the way
  gtk_layer_auto_exclusive_zone_enable(window);

  // The margins are the gaps around the window's edges
  // Margins and anchors can be set like this...
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, 40);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_RIGHT, 40);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, 20);
  // gtk_layer_set_margin (window, GTK_LAYER_SHELL_EDGE_BOTTOM, 0); // 0 is
  // default

  // ... or like this
  // Anchors are if the window is pinned to each edge of the output
  // static const gboolean anchors[] = {TRUE, TRUE, FALSE, TRUE};
  // for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
  //   gtk_layer_set_anchor(window, i, anchors[i]);
  // }

  // Set up a widget
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  // GtkWidget *label = gtk_label_new("");
  // gtk_label_set_markup(GTK_LABEL(label),
  //                      "<span font_desc=\"20.0\">"
  //                      "GTK Layer Shell example!"
  //                      "</span>");
  GtkWidget *image = gtk_image_new_from_file("/home/jattali/Desktop/sway.jpg");

  build_tools_menu(state, vbox);

  // gtk_container_add(GTK_CONTAINER(vbox), label);
  gtk_container_add(GTK_CONTAINER(vbox), image);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width(GTK_CONTAINER(window), 2);
  g_signal_connect(G_OBJECT(window), "key_press_event",
                   G_CALLBACK(keypress_handler), state);
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

void application_finish(struct swappy_state *state) {
  g_object_unref(state->app);
}