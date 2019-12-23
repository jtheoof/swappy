#include "clipboard.h"

#include "notification.h"

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  g_debug("generating pixbuf from area");
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  guint width = gtk_widget_get_allocated_width(state->area);
  guint height = gtk_widget_get_allocated_height(state->area);
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_surface(state->cairo_surface, 0, 0, width, height);

  gtk_clipboard_set_image(clipboard, pixbuf);

  char message[MAX_PATH];
  snprintf(message, MAX_PATH, "Swappshot copied to clipboard\n");
  notification_send("Swappy", message);

  return true;
}