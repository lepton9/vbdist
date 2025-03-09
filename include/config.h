#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_NAME "vbdist.conf"

typedef struct {
  int teams_n;
  int team_size;
  char db_path[512];
  char config_path[512];
  int created;
} config;

config* read_config();
void write_config(config* cfg);

void expand_path(char* path);
int dir_exists(const char* path);
int file_exists(const char* path);

void make_dir(const char* path);
int find_config(char* path);
config* base_config(const char* base_path);
config* create_config(const char* base_path);

#endif
