#include "pixbuf.h"

#include "notification.h"

void pixbuf_save_to_file(struct swappy_state *state) {
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
  snprintf(path, MAX_PATH, "%s/%s %s.png", state->config->save_dir, "Swappshot",
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
  g_object_unref(pixbuf);
}
