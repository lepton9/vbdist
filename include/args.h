#ifndef ARGS_H
#define ARGS_H

typedef struct {
  char* fileName;
  char* dbName;
  int teams;
  int players;
  int pMode;
} args;

args* parseArgs(int argc, char** argv);

#endif
