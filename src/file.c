#define _POSIX_C_SOURCE 200112L

#include <stdbool.h>
#include <sys/stat.h>

bool folder_exists(const char *path) {
  struct stat sb;

  stat(path, &sb);

  return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
}

bool file_exists(const char *path) {
  struct stat sb;

  stat(path, &sb);

  return (stat(path, &sb) == 0 && S_ISREG(sb.st_mode));
}
