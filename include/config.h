#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#define CONFIG_NAME "vbdist.conf"

typedef struct {
  int teams_n;
  int team_size;
  char* db_path;
  char* config_path;
  char* log_path;
  int created;
} config;

config* read_config();
void free_config(config* cfg);
void write_config(config* cfg);
void printCfgLocation(FILE* out);

void set_db_path(config* cfg, const char* path);
char db_is_set(config* cfg);

int find_config(char* path);
config* base_config(const char* base_path);
config* create_config(const char* base_path);

#endif
