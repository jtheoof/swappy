#include "keyboard.h"

#include <gdk/gdk.h>
#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "swappy.h"

struct xkb_us_cache {
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  gboolean initialized;
};

static struct xkb_us_cache xkb_us = {0};

static gboolean ensure_us_keymap() {
  if (xkb_us.initialized) {
    return xkb_us.state != NULL;
  }

  xkb_us.initialized = true;
  xkb_us.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!xkb_us.context) {
    g_warning("failed to create xkb context for physical shortcuts");
    return false;
  }

  struct xkb_rule_names names = {0};
  const gchar *rules = g_getenv("XKB_DEFAULT_RULES");
  const gchar *model = g_getenv("XKB_DEFAULT_MODEL");
  const gchar *variant = g_getenv("XKB_DEFAULT_VARIANT");
  const gchar *options = g_getenv("XKB_DEFAULT_OPTIONS");

  names.rules = (rules && rules[0] != '\0') ? rules : "evdev";
  names.model = (model && model[0] != '\0') ? model : "pc105";
  names.layout = "us";
  names.variant = (variant && variant[0] != '\0') ? variant : NULL;
  names.options = (options && options[0] != '\0') ? options : NULL;
  xkb_us.keymap = xkb_keymap_new_from_names(xkb_us.context, &names,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (!xkb_us.keymap) {
    g_warning("failed to create US keymap for physical shortcuts");
    xkb_context_unref(xkb_us.context);
    xkb_us.context = NULL;
    return false;
  }

  xkb_us.state = xkb_state_new(xkb_us.keymap);
  if (!xkb_us.state) {
    g_warning("failed to create US keymap state for physical shortcuts");
    xkb_keymap_unref(xkb_us.keymap);
    xkb_context_unref(xkb_us.context);
    xkb_us.keymap = NULL;
    xkb_us.context = NULL;
    return false;
  }

  return true;
}

static gboolean is_modifier_keyval(guint keyval) {
  switch (keyval) {
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
    case GDK_KEY_Meta_L:
    case GDK_KEY_Meta_R:
    case GDK_KEY_Super_L:
    case GDK_KEY_Super_R:
    case GDK_KEY_Hyper_L:
    case GDK_KEY_Hyper_R:
    case GDK_KEY_ISO_Level3_Shift:
    case GDK_KEY_Caps_Lock:
    case GDK_KEY_Num_Lock:
    case GDK_KEY_Scroll_Lock:
      return true;
    default:
      return false;
  }
}

static xkb_keysym_t us_keysym_from_event(GdkEventKey *event) {
  if (!ensure_us_keymap()) {
    return XKB_KEY_NoSymbol;
  }

  xkb_keycode_t keycode = event->hardware_keycode;
  if (keycode == 0) {
    return XKB_KEY_NoSymbol;
  }

  return xkb_state_key_get_one_sym(xkb_us.state, keycode);
}

guint keyboard_keysym_for_shortcuts(enum swappy_keyboard_shortcuts mode,
                                    GdkEventKey *event) {
  guint sym = event->keyval;

  if (mode != SWAPPY_KEYBOARD_SHORTCUTS_PHYSICAL) {
    return sym;
  }

  if (is_modifier_keyval(sym)) {
    return sym;
  }

  xkb_keysym_t us_sym = us_keysym_from_event(event);
  if (us_sym != XKB_KEY_NoSymbol) {
    sym = us_sym;
  }

  if (event->state & GDK_SHIFT_MASK) {
    if (sym == GDK_KEY_equal) {
      sym = GDK_KEY_plus;
    }

    sym = gdk_keyval_to_upper(sym);
  } else {
    sym = gdk_keyval_to_lower(sym);
  }

  return sym;
}
