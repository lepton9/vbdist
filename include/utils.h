#ifndef UTILS_H
#define UTILS_H

#include <string.h>

int isBackspace(int c);
int isEnter(int c);
void strcatc(char* str, size_t* len, char c);
char* trimWS(char* str);

#endif
