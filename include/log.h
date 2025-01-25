#ifndef LOG_H
#define LOG_H

#include <time.h>

#define LOG_FILE "log_vbdist.txt"

struct tm* get_timeinfo();
char* timef(struct tm* timeinfo);
void log(const char* msg);
void log_sql_error(const char* msg);

#endif
