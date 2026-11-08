#include "clipboard.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pixbuf.h"
#include "util.h"

#define gtk_clipboard_t GtkClipboard
#define gdk_pixbuf_t GdkPixbuf

// The image swappy holds on the clipboard while it owns it. Set back to NULL
// when another application takes the clipboard (see on_clear).
static gdk_pixbuf_t *owned_pixbuf = NULL;

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
    execlp("wl-copy", "wl-copy", "-t", "image/png", NULL);
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

static void on_get(gtk_clipboard_t *clipboard, GtkSelectionData *selection_data,
                   guint info, gpointer data) {
  gtk_selection_data_set_pixbuf(selection_data, GDK_PIXBUF(data));
}

static void on_clear(gtk_clipboard_t *clipboard, gpointer data) {
  if (data == owned_pixbuf) {
    owned_pixbuf = NULL;
  }
  g_object_unref(data);
}

// Like gtk_target_list_add_image_targets(), but only with formats that can
// hold a screenshot:
// - ICO is skipped: it is limited to 256x256 pixels, so converting to it fails.
// - Only image/ MIME types are kept. ICO also registers text/ico, which
//   clients looking for text pick first: `wl-paste` without `-t` got nothing.
// - Duplicates are skipped: several loaders can register the same format.
// PNG comes first, as the default for clients that don't ask for a type.
static GtkTargetList *image_target_list_new(void) {
  GtkTargetList *list = gtk_target_list_new(NULL, 0);
  GSList *formats = gdk_pixbuf_get_formats();

  gtk_target_list_add(list, gdk_atom_intern_static_string("image/png"), 0, 0);

  for (GSList *l = formats; l != NULL; l = l->next) {
    GdkPixbufFormat *format = l->data;
    gchar *name = gdk_pixbuf_format_get_name(format);
    gboolean skip =
        !gdk_pixbuf_format_is_writable(format) || g_strcmp0(name, "ico") == 0;

    g_free(name);
    if (skip) {
      continue;
    }

    gchar **mime_types = gdk_pixbuf_format_get_mime_types(format);
    for (gchar **mime_type = mime_types; *mime_type != NULL; mime_type++) {
      GdkAtom target = gdk_atom_intern(*mime_type, FALSE);
      if (g_str_has_prefix(*mime_type, "image/") &&
          !gtk_target_list_find(list, target, NULL)) {
        gtk_target_list_add(list, target, 0, 0);
      }
    }
    g_strfreev(mime_types);
  }

  g_slist_free(formats);
  return list;
}

// Offer the image in every format that can hold it (image/png, image/bmp,
// image/tiff, ...), like Pinta or GIMP do. The selection is set from swappy's
// own window, which has focus when the user copies, so every compositor accepts
// it.
static gboolean send_pixbuf_to_gtk_clipboard(gdk_pixbuf_t *pixbuf) {
  gtk_clipboard_t *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTargetList *list = image_target_list_new();
  GtkTargetEntry *targets;
  gint count = 0;
  gboolean done;

  targets = gtk_target_table_new_from_list(list, &count);
  gtk_target_list_unref(list);

  // Replacing our own earlier copy calls on_clear for it first.
  done = gtk_clipboard_set_with_data(clipboard, targets, count, on_get,
                                     on_clear, g_object_ref(pixbuf));
  gtk_target_table_free(targets, count);

  if (!done) {
    g_object_unref(pixbuf);
    return false;
  }

  owned_pixbuf = pixbuf;
  gtk_clipboard_set_can_store(clipboard, NULL, 0);

  return true;
}

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  gdk_pixbuf_t *pixbuf = pixbuf_get_from_state(state);

  // Hold the image on the GTK clipboard while swappy runs, and hand it to
  // `wl-copy` when it exits (see clipboard_hand_off_selection). See README.md.
  if (!send_pixbuf_to_gtk_clipboard(pixbuf)) {
    send_pixbuf_to_wl_copy(pixbuf);
  }

  g_object_unref(pixbuf);

  if (state->config->early_exit) {
    gtk_main_quit();
  }

  return true;
}

void clipboard_hand_off_selection(void) {
  gdk_pixbuf_t *pixbuf = owned_pixbuf;

  // Another application took the clipboard, or nothing was copied.
  if (pixbuf == NULL) {
    return;
  }

  // GTK cannot keep the clipboard once swappy exits on Wayland: hand the image
  // to `wl-copy`, as PNG, so it stays on the clipboard.
  g_object_ref(pixbuf);
  if (!send_pixbuf_to_wl_copy(pixbuf)) {
    // Only X11 clipboard managers can store it.
    gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
  }
  g_object_unref(pixbuf);
}
