#include <stdio.h>
#include <stdlib.h>
#include "../include/log.h"


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

void log(const char* msg) {
  struct tm* timeinfo = get_timeinfo();
  char* tf = timef(timeinfo);
  FILE *log_file = fopen(LOG_FILE, "a");
  if (log_file) {
    fprintf(log_file, "%s | %s\n", tf, msg);
    fclose(log_file);
  }
  free(tf);
}

void log_sql_error(const char* err_msg) {
  char msg[100];
  sprintf(msg, "SQL_ERROR: %s", err_msg);
  log(msg);
}

