#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../include/log.h"
#include "../include/file.h"

static FILE* log_file = NULL;

void init_log(char* log_path) {
  log_file = fopen(log_path, "a");
  if (!log_file) {
    log_file = stderr;
    log_error("Failed to open log file: '%s'\n", log_path);
  }
}

void close_log() {
  if (log_file && log_file != stderr) {
    fclose(log_file);
  }
}

char* get_log_path(char* log_path) {
  char* path = malloc(PATH_SIZE);
  if (log_path) {
    expand_path(log_path);
    make_dir(log_path);
    snprintf(path, PATH_SIZE, "%s", log_path);
  } else {
    char base_path[PATH_SIZE + 1];
    strncpy(base_path, DEFAULT_LOG_PATH, PATH_SIZE);
    expand_path(base_path);
    make_dir(base_path);
    full_path(path, PATH_SIZE, base_path, LOG_FILE);
  }
  return path;
}

void log_with_prefix(const char *prefix, const char *fmt, ...) {
    char msg[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    char full_msg[300];
    snprintf(full_msg, sizeof(full_msg), "[%s] %s", prefix, msg);
    log_msg(full_msg);
}

struct tm* get_timeinfo() {
  time_t t = time(NULL);
  struct tm* tm = localtime(&t);
  return tm;
}

char *timef(struct tm *tm) {
  char *time_f = malloc(100);
  sprintf(time_f, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
          tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  return time_f;
}

void log_msg(const char* msg) {
  struct tm* timeinfo = get_timeinfo();
  char* tf = timef(timeinfo);
  fprintf(log_file, "[%s] %s\n", tf, msg);
  free(tf);
}

