#include "../include/args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printUsage(FILE *out) {
  fprintf(out, "Usage: vbdist [options]\n"
               "  Options:\n"
               "    -f <file>      Path to textfile\n"
               "    -d <database>  Path to sqlite database\n"
               "    -t <int>       Set number of teams\n"
               "    -p <int>       Set number of players in a team\n"
               "    -m <int>       Set print mode (Optional, default minimal)\n"
               "    -h             Display help\n"
               "\nUses a file or database to store and retrieve data based on "
               "which option is set.\n");
}

args *parseArgs(int argc, char **argv) {
  args *params = malloc(sizeof(args));

  size_t optind;
  for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
    if (strlen(argv[optind]) != 2)
      continue;
    switch (argv[optind][1]) {
    case 'f': // Textfile name if using file
      params->fileName = argv[++optind];
      break;
    case 'd': // Database name if using db
      params->dbName = argv[++optind];
      break;
    case 't': // Amount of teams
      params->teams = atoi(argv[++optind]);
      break;
    case 'p': // Players in team
      params->players = atoi(argv[++optind]);
      break;
    case 'm': // Print mode
      params->printMode = atoi(argv[++optind]);
      break;
    }
  }
  return params;
}
