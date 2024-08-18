import 'dart:math';
import 'player.dart';

class Team {
  String name = '';
  late List<Player> players;

  Team(String teamName, int size) {
    name = teamName;
    players = List<Player>.filled(size, Player.empty()); // Initialize with dummy players
  }

  double avgRating() {
    if (players != null) {
      double sumRatingT = players.fold(0.0, (sum, player) => sum + player.ovRating());
      return sumRatingT / players.length;
    }
    return 0.0;
  }
}

double avgRating(Team t) {
  return t?.avgRating() ?? 0.0;
}

