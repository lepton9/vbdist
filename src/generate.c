#include <assert.h>
#include <math.h>
#include <time.h>

#include "../include/utils.h"
#include "../include/generate.h"
#include "../include/combo.h"


context* makeContext() {
  context* ctx = malloc(sizeof(context));
  ctx->teams_dim = malloc(sizeof(dimensions));
  ctx->banned_combos = NULL;
  ctx->pref_combos = NULL;
  ctx->skills = NULL;
  ctx->positions = NULL;
  return ctx;
}

void freeContext(context* ctx) {
  free(ctx->teams_dim);
  free(ctx);
}

void ctxUpdateDimensions(context* ctx, size_t teams_n, size_t team_size) {
  ctx->teams_dim->teams_n = teams_n;
  ctx->teams_dim->team_size = team_size;
}

double averageRating(team** teams, dimensions* dim, dlist* skill_ids) {
  int n = dim->teams_n * dim->team_size;
  if (teams == NULL) return 0.0;
  double sumRating = 0.0;
  for (size_t t = 0; t < dim->teams_n; t++) {
    for (size_t p = 0; p < dim->team_size; p++) {
      sumRating += rating_filter(teams[t]->players[p], skill_ids);
    }
  }
  return (n <= 0) ? 0.0 : sumRating / n;
}

int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation) {
  if (fabs(avg) < 1e-6f) return 1;
  int valid = 0;
  if (fabs(aNew - avg) < fabs(a - avg)) valid++;
  if (fabs(bNew - avg) < fabs(b - avg)) valid++;
  return (valid == 2) ? 1 : (valid == 1 && oneSideValidation) ? 1 : 0;
}

int maxTeamFromPrefCombos(dlist* prefCombos) {
  int biggest_size = 0;
  for (size_t i = 0; i < prefCombos->n; i++) {
    combo* c = prefCombos->items[i];
    if ((int)c->ids->n > biggest_size) {
      biggest_size = (int)c->ids->n;
    }
  }
  return biggest_size;
}

void setPreferredCombos(team** teams, dimensions* dim, dlist* prefCombos) {
  for (size_t c = 0; c < prefCombos->n; c++) {
    combo* combo = prefCombos->items[c];

    if (combo->ids->n < 2) continue;
    for (size_t ind = 0; ind < combo->ids->n - 1; ind++) {
      int id_a = *((int*)combo->ids->items[ind]);
      int id_b = *((int*)combo->ids->items[ind + 1]);

      int t1 = -1;
      int t2 = -1;
      player* p1 = NULL;
      player* p2 = NULL;
      player* pToSwap = NULL;
      // Finds the players in the combo and their teams
      for (size_t i = 0; i < dim->teams_n; i++) {
        for (size_t j = 0; j < dim->team_size; j++) {
          if (teams[i]->players[j]->id == id_a) {
            p1 = teams[i]->players[j];
            t1 = i;
          }
          else if (teams[i]->players[j]->id == id_b) {
            p2 = teams[i]->players[j];
            t2 = i;
          }
        }
        if (p1 && p2) break;
      }
      if (t1 == t2 || t1 < 0 || t2 < 0) continue;
      // Finds player to swap
      for (size_t i = 0; i < dim->team_size; i++) {
        player* maybeSwapP = teams[t1]->players[i];
        if (maybeSwapP->id == p1->id) continue;
        // Makes sure the player has no combos with anyone on the team
        char hasCombo = 0;
        for (size_t j = 0; j < dim->team_size; j++) {
          if ((hasCombo = isCombo(prefCombos, maybeSwapP, teams[t1]->players[j]))) break;
        }
        if (hasCombo) continue;
        else {
          pToSwap = maybeSwapP;
          break;
        }
      }
      if (pToSwap) swapPlayers(pToSwap, p2);
    }
  }
}

int getPlayerOfPosition(player** players, size_t n, position* pos) {
  int a = 0;
  int b = n - 1;
  int best_priority = -1;
  int player_ind = -1;

  for (; a <= b; a++, b--) {
    player* p_a = players[a];
    player* p_b = players[b];
    if (pos) {
      int ipos_a = hasPosition(p_a, pos);
      int ipos_b = hasPosition(p_b, pos);
      if (ipos_a > 0) {
        position* pos_a = p_a->positions->items[ipos_a];
        if (best_priority < 0 || pos_a->priority < best_priority) {
          best_priority = pos_a->priority;
          player_ind = a;
        }
      }
      if (ipos_b > 0) {
        position* pos_b = p_b->positions->items[ipos_b];
        if (best_priority < 0 || pos_b->priority < best_priority) {
          best_priority = pos_b->priority;
          player_ind = b;
        }
      }
    } else {
      if (p_a->positions->n == 0) {
        return a;
      }
      if (p_b->positions->n == 0) {
        return b;
      }
    }
  }
  return player_ind;
}

int findPlayerOfPosRand(player** players, size_t n, position* pos) {
  int player_ind = getPlayerOfPosition(players, n, pos);
  if (player_ind < 0 && n > 0) {
    return rand_int(0, n - 1);
  }
  return player_ind;
}

int balancedClustering(team** teams, int oneSideValidation, context* ctx) {
  double avgR = averageRating(teams, ctx->teams_dim, ctx->skills);
  int swaps = 0;
  int failures = 0;

  while(failures < MAX_FAILURES && swaps < MAX_SWAPS) {
    int teamA = rand_int(0, ctx->teams_dim->teams_n - 1);
    int teamB = rand_int(0, ctx->teams_dim->teams_n - 1);
    while(teamB == teamA) teamB = rand_int(0, ctx->teams_dim->teams_n - 1);
    player* pA = teams[teamA]->players[rand_int(0, ctx->teams_dim->team_size - 1)];
    player* pB = NULL;

    if (ctx->use_positions) {
      position* pos = (pA->positions->n > 0) ? pA->positions->items[0] : NULL;
      int ind = findPlayerOfPosRand(teams[teamB]->players, ctx->teams_dim->team_size, pos);
      pB = teams[teamB]->players[ind];
    } else {
      pB = teams[teamB]->players[rand_int(0, ctx->teams_dim->team_size - 1)];
    }

    double ratingTeamA = team_rating_filter(teams[teamA], ctx->skills);
    double ratingTeamB = team_rating_filter(teams[teamB], ctx->skills);

    swapPlayers(pA, pB);

    double ratingTeamA_new = team_rating_filter(teams[teamA], ctx->skills);
    double ratingTeamB_new = team_rating_filter(teams[teamB], ctx->skills);

    int valid = validateSwap(ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR, oneSideValidation);

    if (comboInTeam(ctx->pref_combos, teams[teamB], pA) || comboInTeam(ctx->pref_combos, teams[teamA], pB)) valid = 0;

    if (valid) {
      if (comboInTeam(ctx->banned_combos, teams[teamA], pA) || comboInTeam(ctx->banned_combos, teams[teamB], pB)) {
        failures++;
        swapPlayers(pA, pB);
      } else {
        swaps++;
        failures = 0;
      }
    } else {
      failures++;
      swapPlayers(pA, pB);
    }
    #ifdef __linux__
      printf("%3d/%3d | %d\r", failures, MAX_FAILURES, swaps);
    #endif
  }

  if (ctx->use_positions) {
    // TODO: sort by positions

  } else {
    for (size_t i = 0; i < ctx->teams_dim->teams_n; i++) {
      qsort(teams[i]->players, ctx->teams_dim->team_size, sizeof(player*), cmpPlayers);
    }
  }

  if (swaps >= MAX_SWAPS && oneSideValidation) swaps += balancedClustering(teams, 0, ctx);

  return swaps;
}

team** makeRandTeamsPositions(dlist* players, dimensions* dim, dlist* positions) {
  assert(players->n == dim->team_size * dim->teams_n);

  team** teams = malloc(sizeof(team*) * dim->teams_n);
  for (size_t i = 0; i < dim->teams_n; i++) {
    char tName[20];
    sprintf(tName, "Team %d", (int)i + 1);
    teams[i] = initTeam(tName, dim->team_size);
  }

  shuffle(players);

  dlist* remaining_players = init_list();
  for (size_t i = 0; i < players->n; i++) {
    list_add(remaining_players, players->items[i]);
  }

  size_t team_sizes[dim->teams_n];
  for (size_t i = 0; i < dim->teams_n; i++) team_sizes[i] = 0;

  // Set one of every position to every team
  for (size_t i = 0; i < positions->n && i < dim->team_size; i++) {
    position* p = positions->items[i];
    for (size_t t = 0; t < dim->teams_n; t++) {
      size_t ind = team_sizes[t];
      if (ind < dim->team_size) {
        int p_ind = findPlayerOfPosRand((player **)remaining_players->items,
                                        remaining_players->n, p);
        if (p_ind > 0) {
          player* player = pop_elem(remaining_players, p_ind);
          teams[t]->players[ind] = player;
          team_sizes[t]++;
        }
      }
    }
  }

  // Put remaining players into teams
  int t_i = 0;
  for (int i = remaining_players->n - 1; i >= 0; i--) {
    while (team_sizes[t_i] == dim->team_size) {
      t_i = (t_i + 1) % dim->teams_n;
    }
    if (team_sizes[t_i] < dim->team_size) {
      teams[t_i]->players[team_sizes[t_i]] = pop_elem(remaining_players, i);
      team_sizes[t_i]++;
    }
  }

  assert(remaining_players->n == 0);

  free_list(remaining_players);
  return teams;
}

team** balanceTeamsRand(dlist* players, dimensions* dim) {
  team** teams = malloc(sizeof(team*) * dim->teams_n);
  for (size_t i = 0; i < dim->teams_n; i++) {
    char tName[20];
    sprintf(tName, "Team %d", (int)i + 1);
    teams[i] = initTeam(tName, dim->team_size);
  }

  qsort(players->items, players->n, sizeof(player*), cmpPlayers);
  srand(time(0));

  int group = 0;
  int inTeam[dim->teams_n * dim->team_size];
  for (size_t i = 0; i < dim->teams_n * dim->team_size; i++) inTeam[i] = 0;

  int teamI = 0;
  int teamCounters[dim->teams_n];
  for (size_t i = 0; i < dim->teams_n; i++) teamCounters[i] = 0;

  for (size_t i = 0; i < players->n; i++) {
    int ind = rand_int(group * dim->teams_n, (group + 1) * dim->teams_n - 1);
    while(inTeam[ind]) ind = rand_int(group * dim->teams_n, (group + 1) * dim->teams_n - 1);
    inTeam[ind] = 1;

    teams[teamI]->players[teamCounters[teamI]++] = players->items[ind];
    teamI = (teamI + 1) % dim->teams_n;
    if (teamI == 0) group++;
  }
  return teams;
}

