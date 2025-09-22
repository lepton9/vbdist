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

typedef enum {
  OPT_DB,
  OPT_FILE,
  OPT_TEAMS,
  OPT_PLAYERS,
  OPT_CONFIG,
  OPT_LOG,
  OPT_VIEWLOG,
  OPT_HELP,
} opt_type;

typedef struct {
  opt_type type;
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
    {OPT_DB, "d", "database", {"path", 1}, "Path to sqlite database", 0},
    {OPT_FILE, "f", "file", {"path", 1}, "Path to textfile", 0},
    {OPT_TEAMS, "t", "teams", {"int", 1}, "Set number of teams", 0},
    {OPT_PLAYERS, "p", "players", {"int", 1}, "Set number of players in a team", 0},
    {OPT_CONFIG, "c", "config", {NULL}, "Print config location", 0},
    {OPT_LOG, "l", "log", {"path", 1}, "Set custom log file path", 0},
    {OPT_VIEWLOG, "v", "viewlog", {"int", 0}, "Print the last rows in log (default: 10)", 0},
    {OPT_HELP, "h", "help", {NULL}, "Print this help", 0}};

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
action argsError(args* args, char* msg);
args* initArgs();
void freeArgs(args *args);

#endif
