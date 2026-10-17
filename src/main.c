#define _POSIX_C_SOURCE 200809L

#include <string.h>

#include "application.h"
#include "config.h"
#include "swappy.h"

/*
 * --clipboard-serve mode: read PNG from stdin, set the GTK clipboard with
 * gtk_clipboard_set_image() (which advertises all image MIME types just like
 * Pinta/GIMP), and stay alive to serve paste requests.  Exit when another app
 * takes clipboard ownership.
 */
static void on_owner_change(GtkClipboard *clipboard, GdkEvent *event,
                            gpointer data) {
  (void)clipboard;
  (void)event;
  (void)data;
  gtk_main_quit();
}

static int clipboard_serve(void) {
  GdkPixbuf *pixbuf;
  GdkPixbufLoader *loader;
  GtkClipboard *clipboard;
  guchar buf[65536];
  ssize_t n;
  GError *error = NULL;

  gtk_init(NULL, NULL);

  loader = gdk_pixbuf_loader_new_with_type("png", &error);
  if (!loader) {
    g_printerr("clipboard-serve: cannot create PNG loader: %s\n",
               error->message);
    g_error_free(error);
    return 1;
  }

  while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    if (!gdk_pixbuf_loader_write(loader, buf, (gsize)n, &error)) {
      g_printerr("clipboard-serve: loader write error: %s\n", error->message);
      g_error_free(error);
      g_object_unref(loader);
      return 1;
    }
  }

  if (!gdk_pixbuf_loader_close(loader, &error)) {
    g_printerr("clipboard-serve: loader close error: %s\n", error->message);
    g_error_free(error);
    g_object_unref(loader);
    return 1;
  }

  pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
  if (!pixbuf) {
    g_printerr("clipboard-serve: no pixbuf from loader\n");
    g_object_unref(loader);
    return 1;
  }
  g_object_ref(pixbuf);
  g_object_unref(loader);

  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_image(clipboard, pixbuf);

  /* Exit when another app takes clipboard ownership. */
  g_signal_connect(clipboard, "owner-change", G_CALLBACK(on_owner_change),
                   NULL);

  gtk_main();

  g_object_unref(pixbuf);
  return 0;
}

int main(int argc, char *argv[]) {
  /* Handle --clipboard-serve before any swappy init. */
  if (argc == 2 && strcmp(argv[1], "--clipboard-serve") == 0) {
    return clipboard_serve();
  }

  struct swappy_state state = {0};
  int status;

  state.argc = argc;
  state.argv = argv;
  state.mode = SWAPPY_PAINT_MODE_BRUSH;

  if (!application_init(&state)) {
    g_critical("failed to initialize gtk application");
    exit(1);
  }

  status = application_run(&state);

  if (status == 0) {
    gtk_main();
  }

  application_finish(&state);

  return status;
}
