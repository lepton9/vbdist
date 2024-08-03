
#define DIFRATINGS 5

// typedef struct {
//   int defence;
//   int spiking;
//   int serving;
//   int setting;
//   int saving;
// } rating;

typedef struct {
  char* name;
  int ratings[DIFRATINGS];
} player;

player* initPlayer();
void freePlayer(player* p);
player* parsePlayer(char* pStr);
int cmpPlayers(const void* a, const void* b);
void swapPlayers(player* a, player* b);
double ovRating(player* p);

