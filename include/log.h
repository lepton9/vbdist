#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <stdio.h>

#ifdef __linux__
#define DEFAULT_LOG_PATH "~/.local/share/vbdist/logs/"
#elif _WIN32
#define DEFAULT_LOG_PATH "C:\\Users\\Public\\vbdist\\logs\\"
#endif

#define LOG_FILE "vbdist.log"

#define log_with_custom_prefix(prefix, fmt, ...) \
    log_with_prefix(prefix, fmt, __VA_ARGS__)

#define log_log(fmt, ...) \
    log_with_custom_prefix("LOG", fmt, __VA_ARGS__)

#define log_error(fmt, ...) \
    log_with_custom_prefix("ERROR", fmt, __VA_ARGS__)

#define log_debug(fmt, ...) \
    log_with_custom_prefix("DEBUG", fmt, __VA_ARGS__)

#define log_sql(fmt, ...) \
    log_with_custom_prefix("SQL", fmt, __VA_ARGS__)

#define log_sql_error(fmt, ...) \
    log_with_custom_prefix("SQL_ERROR", fmt, __VA_ARGS__)

void log_with_prefix(const char *prefix, const char *fmt, ...);
void log_msg(const char* msg);

struct tm* get_timeinfo();
char* timef(struct tm* timeinfo);

void init_log(char* log_path);
void close_log();
char* get_log_path(char* log_path);
void print_logs(FILE* out, size_t n);

#endif
