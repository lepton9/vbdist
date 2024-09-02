
# Volleyball team maker
Makes N teams of M players based on given attributes

## Compiling and running
```
make
./bin/vbdist playerFile.txt TEAMS TEAM_SIZE
```
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

### Forcing teammates
You can force teammates to be on the same team using '+'. All the players connected are in the same team. Given TEAM_SIZE is the maximum amount of connected players.
```
+name6 - name7
+name7 - name8
+name9 - name10 - name11
```

