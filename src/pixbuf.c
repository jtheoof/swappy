#include "pixbuf.h"

#include <gio/gunixoutputstream.h>

#include "notification.h"

GdkPixbuf *pixbuf_get_from_state(struct swappy_state *state) {
  guint width = gtk_widget_get_allocated_width(state->ui->area);
  guint height = gtk_widget_get_allocated_height(state->ui->area);
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_surface(state->cairo_surface, 0, 0, width, height);

  return pixbuf;
}

void pixbuf_save_state_to_folder(GdkPixbuf *pixbuf, char *folder) {
  GError *error = NULL;

  time_t current_time;
  char *c_time_string;

  time(&current_time);

  c_time_string = ctime(&current_time);
  c_time_string[strlen(c_time_string) - 1] = '\0';
  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "%s/%s %s.png", folder, "Swappshot", c_time_string);
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

void pixbuf_save_to_stdout(GdkPixbuf *pixbuf) {
  GOutputStream *out;
  GError *error = NULL;

  out = g_unix_output_stream_new(STDOUT_FILENO, TRUE);

  gdk_pixbuf_save_to_stream(pixbuf, out, "png", NULL, &error);

  if (error != NULL) {
    g_warning("unable to save surface to stdout: %s", error->message);
    g_error_free(error);
    return;
  }

  g_object_unref(out);
}

void pixbuf_save_to_file(GdkPixbuf *pixbuf, char *file) {
  if (g_strcmp0(file, "-") == 0) {
    pixbuf_save_to_stdout(pixbuf);
  } else {
    pixbuf_save_to_file(pixbuf, file);
  }
}