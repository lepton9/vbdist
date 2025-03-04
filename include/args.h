#ifndef ARGS_H
#define ARGS_H
#include <stdio.h>

typedef struct {
  char *fileName;
  char *dbName;
  int teams;
  int players;
  int printMode;
} args;

void printUsage(FILE *out);
int checkForOption(const char *arg, const char *shortOpt, const char *longOpt);
args *parseArgs(int argc, char **argv);
void freeArgs(args *args);

#endif
