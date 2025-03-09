#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __linux__
#define CONFIG_LOCATIONS 3
#define CONFIG_DEFAULT 2
const char* config_paths[CONFIG_LOCATIONS] = {"./", "~/", "~/.config/vbdist/"};
#else
#define CONFIG_LOCATIONS 2
#define CONFIG_DEFAULT 1
const char* config_paths[CONCONFIG_LOCATIONS] = {"./", "C:\\Users\\Public\\"};
#endif


int file_exists(const char* path) {
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

int find_config(char* path) {
  for (int i = 0; i < CONFIG_LOCATIONS; i++) {
    char full_path[512];
    sprintf(full_path, "%s%s", config_paths[i], CONFIG_NAME);
    if (file_exists(full_path)) {
      strcpy(path, config_paths[i]);
      return 1;
    }
  }
  return 0;
}

config* base_config(const char* base_path) {
  config* cfg = malloc(sizeof(config));
  cfg->teams_n = 0;
  cfg->team_size = 0;
  strcpy(cfg->db_path, "");
  sprintf(cfg->config_path, "%s%s", base_path, CONFIG_NAME);
  cfg->created = 0;
  return cfg;
}

config* read_config() {
  char config_path[512];
  if (!find_config(config_path)) {
    return create_config(config_paths[CONFIG_DEFAULT]);
  }
  sprintf(config_path, "%s%s", config_path, CONFIG_NAME);
  config* cfg = base_config(config_path);

  FILE* file = fopen(cfg->config_path, "r");
  if (!file) {
    return NULL;
  }


  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char key[128], value[128];

    if (sscanf(line, "%127[^=]=%127[^\n]", key, value) == 2) {
      if (strlen(value) == 0) continue;
      if (strcmp(key, "teams_n") == 0) {
        cfg->teams_n = atoi(value);
      } else if (strcmp(key, "team_size") == 0) {
        cfg->team_size = atoi(value);
      } else if (strcmp(key, "db_path") == 0) {
        strncpy(cfg->db_path, value, strlen(value));
      }
    }
  }
  fclose(file);
  return cfg;
}

config* create_config(const char* base_path) {
  config* cfg = base_config(base_path);
  cfg->created = 1;
  // TODO: write base config
  return cfg;
}

