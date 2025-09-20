
#ifdef __linux__
#define PATH_SEPARATOR '/'
#elif _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#define PATH_SEPARATOR '\\'
#endif

void full_path(char* path, const char* base_path, const char* filename);
char* absolute_path(const char* path);
int file_exists(const char* path);
int dir_exists(const char* path);
void expand_path(char* path);
void make_dir(const char* path);

