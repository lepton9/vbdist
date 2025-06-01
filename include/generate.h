#ifndef GENERATE_H
#define GENERATE_H

#include "dlist.h"
#include "team.h"

#define MAX_FAILURES 300
#define MAX_SWAPS 1000000

typedef enum {
  OV_AVERAGE = 0,
  SKILL_AVERAGE,
} comparison;

typedef struct {
  size_t teams_n;
  size_t team_size;
} dimensions;

typedef struct {
  dlist* banned_combos;
  dlist* pref_combos;
  dlist* skills;
  dlist* positions;

  comparison compare;
  dimensions* teams_dim;
  int use_positions;
} context;

context* makeContext();
void freeContext(context* ctx);
void ctxUpdateDimensions(context* ctx, size_t teams_n, size_t team_size);
void changeComparison(comparison* c);

double averageRating(team** teams, dimensions* dim, dlist* sel_skills);
dlist* averageSkillRatings(team** teams, dimensions* dim, dlist* sel_skills);

double team_skill_distance(dlist* team_skills, dlist* avg);
int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation);
int validateSwapSkills(dlist* a, dlist* b, dlist* aNew, dlist* bNew, dlist* avg);

int maxTeamFromPrefCombos(dlist* prefCombos);
void setPreferredCombos(team** teams, dimensions* dim, dlist* prefCombos);

int getPlayerOfPosition(player** players, size_t n, position* pos);
int findPlayerOfPosRand(player** players, size_t n, position* pos);
int getPlayerOfPosAsgn(player** players, size_t n, position* pos);
int findPlayerOfPosAsngRand(player** players, size_t n, position* pos);
void sortPositions(dlist* positions, dlist* players);

int balancedClustering(team** teams, int oneSideValidation, context* ctx);

team** makeRandTeamsPositions(dlist* players, dimensions* dim, dlist* positions);
team** balanceTeamsRand(dlist* players, dimensions* dim);
team** initialTeams(dlist* players, dimensions* dim, context* ctx);

#endif

