#include "../include/args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

args* initArgs() {
  args *params = malloc(sizeof(args));
  memset(params, 0, sizeof(args));
  params->viewLogN = 10;
  return params;
}

void freeArgs(args* args) {
  if (!args) return;
  if (args->dbPath) free(args->dbPath);
  if (args->logPath) free(args->logPath);
  if (args->filePath) free(args->filePath);
  if (args->err_msg) free(args->err_msg);
  free(args);
}

void printUsage(FILE *out) {
  fprintf(out, "Usage: vbdist [options]\n\n");
  fprintf(out, "%*sOptions:\n", 2, "");
  char arg[32];
  for (size_t i = 0; i < OPTIONS_N; i++) {
    const option opt = options[i];
    arg[0] = 0;
    if (opt.arg.name) {
      snprintf(arg, 32, "<%s%s>", (opt.arg.required) ? "" : "?",
               (opt.arg.name) ? opt.arg.name : "");
    }
    fprintf(out, "%*s-%s, --%-10s%-8s%s\n", 4, "", opt.name_short,
            opt.name_long, arg, opt.description);
  }
}

char isOption(const char* arg) {
  if (strlen(arg) >= 1 && arg[0] == '-') return 1;
  return 0;
}

int findOptInd(const char* opt_name, char is_long) {
  if (!opt_name) return -1;
  for (size_t i = 0; i < OPTIONS_N; i++) {
    const option opt = options[i];
    if (is_long ? (strcmp(opt_name, opt.name_long) == 0)
                : (strcmp(opt_name, opt.name_short) == 0))
      return i;
    }
  return -1;
}

const option* findValidOption(const char* flag) {
  if (strlen(flag) < 2 || flag[0] != '-') return NULL;
  char long_name = 0;
  flag++;
  if (flag[0] == '-') {
    flag++;
    long_name = 1;
  }
  int i = findOptInd(flag, long_name);
  if (i < 0) return NULL;
  return &options[i];
}

action parseArgs(args* params, int argc, char **argv) {
  if (!params) return ACTION_ERROR;

  char msg[100];
  int optind;
  for (optind = 1; optind < argc; optind++) {
    char* arg = argv[optind];
    if (!isOption(arg)) {
      snprintf(msg, sizeof(msg), "Invalid argument '%s'. Expected an option\n",
               arg);
      return argsError(params, msg);
    }
    const option* opt = findValidOption(arg);
    if (!opt) {
      snprintf(msg, sizeof(msg), "Invalid option '%s'\n", arg);
      return argsError(params, msg);
    }

    char* next_arg = NULL;
    if (opt->arg.name && optind < argc - 1 && !isOption(argv[optind + 1])) {
      next_arg = argv[++optind];
    }

    if (opt->arg.required && !next_arg) {
      snprintf(msg, sizeof(msg), "Missing argument for option '%s'\n", arg);
      return argsError(params, msg);
    }

    switch (opt->type) {
      case OPT_DB:
        params->dbPath = strdup(next_arg);
        break;
      case OPT_FILE:
        params->filePath = strdup(next_arg);
        break;
      case OPT_TEAMS:
        params->teams = atoi(next_arg);
        break;
      case OPT_PLAYERS:
        params->players = atoi(next_arg);
        break;
      case OPT_CONFIG:
        return ACTION_CONFIG;
      case OPT_LOG:
        params->logPath = strdup(next_arg);
        break;
      case OPT_VIEWLOG:
        if (next_arg) params->viewLogN = atoi(next_arg);
        return ACTION_VIEWLOG;
      case OPT_HELP:
        return ACTION_HELP;
    }
  }
  return ACTION_GENERATE;
}

void printArgsError(args* args, FILE* out) {
  fprintf(out, "%s", (args->err_msg) ? args->err_msg : "");
}

action argsError(args* args, char* msg) {
  if (args->err_msg) free(args->err_msg);
  args->err_msg = strdup(msg);
  return ACTION_ERROR;
}

