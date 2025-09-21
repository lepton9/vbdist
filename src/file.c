#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include "../include/file.h"

void full_path(char* path, size_t size, const char* base_path, const char* filename) {
  snprintf(path, size, "%s%s", base_path, filename);
}

char* absolute_path(const char* path) {
  char* abs_path = (char*)malloc(PATH_MAX);
  realpath(path, abs_path);
  return abs_path;
}

int file_exists(const char* path) {
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

int dir_exists(const char* path) {
  struct stat info;
  return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

void expand_path(char* path) {
#ifdef __linux__
  char temp[PATH_SIZE];
  if (path[0] == '~') {
    const char* home = getenv("HOME");
    if (!home) {
      return;
    }
    snprintf(temp, sizeof(temp), "%s%s", home, path + 1);
    strcpy(path, temp);
  }
#endif
}

void make_dir(const char* path) {
  char temp[PATH_SIZE];
  char* p = NULL;
  size_t len;
  strcpy(temp, path);
  len = strlen(temp);
  if (temp[len - 1] == PATH_SEPARATOR) temp[len - 1] = '\0';
  for (p = temp + 1; *p; p++) {
    if (*p == PATH_SEPARATOR) {
      *p = '\0';
      if (!dir_exists(temp)) mkdir(temp, 0755);
      *p = PATH_SEPARATOR;
    }
  }
  if (!dir_exists(temp)) mkdir(temp, 0755);
}

