import 'dart:io';
import 'dart:convert';
import 'dart:math';
import 'player.dart';
import 'team.dart';

const int MAX_FAILURES = 500;

int TEAMS_N = 0;
int TEAM_SIZE = 0;

class BannedPCombo {
  int pidA;
  int pidB;

  BannedPCombo(this.pidA, this.pidB);
}

class BannedCombos {
  List<BannedPCombo> combos = [];

  BannedCombos();

  int size() {return combos.length;}

  void addBPC(int a, int b) {
    combos.add(BannedPCombo(a, b));
  }
}

String trimWS(String str) {
  return str.trim();
}

void parseBPC(String line, List<String> nA, List<String> nB) {
  if (line[0] == '!') {
    line = line.substring(1);
  }
  var tokens = line.split('-');
  if (tokens.isNotEmpty) nA[0] = trimWS(tokens[0]);
  if (tokens.length > 1) nB[0] = trimWS(tokens[1]);
}

List<Player> readPlayers(String fileName, List<int> pn, BannedCombos bpcs) {
  var file = File(fileName);
  if (!file.existsSync()) return [];

  List<Player> ps = [];
  int pid = 0;

  for (var line in file.readAsLinesSync()) {
    line = line.trim();
    if (line.startsWith('#') || line.isEmpty) continue;
    if (line.startsWith('!')) {
      List<String> ap = [''];
      List<String> bp = [''];
      int a = -1;
      int b = -1;
      parseBPC(line, ap, bp);
      for (int i = 0; i < ps.length; i++) {
        if (ps[i].firstName == ap[0] && a < 0) a = ps[i].id;
        if (ps[i].firstName == bp[0] && b < 0) b = ps[i].id;
      }
      if (a >= 0 && b >= 0) bpcs.addBPC(a, b);
      continue;
    }
    ps.add(parsePlayer(line));
    ps.last.id = pid++;
  }

  pn[0] = ps.length;
  return ps;
}

void printPlayers(List<Player> players) {
  for (var player in players) {
    player.printPlayer(stdout);
  }
}

void printTeams(List<Team> teams) {
  for (int i = 0; i < TEAMS_N; i++) {
    print('${teams[i].name} | Rating: ${avgRating(teams[i])}:');
    for (int j = 0; j < TEAM_SIZE; j++) {
      print('  ${teams[i].players[j].firstName} (${teams[i].players[j].ovRating()})');
    }
    print('');
  }
}

int randintRange(int min, int max) {
  return Random().nextInt(max + 1 - min) + min;
}

double averageRating(List<Team> teams) {
  if (teams.isEmpty) return 0.0;
  double sumRating = 0.0;
  for (int t = 0; t < TEAMS_N; t++) {
    sumRating += avgRating(teams[t]);
  }
  return sumRating / TEAMS_N;
}

bool bannedCombo(BannedCombos bpcs, Player a, Player b) {
  for (var combo in bpcs.combos) {
    if ((combo.pidA == a.id && combo.pidB == b.id) ||
        (combo.pidA == b.id && combo.pidB == a.id)) {
      return true;
    }
  }
  return false;
}

bool validateSwap(double a, double b, double aNew, double bNew, double avg, bool oneSideValidation) {
  int valid = 0;
  if ((aNew - avg).abs() < (a - avg).abs()) valid++;
  if ((bNew - avg).abs() < (b - avg).abs()) valid++;

  return (valid == 2) || (valid == 1 && oneSideValidation);
}

int balancedClustering(List<Team> teams, bool oneSideValidation, BannedCombos bpcs) {
  double avgR = averageRating(teams);
  int swaps = 0;
  int failures = 0;

  while (failures < MAX_FAILURES) {
    int teamA = randintRange(0, TEAMS_N - 1);
    int teamB = randintRange(0, TEAMS_N - 1);
    while (teamB == teamA) teamB = randintRange(0, TEAMS_N - 1);
    Player pA = teams[teamA].players[randintRange(0, TEAM_SIZE - 1)];
    Player pB = teams[teamB].players[randintRange(0, TEAM_SIZE - 1)];

    double ratingTeamA = avgRating(teams[teamA]);
    double ratingTeamB = avgRating(teams[teamB]);

    swapPlayers(pA, pB);

    double ratingTeamA_new = avgRating(teams[teamA]);
    double ratingTeamB_new = avgRating(teams[teamB]);

    bool valid = validateSwap(ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR, oneSideValidation);

    if (valid) {
      bool banned = false;
      for (int pI = 0; pI < TEAM_SIZE; pI++) {
        if (bannedCombo(bpcs, pA, teams[teamA].players[pI]) || bannedCombo(bpcs, pB, teams[teamB].players[pI])) {
          banned = true;
          break;
        }
      }
      if (banned) {
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
  }

  for (int i = 0; i < TEAMS_N; i++) {
    teams[i].players.sort((a, b) => cmpPlayers(a, b));
  }

  return swaps;
}

List<Team> balanceTeamsRand(List<Player> players, int n) {
  List<Team> teams = List.generate(TEAMS_N, (i) {
    String tName = 'Team ${i + 1}';
    return Team(tName, TEAM_SIZE);
  });

  players.sort((a, b) => cmpPlayers(a, b));

  int group = 0;
  List<int> inTeam = List.filled(TEAMS_N * TEAM_SIZE, 0);

  int teamI = 0;
  List<int> teamCounters = List.filled(TEAMS_N, 0);

  for (int i = 0; i < n; i++) {
    int ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
    while (inTeam[ind] == 1) ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
    inTeam[ind] = 1;

    teams[teamI].players[teamCounters[teamI]++] = players[ind];
    teamI = (teamI + 1) % TEAMS_N;
    if (teamI == 0) group++;
  }
  return teams;
}

List<Team> balanceTeams(List<Player> players, int n) {
  List<Team> teams = List.generate(TEAMS_N, (i) {
    String tName = 'Team ${i + 1}';
    return Team(tName, TEAM_SIZE);
  });
  players.sort((a, b) => cmpPlayers(a, b));

  int teamI = 0;
  List<int> teamCounters = List.filled(TEAMS_N, 0);

  for (int i = 0; i < n; i++) {
    teams[teamI].players[teamCounters[teamI]++] = players[i];
    teamI = (teamI + 1) % TEAMS_N;
  }
  return teams;
}

void printTeamsVert(List<Team> teams, IOSink out) {
  for (int i = 0; i < TEAMS_N; i++) {
    out.write('${teams[i].name}:\n');
    for (int j = 0; j < TEAM_SIZE; j++) {
      out.write('${teams[i].players[j].firstName}\n');
    }
    out.write('\n');
  }
}

void printTeamsHor(List<Team> teams, IOSink out) {
  for (int i = 0; i < TEAMS_N; i++) {
    out.write('${teams[i].name.padRight(15)}');
  }
  out.write('\r');
  for (int j = 0; j < TEAM_SIZE; j++) {
    for (int i = 0; i < TEAMS_N; i++) {
      out.write('${teams[i].players[j].firstName.padRight(15)}');
    }
    out.write('\r');
  }
}

void writeTeamsToFile(List<Team> teams, String teamsFile) {
  var file = File(teamsFile);
  var sink = file.openWrite();
  printTeamsVert(teams, sink);
  printTeamsHor(teams, sink);
  sink.close();
}

void changeMode(List<Team> teams) {
  printTeamsHor(teams, stdout);
}

void main(List<String> args) {
  if (args.length < 3) {
    print('Usage -> \n  vbdist playersFile TEAMS_N TEAM_SIZE');
    print('\nPlayers file should be format:\n');
    print('PlayerName | Defence Spike Serve Setting Saving Consistency');
    exit(1);
  }

  String fileName = args[0];
  TEAMS_N = int.parse(args[1]);
  TEAM_SIZE = int.parse(args[2]);
  bool clustering = true;
  bool printStd = true;
  if (args.length >= 4) printStd = int.parse(args[3]) == 1;

  List<int> pn = [0];
  BannedCombos bpcs = BannedCombos();
  List<Player> players = readPlayers(fileName, pn, bpcs);

  if (players.isEmpty) {
    print('File $fileName not found');
    exit(1);
  }

  players.sort((a, b) => cmpPlayers(a, b));

  if (printStd) printPlayers(players);
  print('Banned combinations: ${bpcs.size()}');

  if (pn[0] != TEAMS_N * TEAM_SIZE) {
    print('\nFile $fileName contains ${pn[0]} players, but ${TEAMS_N * TEAM_SIZE} was expected');
    exit(1);
  }

  List<Team> teams = balanceTeamsRand(players, pn[0]);

  if (clustering) {
    print('\nBalancing teams..');
    int swaps = balancedClustering(teams, true, bpcs);
    print('Total swaps: $swaps');
  }

  print('');
  if (printStd) printTeams(teams);

  writeTeamsToFile(teams, 'teams.txt');

}

