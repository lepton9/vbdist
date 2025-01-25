#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/log.h"

void log_with_prefix(const char *prefix, const char *fmt, ...) {
    char msg[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    char full_msg[300];
    snprintf(full_msg, sizeof(full_msg), "%s: %s", prefix, msg);
    log_msg(full_msg);
}

struct tm* get_timeinfo() {
  time_t t = time(NULL);
  struct tm* tm = localtime(&t);
  return tm;
}

char *timef(struct tm *tm) {
  char *time_f = malloc(20);
  sprintf(time_f, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
          tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  return time_f;
}

void log_msg(const char* msg) {
  struct tm* timeinfo = get_timeinfo();
  char* tf = timef(timeinfo);
  FILE *log_file = fopen(LOG_FILE, "a");
  if (log_file) {
    fprintf(log_file, "%s | %s\n", tf, msg);
    fclose(log_file);
  }
  free(tf);
}

