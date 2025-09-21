#include <stdlib.h>

#ifdef __linux__
#define PATH_SEPARATOR '/'
#elif _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#define PATH_SEPARATOR '\\'
#endif

#define PATH_SIZE 512

void full_path(char* path, size_t size, const char* base_path, const char* filename);
char* absolute_path(const char* path);
int file_exists(const char* path);
int dir_exists(const char* path);
void expand_path(char* path);
void make_dir(const char* path);

