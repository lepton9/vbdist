#include <stdio.h>

#define DIFRATINGS 6

typedef struct {
  char* firstName;
  char* surName;
  int id;
  int ratings[DIFRATINGS];
} player;

player* initPlayer();
void freePlayer(player* p);
player* parsePlayer(char* pStr);
int cmpPlayers(const void* a, const void* b);
void swapPlayers(player* a, player* b);
double ovRating(player* p);
void printPlayer(FILE* out, player* p);


