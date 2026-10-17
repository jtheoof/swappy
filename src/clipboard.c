#include "clipboard.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pixbuf.h"
#include "util.h"

#define gtk_clipboard_t GtkClipboard
#define gdk_pixbuf_t GdkPixbuf

/*
 * Fork a child that execs "swappy --clipboard-serve".  The child reads PNG
 * from stdin and sets the GTK clipboard with gtk_clipboard_set_image(), which
 * advertises every image MIME type GTK supports (image/png, image/bmp,
 * image/tiff, …) — the same behaviour as Pinta or GIMP.  The child stays
 * alive to serve paste requests and exits when another app takes clipboard
 * ownership.
 */
static gboolean send_pixbuf_to_clipboard_serve(gdk_pixbuf_t *pixbuf) {
  pid_t clipboard_process = 0;
  int pipefd[2];
  ssize_t written;
  gsize size;
  gchar *buffer = NULL;
  GError *error = NULL;

  if (pipe(pipefd) < 0) {
    g_warning("unable to pipe for clipboard-serve process");
    return false;
  }
  clipboard_process = fork();
  if (clipboard_process == -1) {
    g_warning("unable to fork process for clipboard-serve");
    return false;
  }
  if (clipboard_process == 0) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    /* Detach so swappy can exit while the child keeps serving clipboard. */
    setsid();
    execlp("swappy", "swappy", "--clipboard-serve", NULL);
    /* Fallback: try wl-copy if swappy --clipboard-serve is unavailable. */
    execlp("wl-copy", "wl-copy", "-t", "image/png", NULL);
    g_warning(
        "Unable to copy contents to clipboard. Please make sure swappy or "
        "`wl-clipboard` is installed.");
    exit(1);
  }
  close(pipefd[0]);

  gdk_pixbuf_save_to_buffer(pixbuf, &buffer, &size, "png", &error, NULL);

  if (error != NULL) {
    g_critical("unable to save pixbuf to buffer for copy: %s", error->message);
    g_error_free(error);
    return false;
  }

  written = write(pipefd[1], buffer, size);
  if (written == -1) {
    g_warning("unable to write to pipe fd for copy");
    g_free(buffer);
    return false;
  }

  close(pipefd[1]);
  g_free(buffer);

  /* Don't waitpid — the child intentionally outlives swappy. */
  return true;
}

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  gdk_pixbuf_t *pixbuf = pixbuf_get_from_state(state);

  if (!send_pixbuf_to_clipboard_serve(pixbuf)) {
    g_warning("clipboard-serve failed, falling back to wl-copy");
  }

  g_object_unref(pixbuf);

  if (state->config->early_exit) {
    gtk_main_quit();
  }

  return true;
}
