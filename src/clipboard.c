#include "clipboard.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "notification.h"
#include "util.h"

#define gtk_clipboard_t GtkClipboard
#define gdk_pixbuf_t GdkPixbuf

static gboolean send_pixbuf_to_wl_copy(gdk_pixbuf_t *pixbuf) {
  pid_t clipboard_process = 0;
  int pipefd[2];
  int status;
  ssize_t written;
  gsize size;
  gchar *buffer = NULL;
  GError *error = NULL;

  if (pipe(pipefd) < 0) {
    g_warning("unable to pipe for copy process to work");
    return false;
  }
  clipboard_process = fork();
  if (clipboard_process == -1) {
    g_warning("unable to fork process for copy");
    return false;
  }
  if (clipboard_process == 0) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    execlp("wl-copy", "wl-copy", NULL);
    g_warning(
        "Unable to copy contents to clipboard. Please make sure you have "
        "`wl-clipboard`, `xclip`, or `xsel` installed.");
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
  waitpid(clipboard_process, &status, 0);

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status) == 0;  // Make sure the child exited properly
  }

  return false;
}

static void send_pixbuf_to_gdk_clipboard(gdk_pixbuf_t *pixbuf) {
  gtk_clipboard_t *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_image(clipboard, pixbuf);
  gtk_clipboard_store(clipboard);  // Does not work for Wayland gdk backend
}

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  int width = gtk_widget_get_allocated_width(state->ui->area);
  int height = gtk_widget_get_allocated_height(state->ui->area);
  gdk_pixbuf_t *pixbuf =
      gdk_pixbuf_get_from_surface(state->cairo_surface, 0, 0, width, height);

  // Try `wl-copy` first and fall back to gtk function. See README.md.
  if (!send_pixbuf_to_wl_copy(pixbuf)) {
    send_pixbuf_to_gdk_clipboard(pixbuf);
  }

  char message[MAX_PATH];
  snprintf(message, MAX_PATH, "Swappshot copied to clipboard\n");
  notification_send("Swappy", message);

  return true;
}