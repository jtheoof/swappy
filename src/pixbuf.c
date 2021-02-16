#include "pixbuf.h"

#include <cairo/cairo.h>
#include <gio/gunixoutputstream.h>

#include "notification.h"

GdkPixbuf *pixbuf_get_from_state(struct swappy_state *state) {
  guint width = cairo_image_surface_get_width(state->rendered_surface);
  guint height = cairo_image_surface_get_height(state->rendered_surface);
  GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_surface(state->rendered_surface, 0, 0, width, height);

  return pixbuf;
}

static void write_file(GdkPixbuf *pixbuf, char *path) {
  GError *error = NULL;
  gdk_pixbuf_savev(pixbuf, path, "png", NULL, NULL, &error);

  if (error != NULL) {
    g_critical("unable to save drawing area to pixbuf: %s", error->message);
    g_error_free(error);
  }

  char *msg = "Saved Swappshot to: ";
  size_t len = strlen(msg) + strlen(path) + 1;
  char *message = g_new(char, len);
  g_snprintf(message, len, "%s%s", msg, path);
  notification_send("Swappy", message);
  g_free(message);
}

void pixbuf_save_state_to_folder(GdkPixbuf *pixbuf, char *folder,
                                 char *filename_format) {
  time_t current_time = time(NULL);
  char *c_time_string;
  char filename[strlen(filename_format) + 3];

  c_time_string = ctime(&current_time);
  c_time_string[strlen(c_time_string) - 1] = '\0';
  strftime(filename, sizeof(filename), filename_format,
           localtime(&current_time));
  char path[MAX_PATH];
  g_snprintf(path, MAX_PATH, "%s/%s", folder, filename);
  write_file(pixbuf, path);
}

void pixbuf_save_to_stdout(GdkPixbuf *pixbuf) {
  GOutputStream *out;
  GError *error = NULL;

  out = g_unix_output_stream_new(STDOUT_FILENO, TRUE);

  gdk_pixbuf_save_to_stream(pixbuf, out, "png", NULL, &error, NULL);

  if (error != NULL) {
    g_warning("unable to save surface to stdout: %s", error->message);
    g_error_free(error);
    return;
  }

  g_object_unref(out);
}

GdkPixbuf *pixbuf_init_from_file(struct swappy_state *state) {
  GError *error = NULL;
  char *file =
      state->temp_file_str != NULL ? state->temp_file_str : state->file_str;
  GdkPixbuf *image = gdk_pixbuf_new_from_file(file, &error);

  if (error != NULL) {
    g_error("unable to load file: %s - reason: %s", file, error->message);
    return NULL;
  }

  state->original_image = image;
  return image;
}

void pixbuf_save_to_file(GdkPixbuf *pixbuf, char *file) {
  if (g_strcmp0(file, "-") == 0) {
    pixbuf_save_to_stdout(pixbuf);
  } else {
    write_file(pixbuf, file);
  }
}

void pixbuf_scale_surface_from_widget(struct swappy_state *state,
                                      GtkWidget *widget) {
  GtkAllocation *alloc = g_new(GtkAllocation, 1);
  GdkPixbuf *image = state->original_image;
  gtk_widget_get_allocation(widget, alloc);

  gboolean has_alpha = gdk_pixbuf_get_has_alpha(image);
  cairo_format_t format = has_alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
  gint image_width = gdk_pixbuf_get_width(image);
  gint image_height = gdk_pixbuf_get_height(image);

  cairo_surface_t *scaled_surface =
      cairo_image_surface_create(format, image_width, image_height);

  if (!scaled_surface) {
    g_error("unable to create cairo surface from pixbuf");
    goto finish;
  } else {
    cairo_t *cr;
    cr = cairo_create(scaled_surface);
    gdk_cairo_set_source_pixbuf(cr, image, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
  }

  cairo_surface_t *rendered_surface =
      cairo_image_surface_create(format, image_width, image_height);

  if (!rendered_surface) {
    g_error("unable to create rendering surface");
    goto finish;
  }

  g_info("size of area to render: %ux%u", alloc->width, alloc->height);

finish:
  if (state->scaled_image_surface) {
    cairo_surface_destroy(state->scaled_image_surface);
    state->scaled_image_surface = NULL;
  }
  state->scaled_image_surface = scaled_surface;

  if (state->rendered_surface) {
    cairo_surface_destroy(state->rendered_surface);
    state->rendered_surface = NULL;
  }
  state->rendered_surface = rendered_surface;

  g_free(alloc);
}
