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
               "    -m, --mode <int>           Set print mode 0-2 (Optional, default minimal)\n"
               "    -h, --help                 Display help\n"
               "\nUses a file or database to store and retrieve data based on "
               "which option is set.\n");
}

void printUsageVerbose(FILE *out) {
  printUsage(out);
  fprintf(out, "\nExample usage:\n"
               "  Using a text file:\n"
               "    vbdist -f players.txt -t 4 -p 6 -m 1\n"
               "  Using a database:\n"
               "    vbdist -d sql.db -f players.txt -t 4 -p 6 -m 1\n"
               "");
}

args *parseArgs(int argc, char **argv) {
  args *params = malloc(sizeof(args));
  memset(params, 0, sizeof(args));

  int optind;
  for (optind = 1; optind < argc; optind++) {
    char* arg = argv[optind];

    if (strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0) {
      params->fileName = argv[++optind];
    } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--database") == 0) {
      params->dbName = argv[++optind];
    } else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--teams") == 0) {
      params->teams = atoi(argv[++optind]);
    } else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--players") == 0) {
      params->players = atoi(argv[++optind]);
    } else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--mode") == 0) {
      params->printMode = atoi(argv[++optind]);
    } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      free(params);
      printUsageVerbose(stdout);
      return NULL;
    } else {
      free(params);
      printf("Invalid option `%s`\n", arg);
      printf("See `vbdist --help` for more.\n\n");
      printUsage(stdout);
      return NULL;
    }
  }
  return params;
}
