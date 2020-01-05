#include "clipboard.h"

#include "notification.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"

#define gdk_pixbuf_t GdkPixbuf

static void data_control_data_offer(
    void *data, struct zwlr_data_control_device_v1 *zwlr_data_control_device_v1,
    struct zwlr_data_control_offer_v1 *id) {
  g_debug("data control data offer");
}

static void data_control_selection(
    void *data, struct zwlr_data_control_device_v1 *zwlr_data_control_device_v1,
    struct zwlr_data_control_offer_v1 *id) {
  g_debug("data control selection");
}

static void data_control_primary_selection(
    void *data, struct zwlr_data_control_device_v1 *zwlr_data_control_device_v1,
    struct zwlr_data_control_offer_v1 *id) {
  g_debug("data control primary selection");
}

static void data_control_finished(
    void *data,
    struct zwlr_data_control_device_v1 *zwlr_data_control_device_v1) {
  g_debug("data control_finished");
}

static void send_pixbuf_to_gdk_clipboard(gdk_pixbuf_t *pixbuf) {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  gtk_clipboard_set_image(clipboard, pixbuf);
  gtk_clipboard_store(clipboard);  // Does not work for Wayland gdk backend
}

struct zwlr_data_control_device_v1_listener listener = {
    .data_offer = data_control_data_offer,
    .selection = data_control_selection,
    .primary_selection = data_control_primary_selection,
    .finished = data_control_finished,
};

void send_pixbuf_to_data_control_clipboard(struct swappy_state *state,
                                           gdk_pixbuf_t *pixbuf) {
  struct zwlr_data_control_manager_v1 *zwlr_data_control_manager_v1 =
      state->wl->zwlr_data_control_manager;

  struct swappy_wl_seat *seat;
  wl_list_for_each(seat, &state->wl->seats, link) {
    struct zwlr_data_control_device_v1 *zwlr_data_control_device_v1 =
        zwlr_data_control_manager_v1_get_data_device(
            zwlr_data_control_manager_v1, seat->wl_seat);

    zwlr_data_control_device_v1_add_listener(zwlr_data_control_device_v1,
                                             &listener, pixbuf);
  }
}

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state) {
  int width = gtk_widget_get_allocated_width(state->ui->area);
  int height = gtk_widget_get_allocated_height(state->ui->area);
  gdk_pixbuf_t *pixbuf =
      gdk_pixbuf_get_from_surface(state->cairo_surface, 0, 0, width, height);
  send_pixbuf_to_gdk_clipboard(pixbuf);
  char message[MAX_PATH];
  snprintf(message, MAX_PATH, "Swappshot copied to clipboard\n");
  notification_send("Swappy", message);

  return true;
}