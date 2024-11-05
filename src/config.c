#include "config.h"

#include <glib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wordexp.h>

#include "file.h"
#include "swappy.h"

static void print_config(struct swappy_config *config) {
  g_info("printing config:");
  g_info("config_dir: %s", config->config_file);
  g_info("save_dir: %s", config->save_dir);
  g_info("save_filename_format: %s", config->save_filename_format);
  g_info("show_panel: %d", config->show_panel);
  g_info("line_size: %d", config->line_size);
  g_info("text_font: %s", config->text_font);
  g_info("text_size: %d", config->text_size);
  g_info("paint_mode: %d", config->paint_mode);
  g_info("early_exit: %d", config->early_exit);
  g_info("fill_shape: %d", config->fill_shape);
  g_info("auto_save: %d", config->auto_save);
  g_info("custom_color: %s", config->custom_color);
  g_info("transparent: %d", config->transparent);
}

static char *get_default_save_dir() {
  static const char *storage_paths[] = {
      "$XDG_DESKTOP_DIR",
      "$XDG_CONFIG_HOME/Desktop",
      "$HOME/Desktop",
      "$HOME",
  };

  for (size_t i = 0; i < sizeof(storage_paths) / sizeof(char *); ++i) {
    wordexp_t p;
    if (wordexp(storage_paths[i], &p, 0) == 0) {
      char *path = g_strdup(p.we_wordv[0]);
      wordfree(&p);
      if (path && folder_exists(path)) {
        return path;
      }
      g_free(path);
    }
  }

  return NULL;
}

static char *get_config_file() {
  static const char *storage_paths[] = {
      "$XDG_CONFIG_HOME/swappy/config",
      "$HOME/.config/swappy/config",
  };

  for (size_t i = 0; i < sizeof(storage_paths) / sizeof(char *); ++i) {
    wordexp_t p;
    if (wordexp(storage_paths[i], &p, 0) == 0) {
      char *path = g_strdup(p.we_wordv[0]);
      wordfree(&p);
      if (path && file_exists(path)) {
        return path;
      }
      g_free(path);
    }
  }

  return NULL;
}

static void load_config_from_file(struct swappy_config *config,
                                  const char *file) {
  GKeyFile *gkf;
  const gchar *group = "Default";
  gchar *save_dir = NULL;
  gchar *save_filename_format = NULL;
  gboolean show_panel;
  gchar *save_dir_expanded = NULL;
  guint64 line_size, text_size;
  gchar *text_font = NULL;
  gchar *paint_mode = NULL;
  gboolean early_exit;
  gboolean fill_shape;
  gboolean auto_save;
  gchar *custom_color = NULL;
  gboolean transparent;
  GError *error = NULL;

  if (file == NULL) {
    return;
  }

  gkf = g_key_file_new();

  if (!g_key_file_load_from_file(gkf, file, G_KEY_FILE_NONE, NULL)) {
    g_warning("could not read config file %s", file);
    g_key_file_free(gkf);
    return;
  }

  save_dir = g_key_file_get_string(gkf, group, "save_dir", &error);

  if (error == NULL) {
    wordexp_t p;
    if (wordexp(save_dir, &p, 0) == 0) {
      save_dir_expanded = g_strdup(p.we_wordv[0]);
      wordfree(&p);
      if (!save_dir_expanded || !folder_exists(save_dir_expanded)) {
        g_info("save_dir: attempting to create non-existent directory '%s'",
               save_dir_expanded);
        if (g_mkdir_with_parents(save_dir_expanded, 0755)) {
          g_warning("save_dir: failed to create '%s'", save_dir_expanded);
        }
      }

      g_free(save_dir);
      g_free(config->save_dir);
      config->save_dir = save_dir_expanded;
    }
  } else {
    g_info("save_dir is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  save_filename_format =
      g_key_file_get_string(gkf, group, "save_filename_format", &error);

  if (error == NULL) {
    config->save_filename_format = save_filename_format;
  } else {
    g_info("save_filename_format is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  line_size = g_key_file_get_uint64(gkf, group, "line_size", &error);

  if (error == NULL) {
    if (line_size >= SWAPPY_LINE_SIZE_MIN &&
        line_size <= SWAPPY_LINE_SIZE_MAX) {
      config->line_size = line_size;
    } else {
      g_warning("line_size is not a valid value: %" PRIu64
                " - see man page for details",
                line_size);
    }
  } else {
    g_info("line_size is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  text_size = g_key_file_get_uint64(gkf, group, "text_size", &error);

  if (error == NULL) {
    if (text_size >= SWAPPY_TEXT_SIZE_MIN &&
        text_size <= SWAPPY_TEXT_SIZE_MAX) {
      config->text_size = text_size;
    } else {
      g_warning("text_size is not a valid value: %" PRIu64
                " - see man page for details",
                text_size);
    }
  } else {
    g_info("text_size is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  text_font = g_key_file_get_string(gkf, group, "text_font", &error);

  if (error == NULL) {
    g_free(config->text_font);
    config->text_font = text_font;
  } else {
    g_info("text_font is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  show_panel = g_key_file_get_boolean(gkf, group, "show_panel", &error);

  if (error == NULL) {
    config->show_panel = show_panel;
  } else {
    g_info("show_panel is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  early_exit = g_key_file_get_boolean(gkf, group, "early_exit", &error);

  if (error == NULL) {
    config->early_exit = early_exit;
  } else {
    g_info("early_exit is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  paint_mode = g_key_file_get_string(gkf, group, "paint_mode", &error);

  if (error == NULL) {
    if (g_ascii_strcasecmp(paint_mode, "brush") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_BRUSH;
    } else if (g_ascii_strcasecmp(paint_mode, "text") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_TEXT;
    } else if (g_ascii_strcasecmp(paint_mode, "rectangle") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_RECTANGLE;
    } else if (g_ascii_strcasecmp(paint_mode, "ellipse") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_ELLIPSE;
    } else if (g_ascii_strcasecmp(paint_mode, "arrow") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_ARROW;
    } else if (g_ascii_strcasecmp(paint_mode, "blur") == 0) {
      config->paint_mode = SWAPPY_PAINT_MODE_BLUR;
    } else {
      g_warning(
          "paint_mode is not a valid value: %s - see man page for details",
          paint_mode);
    }
  } else {
    g_info("paint_mode is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  fill_shape = g_key_file_get_boolean(gkf, group, "fill_shape", &error);

  if (error == NULL) {
    config->fill_shape = fill_shape;
  } else {
    g_info("fill_shape is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  auto_save = g_key_file_get_boolean(gkf, group, "auto_save", &error);

  if (error == NULL) {
    config->auto_save = auto_save;
  } else {
    g_info("auto_save is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  custom_color = g_key_file_get_string(gkf, group, "custom_color", &error);

  if (error == NULL) {
    config->custom_color = custom_color;
  } else {
    g_info("custom_color is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  transparent = g_key_file_get_boolean(gkf, group, "transparent", &error);

  if (error == NULL) {
    config->transparent = transparent;
  } else {
    g_info("transparent is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  g_key_file_free(gkf);
}

static void load_default_config(struct swappy_config *config) {
  if (config == NULL) {
    return;
  }

  config->save_dir = get_default_save_dir();
  config->save_filename_format = g_strdup(CONFIG_SAVE_FILENAME_FORMAT_DEFAULT);
  config->line_size = CONFIG_LINE_SIZE_DEFAULT;
  config->text_font = g_strdup(CONFIG_TEXT_FONT_DEFAULT);
  config->text_size = CONFIG_TEXT_SIZE_DEFAULT;
  config->show_panel = CONFIG_SHOW_PANEL_DEFAULT;
  config->paint_mode = CONFIG_PAINT_MODE_DEFAULT;
  config->early_exit = CONFIG_EARLY_EXIT_DEFAULT;
  config->fill_shape = CONFIG_FILL_SHAPE_DEFAULT;
  config->auto_save = CONFIG_AUTO_SAVE_DEFAULT;
  config->custom_color = g_strdup(CONFIG_CUSTOM_COLOR_DEFAULT);
  config->transparent = CONFIG_TRANSPARENT_DEFAULT;
}

void config_load(struct swappy_state *state) {
  struct swappy_config *config = g_new(struct swappy_config, 1);

  load_default_config(config);

  char *file = get_config_file();
  if (file) {
    load_config_from_file(config, file);
  } else {
    g_info("could not find swappy config file, using defaults");
  }

  config->config_file = file;
  state->config = config;

  print_config(state->config);
}

void config_free(struct swappy_state *state) {
  if (state->config) {
    g_free(state->config->config_file);
    g_free(state->config->save_dir);
    g_free(state->config->save_filename_format);
    g_free(state->config->text_font);
    g_free(state->config->custom_color);
    g_free(state->config);
    state->config = NULL;
  }
}
