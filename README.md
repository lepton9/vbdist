
# Volleyball team maker
Makes `N` teams of `M` players based on given attributes

## Installing and compiling
### Install dependencies
  *Arch*:
```
sudo pacman -S ncurses
```
  *Ubuntu/Debian*:
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### Clone and compile
```
git clone https://github.com/lepton9/vbdist.git && cd vbdist
make
```

## Running the program
```
./bin/vbdist playerFile.txt TEAMS TEAM_SIZE PRINT_MODE
```
`TEAMS`:
  - `N` amount of teams

`TEAM_SIZE`:
  - `M` players in a team

`PRINT_MODE`: (optional)
  - `0` (Minimal printing, default)
  - `1` (Print without player ratings)
  - `2` (Print all)

## Player file format
```
# Line starting with '#' is a comment

# Name | Defence Spike Serve Setting Saving Consistency

name1 | 1 2 3 4 5 5
name2 | 5 5 5 5 5 5
...
nameN | 1 1 1 1 1 1
```

### Banning player combinations
Ban player combinations using '!'. All the players with names on the same line are banned with the first player on the line.
```
!name1 - name2
!name3 - name4 - name5
```

Using `?`, you can ban all the player combinations on the line. Every player on 
the line will be in different teams if possible.
```
?name1 - name2 - name3
```

### Forcing teammates
You can force teammates to be on the same team using '+'. All the players connected are in the same team. Given `TEAM_SIZE` is the maximum amount of connected players.
```
+name6 - name7
+name7 - name8
+name9 - name10 - name11
```

