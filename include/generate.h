#ifndef GENERATE_H
#define GENERATE_H

#include "dlist.h"
#include "team.h"

#define MAX_FAILURES 300
#define MAX_SWAPS 1000000

typedef struct {
  size_t teams_n;
  size_t team_size;
} dimensions;

typedef struct {
  dlist* banned_combos;
  dlist* pref_combos;
  dlist* skills;
  dlist* positions;

  dimensions* teams_dim;
  int use_positions;
} context;

context* makeContext();
void freeContext(context* ctx);
void ctxUpdateDimensions(context* ctx, size_t teams_n, size_t team_size);

double averageRating(team** teams, dimensions* dim, dlist* skill_ids);
int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation);

int maxTeamFromPrefCombos(dlist* prefCombos);
void setPreferredCombos(team** teams, dimensions* dim, dlist* prefCombos);

int getPlayerOfPosition(player** players, size_t n, position* pos);
int findPlayerOfPosRand(player** players, size_t n, position* pos);
int balancedClustering(team** teams, int oneSideValidation, context* ctx);

team** makeRandTeamsPositions(dlist* players, dimensions* dim, dlist* positions);
team** balanceTeamsRand(dlist* players, dimensions* dim);

#endif

