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
  ctx->compare = OV_AVERAGE;
  ctx->use_positions = 0;
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

void changeComparison(comparison* c) {
  *c = (*c == OV_AVERAGE) ? SKILL_AVERAGE : OV_AVERAGE;
}

dlist* averageSkillRatings(team** teams, dimensions* dim, dlist* sel_skills) {
  dlist* avg_skills = init_list();
  int* psw_skills = calloc(sel_skills->n, sizeof(int));
  for (size_t i = 0; i < sel_skills->n; i++) {
    list_add(avg_skills, copySkillVal(sel_skills->items[i], 0));
  }

  for (size_t t_i = 0; t_i < dim->teams_n; t_i++) {
    for (size_t p_i = 0; p_i < dim->team_size; p_i++) {
      player* p = teams[t_i]->players[p_i];
      for (size_t s_i = 0; s_i < avg_skills->n; s_i++) {
        skill* s = avg_skills->items[s_i];
        double val = get_skill_value(p, s);
        if (fabs(val) > 1e-6f) {
          s->value += val;
          psw_skills[s_i]++;
        }
      }
    }
  }

  for (size_t s_i = 0; s_i < avg_skills->n; s_i++) {
    skill* s = avg_skills->items[s_i];
    s->value = s->value / ((psw_skills[s_i] > 0) ? psw_skills[s_i] : 1);
  }
  free(psw_skills);
  return avg_skills;
}

double averageRating(team** teams, dimensions* dim, dlist* sel_skills) {
  int n = dim->teams_n * dim->team_size;
  if (teams == NULL) return 0.0;
  double sumRating = 0.0;
  for (size_t t = 0; t < dim->teams_n; t++) {
    for (size_t p = 0; p < dim->team_size; p++) {
      sumRating += rating_filter(teams[t]->players[p], sel_skills);
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

// Calc L2 norm
double team_skill_distance(dlist* team_skills, dlist* avg) {
  double dist = 0.0;
  for (size_t i = 0; i < avg->n; i++) {
    double delta = ((skill*)team_skills->items[i])->value - ((skill*)avg->items[i])->value;
    dist += delta * delta;
  }
  return sqrt(dist);
}

int validateSwapSkills(dlist* a, dlist* b, dlist* aNew, dlist* bNew, dlist* avg) {
  double oldDist = team_skill_distance(a, avg) + team_skill_distance(b, avg);
  double newDist = team_skill_distance(aNew, avg) + team_skill_distance(bNew, avg);
  return newDist < oldDist;
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

// TODO: deprecated
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

int getPlayerOfPosAsgn(player** players, size_t n, position* pos) {
  int a = 0;
  int b = n - 1;
  for (; a <= b; a++, b--) {
    position* pos_a = assignedPosition(players[a]);
    position* pos_b = assignedPosition(players[b]);
    if ((pos && pos_a && pos_a->id == pos->id) || (!pos && !pos_a)) {
      return a;
    }
    if ((pos && pos_b && pos_b->id == pos->id) || (!pos && !pos_b)) {
      return b;
    }
  }
  return -1;
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
      if (ipos_a >= 0) {
        position* pos_a = p_a->positions->items[ipos_a];
        if (best_priority < 0 || pos_a->priority < best_priority || p_a->positions->n == 1) {
          best_priority = pos_a->priority;
          player_ind = a;
        }
      }
      if (ipos_b >= 0) {
        position* pos_b = p_b->positions->items[ipos_b];
        if (best_priority < 0 || pos_b->priority < best_priority || p_b->positions->n == 1) {
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

int balancedClustering(team** teams, int oneSideValidation, context* ctx) {
  int swaps = 0;
  int failures = 0;

  double avgR = averageRating(teams, ctx->teams_dim, ctx->skills);

  dlist* avg_skills = NULL;
  dlist* ta_rating = NULL;
  dlist* tb_rating = NULL;
  dlist* ta_rating_new = NULL;
  dlist* tb_rating_new = NULL;

  if (ctx->compare == SKILL_AVERAGE) {
    avg_skills = averageSkillRatings(teams, ctx->teams_dim, ctx->skills);
    ta_rating = init_list();
    tb_rating = init_list();
    ta_rating_new = init_list();
    tb_rating_new = init_list();
    for (size_t i = 0; i < avg_skills->n; i++) {
      skill* s = avg_skills->items[i];
      list_add(ta_rating, copySkillVal(s, 0));
      list_add(tb_rating, copySkillVal(s, 0));
      list_add(ta_rating_new, copySkillVal(s, 0));
      list_add(tb_rating_new, copySkillVal(s, 0));
    }
  }

  while(failures < MAX_FAILURES && swaps < MAX_SWAPS) {
    int teamA = rand_int(0, ctx->teams_dim->teams_n - 1);
    int teamB = rand_int(0, ctx->teams_dim->teams_n - 1);
    while(teamB == teamA) teamB = rand_int(0, ctx->teams_dim->teams_n - 1);
    player* pA = teams[teamA]->players[rand_int(0, ctx->teams_dim->team_size - 1)];
    player* pB = NULL;

    if (ctx->use_positions) {
      position* pos = assignedPosition(pA);
      int ind_b = getPlayerOfPosAsgn(teams[teamB]->players, ctx->teams_dim->team_size, pos);
      if (ind_b >= 0) {
        pB = teams[teamB]->players[ind_b];
      } else {
        failures++;
        continue;
      }
    } else {
      pB = teams[teamB]->players[rand_int(0, ctx->teams_dim->team_size - 1)];
    }

    int valid = 0;
    if (ctx->compare == SKILL_AVERAGE) {
      team_average_skills(teams[teamA], ta_rating);
      team_average_skills(teams[teamB], tb_rating);

      swapPlayers(pA, pB);

      team_average_skills(teams[teamA], ta_rating_new);
      team_average_skills(teams[teamB], tb_rating_new);

      valid = validateSwapSkills(ta_rating, tb_rating, ta_rating_new, tb_rating_new, avg_skills);
    } else {
      double ratingTeamA = team_rating_filter(teams[teamA], ctx->skills);
      double ratingTeamB = team_rating_filter(teams[teamB], ctx->skills);

      swapPlayers(pA, pB);

      double ratingTeamA_new = team_rating_filter(teams[teamA], ctx->skills);
      double ratingTeamB_new = team_rating_filter(teams[teamB], ctx->skills);

      valid = validateSwap(ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR, oneSideValidation);
    }

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

    // Reset team ratings
    if (ctx->compare == SKILL_AVERAGE) {
      for (size_t i = 0; i < avg_skills->n; i++) {
        ((skill*)ta_rating->items[i])->value = 0.0;
        ((skill*)tb_rating->items[i])->value = 0.0;
        ((skill*)ta_rating_new->items[i])->value = 0.0;
        ((skill*)tb_rating_new->items[i])->value = 0.0;
      }
    }
  }

  if (ctx->use_positions) {
    // TODO: sort by positions

  } else {
    for (size_t i = 0; i < ctx->teams_dim->teams_n; i++) {
      qsort(teams[i]->players, ctx->teams_dim->team_size, sizeof(player*), cmpPlayers);
    }
  }

  if (ctx->compare == SKILL_AVERAGE) {
    for (size_t i = 0; i < avg_skills->n; i++) {
      freeSkill(avg_skills->items[i]);
      freeSkill(ta_rating->items[i]);
      freeSkill(tb_rating->items[i]);
      freeSkill(ta_rating_new->items[i]);
      freeSkill(tb_rating_new->items[i]);
    }
    free_list(avg_skills);
    free_list(ta_rating);
    free_list(tb_rating);
    free_list(ta_rating_new);
    free_list(tb_rating_new);
  }

  if (swaps >= MAX_SWAPS && oneSideValidation) swaps += balancedClustering(teams, 0, ctx);

  return swaps;
}

// TODO: temporary
void printTeamsTemp(FILE *out, team **teams, const int printWidth,
                const int teams_n, const int team_size, const char indent) {
  char str[printWidth];
  for (int i = 0; i < teams_n; i++) {
    team* team = teams[i];
    sprintf(str, "%s | %.2f:", team->name, avgRating(team));
    fprintf(out, "%-*s", printWidth, str);
  }
  fprintf(out, "\n");
  for(int j = 0; j < 6; j++) {
    for (int i = 0; i < teams_n; i++) {
      player* p = teams[i]->players[j];
      position* pos = assignedPosition(p);
      sprintf(str, "%s%-15s (%s | %d)", (indent) ? "  " : "", p->firstName, (pos) ? pos->name : "", (pos) ? pos->priority : -1);
      fprintf(out, "%-*s", printWidth, str);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
}

void divideDupPositions(dlist* positions, int* pAmount) {
  int n = 0;
  int indexes[positions->n];
  char processed[positions->n];
  memset(indexes, 0, sizeof(indexes));
  memset(processed, 0, sizeof(processed));
  for (size_t i = 0; i < positions->n; i++) {
    if (processed[i]) continue;
    processed[i] = 1;

    position* pos = positions->items[i];
    for (int ind = findPosition(positions, pos); ind >= 0;
         ind = findPositionFrom(positions, pos, ind + 1)) {
      indexes[n++] = ind;
      processed[ind] = 1;
    }
    for (int j = 0; j < n; j++) {
      pAmount[indexes[j]] = pAmount[indexes[j]] / n;
    }
    n = 0;
    memset(indexes, 0, sizeof(indexes));
  }
}

void incPosAmount(dlist* positions, position* pos, int* pAmount) {
  for (int ind = findPosition(positions, pos); ind >= 0;
       ind = findPositionFrom(positions, pos, ind + 1)) {
    pAmount[ind]++;
  }
}

int* calcPosAmount(dlist* positions, dlist* players) {
  int* pAmount = calloc(positions->n, sizeof(int));
  for (size_t i = 0; i < players->n; i++) {
    player* p = players->items[i];
    for (size_t j = 0; j < p->positions->n; j++) {
      position* pos = p->positions->items[j];
      incPosAmount(positions, pos, pAmount);
    }
  }
  return pAmount;
}

void sortPositions(dlist* positions, dlist* players) {
  if (positions->n <= 1) return;
  int* pAmount = calcPosAmount(positions, players);

  divideDupPositions(positions, pAmount);

  for (size_t i = 0; i < positions->n; i++) {
    size_t least_ind = i;
    for (size_t j = i + 1; j < positions->n; j++) {
      if (pAmount[j] < pAmount[least_ind]) {
        least_ind = j;
      }
    }
    if (least_ind != i) {
      swap_elems(positions, i, least_ind);
      pAmount[i] ^= pAmount[least_ind];
      pAmount[least_ind] ^= pAmount[i];
      pAmount[i] ^= pAmount[least_ind];
    }
  }
  free(pAmount);
}

player* findFromTeams(team** teams, size_t teams_n, size_t* team_sizes, dlist* remaining_players, position* pos) {
  int t_i = -1, p_i = -1;
  int replacement_ind = -1;
  int potential_prio = -1;
  int replacement_prio = -1;

  for (size_t i = 0; i < teams_n; i++) {
    for (size_t j = 0; j < team_sizes[i]; j++) {
      player* p = teams[i]->players[j];
      int ind = hasPosition(p, pos);
      if (ind < 0) continue;
      position* asg_pos = assignedPosition(p);
      if (!asg_pos) continue;
      int p_ind = getPlayerOfPosition((player **)remaining_players->items,
                                      remaining_players->n, asg_pos);
      if (p_ind < 0) continue;

      player* replacement = remaining_players->items[p_ind];
      int pos_ind = hasPosition(replacement, asg_pos);
      int potential_priority = ((position *)p->positions->items[ind])->priority;
      int replacement_priority = ((position *)replacement->positions->items[pos_ind])->priority;

      if (replacement_prio < 0 || potential_prio < 0 || 
          (potential_priority <= potential_prio && replacement_priority <= replacement_prio)) {
        replacement_prio = replacement_priority;
        potential_prio = potential_priority;
        replacement_ind = p_ind;
        t_i = i;
        p_i = j;
      }
    }
  }

  if (t_i >= 0 && p_i >= 0 && replacement_ind >= 0) {
    player* found_p = teams[t_i]->players[p_i];
    player* replacement = pop_elem(remaining_players, replacement_ind);
    setPlayerPosition(replacement, assignedPosition(found_p));
    teams[t_i]->players[p_i] = replacement;
    return found_p;
  }
  return NULL;
}

team** initTeams(const size_t n, const size_t size) {
  team** teams = malloc(sizeof(team*) * n);
  for (size_t i = 0; i < n; i++) {
    char tName[20];
    sprintf(tName, "Team %d", (int)i + 1);
    teams[i] = initTeam(tName, size);
  }
  return teams;
}

void putPrefComboToTeam(team* t, size_t n, combo* c, dlist* players) {
  if (t->size - n < c->ids->n) return;
  for (size_t i = 0; i < c->ids->n; i++) {
    int id = *((int*)c->ids->items[i]);
    int ind = playerInList(players, id);
    if (ind >= 0) {
      t->players[n++] = pop_elem(players, ind);
    }
  }
}

void initPrefCsToTeams(team** teams, size_t* team_sizes, context* ctx, dlist* players) {
  int last_team = 0;
  for (size_t c = 0; c < ctx->pref_combos->n; c++) {
    combo* combo = ctx->pref_combos->items[c];
    if (combo->ids->n < 2) continue;

    // Get team to insert to
    int misses = 0;
    while (misses < (int)ctx->teams_dim->teams_n && (ctx->teams_dim->team_size - team_sizes[last_team] < combo->ids->n)) {
      last_team = (last_team + 1) % ctx->teams_dim->teams_n;
      misses++;
    }
    if (ctx->teams_dim->team_size - team_sizes[last_team] >= combo->ids->n) {
      int rem = players->n;
      putPrefComboToTeam(teams[last_team], team_sizes[last_team], combo, players);
      team_sizes[last_team] += rem - players->n;
    } else {
      for (size_t i = 0; i < combo->ids->n; i++) {
        int id = *((int*)combo->ids->items[i]);
        int ind = playerInList(players, id);
        if (ind >= 0) {
          while ((team_sizes[last_team] >= ctx->teams_dim->team_size)) {
            last_team = (last_team + 1) % ctx->teams_dim->teams_n;
          }
          player* p = pop_elem(players, ind);
          teams[last_team]->players[team_sizes[last_team]++] = p;
        }
      }
    }
  }
}

void initBpcsToTeams(team** teams, size_t* team_sizes, context* ctx, dlist* players) {
  int last_team = 0;
  for (size_t c = 0; c < ctx->banned_combos->n; c++) {
    combo* combo = ctx->banned_combos->items[c];
    if (combo->ids->n < 2) continue;
    for (size_t i = 0; i < combo->ids->n; i++) {
      int id = *((int*)combo->ids->items[i]);
      int ind = playerInList(players, id);
      player* p = players->items[ind];
      if (ind >= 0 && p) {
        int misses = 0;
        while (misses < (int)ctx->teams_dim->teams_n && (team_sizes[last_team] >= ctx->teams_dim->team_size ||
               comboInTeamSize(ctx->banned_combos, teams[last_team],
                               team_sizes[last_team], p))) {
          last_team = (last_team + 1) % ctx->teams_dim->teams_n;
          misses++;
        }
        teams[last_team]->players[team_sizes[last_team]++] = pop_elem(players, ind);
        last_team = (last_team + 1) % ctx->teams_dim->teams_n;
      }
    }
  }
}

void initPosToTeams(team** teams, size_t* team_sizes, context* ctx, dlist* players) {
  for (size_t i = 0; i < ctx->positions->n && i < ctx->teams_dim->team_size; i++) {
    position* pos = ctx->positions->items[i];
    for (size_t t = 0; t < ctx->teams_dim->teams_n; t++) {
      size_t ind = team_sizes[t];
      if (ind < ctx->teams_dim->team_size) {
        int p_ind = getPlayerOfPosition((player **)players->items,
                                        players->n, pos);
        player* player = NULL;
        if (p_ind >= 0) {
          player = pop_elem(players, p_ind);
          setPlayerPosition(player, pos);
          teams[t]->players[ind] = player;
          team_sizes[t]++;
        } else {
          player = findFromTeams(teams, ctx->teams_dim->teams_n, team_sizes, players, pos);
          if (player) {
            setPlayerPosition(player, pos);
            teams[t]->players[ind] = player;
            team_sizes[t]++;
          } else {
            // TODO: set borrowed position?
            printf("Can't find player for position: %s\n", pos->name);
          }
        }
      }
    }
  }
}

team** initialTeams(dlist* players, context* ctx) {
  assert(players->n == ctx->teams_dim->team_size * ctx->teams_dim->teams_n);

  team** teams = initTeams(ctx->teams_dim->teams_n, ctx->teams_dim->team_size);

  shuffle(players);

  dlist* remaining_players = init_list();
  for (size_t i = 0; i < players->n; i++) {
    list_add(remaining_players, players->items[i]);
  }
  size_t team_sizes[ctx->teams_dim->teams_n];
  memset(team_sizes, 0, sizeof(team_sizes));

  int last_team = 0;

  // Set preferred combos to teams
  initPrefCsToTeams(teams, team_sizes, ctx, remaining_players);

  // Set banned combos to teams
  initBpcsToTeams(teams, team_sizes, ctx, remaining_players);

  // Set rest of the players into teams
  while(remaining_players->n > 0) {
    while ((team_sizes[last_team] >= ctx->teams_dim->team_size)) {
      last_team = (last_team + 1) % ctx->teams_dim->teams_n;
    }
    player* p = pop_elem(remaining_players, remaining_players->n - 1);
    teams[last_team]->players[team_sizes[last_team]++] = p;
  }

  assert(remaining_players->n == 0);
  free_list(remaining_players);
  return teams;
}

team** initialTeamsPositions(dlist* players, context* ctx) {
  dimensions* dim = ctx->teams_dim;
  assert(players->n == dim->team_size * dim->teams_n);
  assert(ctx->positions->n == dim->team_size);

  team** teams = initTeams(dim->teams_n, dim->team_size);

  sortPositions(ctx->positions, players);
  shuffle(players);

  dlist* remaining_players = init_list();
  for (size_t i = 0; i < players->n; i++) {
    list_add(remaining_players, players->items[i]);
  }
  size_t team_sizes[dim->teams_n];
  memset(team_sizes, 0, sizeof(team_sizes));

  // TODO: handle combos

  // Set one of every position to every team
  initPosToTeams(teams, team_sizes, ctx, remaining_players);

  // TODO: needed?
  // if all players are set before this
  printf("\nRemaining: %d\n\n", (int)remaining_players->n);

  // Put remaining players into teams
  int t_i = 0;
  for (int i = remaining_players->n - 1; i >= 0; i--) {
    while (team_sizes[t_i] == dim->team_size) {
      t_i = (t_i + 1) % dim->teams_n;
    }
    if (team_sizes[t_i] < dim->team_size) {
      teams[t_i]->players[team_sizes[t_i]++] = pop_elem(remaining_players, i);
    }
  }

  // TODO: remove
  printTeamsTemp(stdout, teams, 35, dim->teams_n, dim->team_size, 2);

  assert(remaining_players->n == 0);
  free_list(remaining_players);
  return teams;
}

