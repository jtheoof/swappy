#include "clipboard.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pixbuf.h"

#define gtk_clipboard_t GtkClipboard
#define gdk_pixbuf_t GdkPixbuf

static gboolean send_to_wl_copy(gchar *buffer, gsize size) {
  pid_t clipboard_process = 0;
  int pipefd[2];
  int status;
  ssize_t written;

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
    execlp("wl-copy", "wl-copy", "-t", "image/png", NULL);
    g_warning(
        "Unable to copy contents to clipboard. Please make sure you have "
        "`wl-clipboard`, `xclip`, or `xsel` installed.");
    exit(1);
  }
  close(pipefd[0]);

  written = write(pipefd[1], buffer, size);
  if (written == -1) {
    g_warning("unable to write to pipe fd for copy");
    g_free(buffer);
    return false;
  }

  close(pipefd[1]);
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
  gdk_pixbuf_t *pixbuf = pixbuf_get_from_state(state);
  gchar *buffer = NULL;
  gsize size;
  GError *error = NULL;
  gdk_pixbuf_save_to_buffer(pixbuf, &buffer, &size, "png", &error, NULL);

  if (error != NULL) {
    g_critical("unable to save pixbuf to buffer for copy: %s", error->message);
    g_error_free(error);
    return false;
  }

  // Try `wl-copy` first and fall back to gtk function. See README.md.
  if (!send_to_wl_copy(buffer, size)) {
    send_pixbuf_to_gdk_clipboard(pixbuf);
  }

  g_free(buffer);
  g_object_unref(pixbuf);

  if (state->config->early_exit) {
    gtk_main_quit();
  }

  return true;
}
bool clipboard_copy_text_to_selection(struct swappy_state *state) {
  GtkTextIter start, end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(state->ui->ocr_text);
  gchar *text_to_copy;
  gsize size;

  if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
    text_to_copy = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  } else {
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &start);
    text_to_copy = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  }
  /* disable selection */
  gtk_text_buffer_select_range(buffer, &start, &start);

  if (text_to_copy != NULL) {
    size = strlen(text_to_copy);
    send_to_wl_copy(text_to_copy, size);
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, text_to_copy, -1);
    g_free(text_to_copy);
  }
  return true;
}
bool clipboard_copy_to_selection(struct swappy_state *state) {
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(state->ui->ocr_text);
  GtkTextIter start, end;
  if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
    return clipboard_copy_text_to_selection(state);
  }
  return clipboard_copy_drawing_area_to_selection(state);
}
