#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __linux__
#define CONFIG_LOCATIONS 3
#define CONFIG_DEFAULT 2
#define PATH_SEPARATOR '/'
const char* config_paths[CONFIG_LOCATIONS] = {"./", "~/", "~/.config/vbdist/"};
#else
#include <windows.h>
#define mkdir(dir, mode) _mkdir(dir)
#define CONFIG_LOCATIONS 2
#define CONFIG_DEFAULT 1
#define PATH_SEPARATOR '\\'
const char* config_paths[CONCONFIG_LOCATIONS] = {".\\", "C:\\Users\\Public\\vbdist\\"};
#endif


void cfg_full_path(char* full, const char* base_path) {
  sprintf(full, "%s%s", base_path, CONFIG_NAME);
}

int file_exists(const char* path) {
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

int dir_exists(const char* path) {
  struct stat info;
  return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

int find_config(char* path) {
  for (int i = 0; i < CONFIG_LOCATIONS; i++) {
    char full_path[512];
    char base_path[420];
    strcpy(base_path, config_paths[i]);
    expand_path(base_path);
    cfg_full_path(full_path, base_path);
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
  strcpy(cfg->db_path, "");
  cfg_full_path(cfg->config_path, base_path);
  cfg->created = 0;
  return cfg;
}

config* read_config() {
  char config_path[420];
  if (!find_config(config_path)) {
    return create_config(config_paths[CONFIG_DEFAULT]);
  }
  config* cfg = base_config(config_path);

  FILE* file = fopen(cfg->config_path, "r");
  if (!file) {
    return cfg;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char key[128], value[256];

    if (sscanf(line, "%127[^=]=%127[^\n]", key, value) == 2) {
      if (strlen(value) == 0) continue;
      if (strcmp(key, "teams_n") == 0) {
        cfg->teams_n = atoi(value);
      } else if (strcmp(key, "team_size") == 0) {
        cfg->team_size = atoi(value);
      } else if (strcmp(key, "db_path") == 0) {
        strcpy(cfg->db_path, value);
      }
    }
  }
  fclose(file);
  return cfg;
}

config* create_config(const char* base_path) {
  char path[512];
  strcpy(path, base_path);
  expand_path(path);
  config* cfg = base_config(path);
  cfg->created = 1;
  make_dir(path);
  write_config(cfg);
  return cfg;
}

void expand_path(char* path) {
#ifdef __linux__
  char temp[512];
  if (path[0] == '~') {
    const char* home = getenv("HOME");
    if (!home) {
      return;
    }
    snprintf(temp, sizeof(temp), "%s%s", home, path + 1);
    strcpy(path, temp);
  }
#endif
}

void make_dir(const char* path) {
  char temp[512];
  char* p = NULL;
  size_t len;
  strcpy(temp, path);
  len = strlen(temp);
  if (temp[len - 1] == PATH_SEPARATOR) temp[len - 1] = '\0';
  for (p = temp + 1; *p; p++) {
    if (*p == PATH_SEPARATOR) {
      *p = '\0';
      if (!dir_exists(temp)) mkdir(temp, 0755);
      *p = PATH_SEPARATOR;
    }
  }
  if (!dir_exists(temp)) mkdir(temp, 0755);
}

void write_config(config* cfg) {
  FILE* file = fopen(cfg->config_path, "w");
  if (!file) {
    return;
  }
  fprintf(file, "teams_n=%d\n", cfg->teams_n);
  fprintf(file, "team_size=%d\n", cfg->team_size);
  fprintf(file, "db_path=%s\n", cfg->db_path);
  fclose(file);
}

