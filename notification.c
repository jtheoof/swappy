#include "notification.h"

#include <libnotify/notify.h>

void notification_send(char *title, char *message) {
  notify_init("Hello world!");
  NotifyNotification *notification =
      notify_notification_new(title, message, "dialog-information");
  notify_notification_show(notification, NULL);
  g_object_unref(G_OBJECT(notification));
  notify_uninit();
}