
# Volleyball team maker

[![Build](https://github.com/lepton9/vbdist/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/lepton9/vbdist/actions/workflows/build.yml)
![Release](https://img.shields.io/github/v/release/lepton9/vbdist)

Makes `N` teams of `M` players based on given attributes

## Installing and compiling
### Install dependencies
  *Arch*:
```
sudo pacman -S ncurses sqlite3
```
  *Ubuntu/Debian*:
```
sudo apt-get install libncurses5-dev libncursesw5-dev libsqlite3-dev
```

### Clone and compile
```
git clone https://github.com/lepton9/vbdist.git && cd vbdist
make dep
make
```

## Usage
```
vbdist [options]

  Options:
    -f, --file <file>          Path to textfile
    -d, --database <database>  Path to sqlite database
    -t, --teams <int>          Set number of teams
    -p, --players <int>        Set number of players in a team
    -h, --help                 Display help
```
If you are using a SQLite3 database, you will need both the database and player file.
If only the player file is passed, the format shown below will be used.

## Player file format

### Using a database
List of players IDs in the database.
```
1000
1001
...
1010
```

### Using only a text file
```
# Line starting with '#' is a comment

# Name | Defence Spike Serve Setting Saving Consistency

name1 | 1 2 3 4 5 5
name2 | 5 5 5 5 5 5
...
nameN | 1 1 1 1 1 1
```

### Banning player combinations
Ban player combinations using `!`.
All the players with names on the same line are banned with the first player on 
the line.
Use IDs if using a database; otherwise, use names.
```
!p1 - p2
!p3 - p4 - p5
```

Using `?`, you can ban all the player combinations on the line.
Every player on the line will be in different teams if possible.
```
?p1 - p2 - p3
```

### Forcing teammates
You can force teammates to be on the same team using `+`.
All the players connected are in the same team. Given `TEAM_SIZE` is the maximum 
amount of connected players.
```
+p6 - p7
+p7 - p8
+p9 - p10 - p11
```

