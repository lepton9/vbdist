#include "../include/args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printUsage(FILE *out) {
  fprintf(out, "Usage: vbdist [options]\n"
               "  Options:\n"
               "    -f, --file <file>          Path to textfile\n"
               "    -d, --database <database>  Path to sqlite database\n"
               "    -t, --teams <int>          Set number of teams\n"
               "    -p, --players <int>        Set number of players in a team\n"
               "    -h, --help                 Display help\n"
               "\nUses a file or database to store and retrieve data based on "
               "which option is set.\n");
}

int checkForOption(const char *arg, const char *shortOpt, const char *longOpt) {
  return (strcmp(arg, shortOpt) == 0) || (strcmp(arg, longOpt) == 0);
}

args *parseArgs(int argc, char **argv) {
  args *params = malloc(sizeof(args));
  memset(params, 0, sizeof(args));

  int optind;
  for (optind = 1; optind < argc; optind++) {
    char* arg = argv[optind];
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      free(params);
      printUsage(stdout);
      return NULL;
    }
    if (checkForOption(arg, "-f", "--file")) {
      if (optind == argc - 1 || argv[optind + 1][0] == '-') continue;
      params->fileName = strdup(argv[++optind]);
    } else if (checkForOption(arg, "-d", "--database")) {
      if (optind == argc - 1 || argv[optind + 1][0] == '-') continue;
      params->dbName = strdup(argv[++optind]);
    } else if (checkForOption(arg, "-t", "--teams")) {
      if (optind == argc - 1 || argv[optind + 1][0] == '-') continue;
      params->teams = atoi(argv[++optind]);
    } else if (checkForOption(arg, "-p", "--players")) {
      if (optind == argc - 1 || argv[optind + 1][0] == '-') continue;
      params->players = atoi(argv[++optind]);
    } else {
      printf("Invalid option `%s`\n", arg);
      printf("See `vbdist --help` for more.\n\n");
      freeArgs(params);
      return NULL;
    }
  }
  return params;
}

void freeArgs(args* args) {
  if (!args) return;
  if (args->dbName) free(args->dbName);
  if (args->fileName) free(args->fileName);
  free(args);
}

