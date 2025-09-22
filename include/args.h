#ifndef ARGS_H
#define ARGS_H
#include <stdio.h>

typedef enum {
  ACTION_GENERATE,
  ACTION_HELP,
  ACTION_CONFIG,
  ACTION_VIEWLOG,
  ACTION_ERROR
} action;

typedef struct {
  const char* name_short;
  const char* name_long;
  struct {
    const char* name;
    char required;
  } arg;
  const char* description;
  char required;
} option;

#define OPTIONS_N 8

static const option options[OPTIONS_N] = {
    {"d", "database", {"path", 1}, "Path to sqlite database", 0},
    {"f", "file", {"path", 1}, "Path to textfile", 0},
    {"t", "teams", {"int", 1}, "Set number of teams", 0},
    {"p", "players", {"int", 1}, "Set number of players in a team", 0},
    {"c", "config", {NULL}, "Print config location", 0},
    {"l", "log", {"path", 1}, "Set custom log file path", 0},
    {"v", "viewlog", {"int", 0}, "Print the last rows in log (default: 10)", 0},
    {"h", "help", {NULL}, "Print this help", 0}};

typedef struct {
  char *filePath;
  char *dbPath;
  char *logPath;
  char *err_msg;
  int teams;
  int players;
  size_t viewLogN;
} args;

void printUsage(FILE *out);
void printArgsError(args* args, FILE* out);
int checkForOption(const char *arg, const char *shortOpt, const char *longOpt);
action parseArgs(args* params, int argc, char **argv);
void argsError(args* args, char* msg);
args* initArgs();
void freeArgs(args *args);

#endif
