#ifndef ARGS_H
#define ARGS_H
#include <stdio.h>

typedef enum {
  ACTION_GENERATE,
  ACTION_HELP,
  ACTION_ERROR
} action;

typedef struct {
  char *fileName;
  char *dbName;
  char *err_msg;
  int teams;
  int players;
} args;

void printUsage(FILE *out);
void printArgsError(args* args, FILE* out);
int checkForOption(const char *arg, const char *shortOpt, const char *longOpt);
action parseArgs(args* params, int argc, char **argv);
void argsError(args* args, char* msg);
args* initArgs();
void freeArgs(args *args);

#endif
