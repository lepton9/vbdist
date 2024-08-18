import 'dart:io';
import 'dart:math';

const int DIFRATINGS = 6; // Assuming DIFRATINGS is defined as 6

class Player {
  String firstName = '';
  String surName = '';
  int id = -1;
  List<int> ratings = List.filled(DIFRATINGS, 0);

  Player.empty() {
    this.firstName = '';
    this.surName = '';
  }

  Player(this.firstName, this.surName, this.ratings);

  Player.fromPlayer(Player p) {
    firstName = p.firstName;
    surName = p.surName;
    ratings = p.ratings;
  }

  double ovRating() {
    if (ratings.isEmpty) return 0.0;
    int sum = ratings.reduce((a, b) => a + b);
    return sum / DIFRATINGS;
  }

  void printPlayer(IOSink out) {
    String fullName = firstName ?? '';
    if (surName != null) fullName += ' ${surName}';
    out.write('${fullName.padRight(20)} ');
    for (int rating in ratings) {
      out.write('$rating ');
    }
    out.write('| ${ovRating().toStringAsFixed(1)}\n');
  }
}

Player parsePlayer(String pStr) {
  Player p = Player.empty();
  List<String> tokens = pStr.split('|');
  String? fullName;
  
  if (tokens.isNotEmpty) {
    fullName = tokens[0].trim();
  }

  if (fullName != null) {
    List<String> nameParts = fullName.split(' ');
    if (nameParts.length > 1) {
      p.firstName = nameParts.sublist(0, nameParts.length - 1).join(' ');
      p.surName = nameParts.last;
    } else {
      p.firstName = fullName;
      p.surName = "";
    }
  }

  if (tokens.length > 1) {
    List<String> ratingTokens = tokens[1].trim().split(' ');
    for (int i = 0; i < min(DIFRATINGS, ratingTokens.length); i++) {
      p.ratings[i] = int.parse(ratingTokens[i]);
    }
  }

  return p;
}

int cmpPlayers(Player a, Player b) {
  double ret = b.ovRating() - a.ovRating();
  if (ret < 0) return -1;
  if (ret > 0) return 1;
  return 0;
}

void swapPlayers(Player a, Player b) {
  Player tmp = Player.fromPlayer(a);
  
  a.firstName = b.firstName;
  a.surName = b.surName;
  a.ratings = List.from(b.ratings);
  
  b.firstName = tmp.firstName;
  b.surName = tmp.surName;
  b.ratings = tmp.ratings;
}


