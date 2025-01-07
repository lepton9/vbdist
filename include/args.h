#ifndef ARGS_H
#define ARGS_H
#include <stdio.h>

typedef struct {
  char *fileName;
  char *dbName;
  int teams;
  int players;
  int pMode;
} args;

void printUsage(FILE *out);
args *parseArgs(int argc, char **argv);

#endif
