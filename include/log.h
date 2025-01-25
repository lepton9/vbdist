#ifndef LOG_H
#define LOG_H

#include <time.h>

#define LOG_FILE "log_vbdist.txt"

#define log_with_custom_prefix(prefix, fmt, ...) \
    log_with_prefix(prefix, fmt, __VA_ARGS__)

#define log_sql(fmt, ...) \
    log_with_custom_prefix("SQL", fmt, __VA_ARGS__)

#define log_sql_error(fmt, ...) \
    log_with_custom_prefix("SQL_ERROR", fmt, __VA_ARGS__)

void log_with_prefix(const char *prefix, const char *fmt, ...);
void log(const char* msg);

struct tm* get_timeinfo();
char* timef(struct tm* timeinfo);

#endif
