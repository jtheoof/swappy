#include "clipboard.h"

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(state->area));
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_window(window, 0, 0, state->width, state->height);

  gtk_clipboard_set_image(clipboard, pixbuf);

  return true;
}