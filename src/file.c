#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_SIZE 1024

bool folder_exists(const char *path) {
  return g_file_test(path, G_FILE_TEST_IS_DIR);
}

bool file_exists(const char *path) {
  return g_file_test(path, G_FILE_TEST_EXISTS);
}

char *file_dump_stdin_into_a_temp_file() {
  char buf[BLOCK_SIZE];
  GError *error = NULL;

  if (isatty(STDIN_FILENO)) {
    g_warning("stdin is a tty");
    return NULL;
  }

  // Reopen stdin as binary mode
  FILE *input_file = g_freopen(NULL, "rb", stdin);

  if (!input_file) {
    g_warning("unable to reopen stdin in binary mode: %s", g_strerror(errno));
    return NULL;
  }

  const gchar *tempdir = g_get_tmp_dir();
  gchar filename[] = "swappy-stdin-XXXXXX.png";
  gchar *ret = g_build_filename(tempdir, filename, NULL);
  gint fd = g_mkstemp(ret);

  if (fd == -1) {
    g_warning("unable to dump stdin into temporary file");
    return NULL;
  }

  g_info("writing stdin content into filepath: %s", ret);

  size_t count = 1;
  while (count > 0) {
    count = fread(buf, 1, sizeof(buf), stdin);
    if (write(fd, &buf, count) == -1) {
      g_warning("error while writing stdin to temporary file: %s - %s", ret,
                g_strerror(errno));
    }
  }

  g_close(fd, &error);

  if (error) {
    g_warning("unable to close temporary file: %s", error->message);
    g_error_free(error);
    return NULL;
  }

  return ret;
}