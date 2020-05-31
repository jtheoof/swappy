#include "config.h"

#include <glib.h>
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
  g_info("blur_radius: %d", config->blur_radius);
  g_info("line_size: %d", config->line_size);
  g_info("text_font: %s", config->text_font);
  g_info("text_size: %d", config->text_size);
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
  gchar *save_dir_expanded = NULL;
  guint64 line_size, text_size, blur_radius;
  gchar *text_font = NULL;
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
        g_warning("save_dir: %s is not a valid directory", save_dir_expanded);
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

  line_size = g_key_file_get_uint64(gkf, group, "line_size", &error);

  if (error == NULL) {
    if (line_size >= SWAPPY_LINE_SIZE_MIN &&
        line_size <= SWAPPY_LINE_SIZE_MAX) {
      config->line_size = line_size;
    } else {
      g_warning(
          "line_size is not a valid value: %ld - see man page for details",
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
      g_warning(
          "text_size is not a valid value: %ld - see man page for details",
          text_size);
    }
  } else {
    g_info("text_size is missing in %s (%s)", file, error->message);
    g_error_free(error);
    error = NULL;
  }

  blur_radius = g_key_file_get_uint64(gkf, group, "blur_radius", &error);

  if (error == NULL) {
    if (blur_radius >= SWAPPY_BLUR_RADIUS_MIN &&
        blur_radius <= SWAPPY_BLUR_RADIUS_MAX) {
      config->blur_radius = blur_radius;
    } else {
      g_warning(
          "blur_radius is not a valid value: %ld - see man page for details",
          blur_radius);
    }
  } else {
    g_info("blur_radius is missing in %s (%s)", file, error->message);
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

  g_key_file_free(gkf);
}

static void load_default_config(struct swappy_config *config) {
  if (config == NULL) {
    return;
  }

  config->save_dir = get_default_save_dir();
  config->blur_radius = CONFIG_BLUR_RADIUS_DEFAULT;
  config->line_size = CONFIG_LINE_SIZE_DEFAULT;
  config->text_font = g_strdup(CONFIG_TEXT_FONT_DEFAULT);
  config->text_size = CONFIG_TEXT_SIZE_DEFAULT;
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
    g_free(state->config->text_font);
    g_free(state->config);
    state->config = NULL;
  }
}
