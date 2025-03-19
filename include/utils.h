#ifndef UTILS_H
#define UTILS_H

#include <string.h>

int isBackspace(int c);
int isEnter(int c);
void strcatc(char* str, size_t* len, char c);
char* trimWS(char* str);
int min_int(int a, int b);
int max_int(int a, int b);
int rand_int(const int min, const int max);

#endif
