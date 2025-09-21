#include <stdlib.h>
#include <string.h>
#include "../include/config.h"
#include "../include/file.h"
#include "../include/utils.h"

#ifdef __linux__
#define CONFIG_LOCATIONS 3
#define CONFIG_DEFAULT 2
const char* config_paths[CONFIG_LOCATIONS] = {"./", "~/", "~/.config/vbdist/"};
#elif _WIN32
#define CONFIG_LOCATIONS 2
#define CONFIG_DEFAULT 1
const char* config_paths[CONFIG_LOCATIONS] = {".\\", "C:\\Users\\Public\\vbdist\\"};
#endif

void set_db_path(config* cfg, const char* path) {
  char* abs_path = absolute_path(path);
  if (cfg->db_path) free(cfg->db_path);
  cfg->db_path = abs_path;
}

void set_log_path(config* cfg, const char* path) {
  char* abs_path = absolute_path(path);
  if (cfg->log_path) free(cfg->log_path);
  cfg->log_path = abs_path;
}

char db_is_set(config* cfg) {
  return cfg->db_path && strcmp(cfg->db_path, "") != 0;
}

void cfg_full_path(char* full, size_t size, const char* base_path) {
  full_path(full, size, base_path, CONFIG_NAME);
}

int find_config(char* path) {
  for (int i = 0; i < CONFIG_LOCATIONS; i++) {
    char full_path[PATH_SIZE];
    char base_path[420];
    strcpy(base_path, config_paths[i]);
    expand_path(base_path);
    cfg_full_path(full_path, PATH_SIZE, base_path);
    if (file_exists(full_path)) {
      strcpy(path, base_path);
      return 1;
    }
  }
  return 0;
}

config* base_config(const char* base_path) {
  config* cfg = malloc(sizeof(config));
  cfg->teams_n = 0;
  cfg->team_size = 0;
  cfg->db_path = NULL;
  cfg->log_path = NULL;
  cfg->config_path = malloc(PATH_SIZE);
  cfg_full_path(cfg->config_path, PATH_SIZE, base_path);
  cfg->created = 0;
  return cfg;
}

config* read_config() {
  char config_path[PATH_SIZE];
  if (!find_config(config_path)) {
    return create_config(config_paths[CONFIG_DEFAULT]);
  }
  config* cfg = base_config(config_path);

  FILE* file = fopen(cfg->config_path, "r");
  if (!file) {
    return cfg;
  }

  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    char key[128], value[512];

    if (sscanf(line, "%127[^=]=%127[^\n]", key, value) == 2) {
      char* val = trimWS(value);
      if (strlen(val) == 0) continue;
      if (strcmp(key, "teams_n") == 0) {
        cfg->teams_n = atoi(val);
      } else if (strcmp(key, "team_size") == 0) {
        cfg->team_size = atoi(val);
      } else if (strcmp(key, "db_path") == 0) {
        if (cfg->db_path) free(cfg->db_path);
        cfg->db_path = strdup(val);
      } else if (strcmp(key, "log_path") == 0) {
        if (cfg->log_path) free(cfg->log_path);
        cfg->log_path = strdup(val);
      }
    }
  }
  fclose(file);
  return cfg;
}

config* create_config(const char* base_path) {
  char path[PATH_SIZE + 1];
  strncpy(path, base_path, PATH_SIZE);
  expand_path(path);
  config* cfg = base_config(path);
  cfg->created = 1;
  make_dir(path);
  write_config(cfg);
  return cfg;
}

void free_config(config* cfg) {
  if (cfg->db_path) free(cfg->db_path);
  if (cfg->config_path) free(cfg->config_path);
  if (cfg->log_path) free(cfg->log_path);
  free(cfg);
}

void write_config(config* cfg) {
  if (!cfg->config_path) return;
  FILE* file = fopen(cfg->config_path, "w");
  if (!file) {
    return;
  }
  fprintf(file, "teams_n=%d\n", cfg->teams_n);
  fprintf(file, "team_size=%d\n", cfg->team_size);
  if (cfg->db_path) fprintf(file, "db_path=%s\n", cfg->db_path);
  if (cfg->log_path) fprintf(file, "log_path=%s\n", cfg->log_path);
  fclose(file);
}

void printCfgLocation(FILE* out) {
  char config_path[420];
  if (find_config(config_path)) {
    fprintf(out, "%s%s\n", config_path, CONFIG_NAME);
  }
}

