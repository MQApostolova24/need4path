# Tomb of the Forgotten Pharaoh

A maze game in C++ with **raylib**. You are an explorer trapped in the cursed tomb of Pharaoh Akhen-Ra. Every expedition generates a brand-new labyrinth of sandstone corridors. You enter at the **top-left corner** and must reach the **Sun Gate** in the **bottom-right corner**.

## Story

Pharaoh Akhen-Ra sealed his tomb with a curse: the corridors rearrange themselves for every new visitor. Ancient builders left secret doors marked with the **Eye of Horus**, and only an **Ankh key** can open them. Scarab traps hide in the side halls, and on the hardest level a **mummy** still guards the tomb.

## Game rules

| Element | What it does |
|---|---|
| Explorer (hat) | You. Start at row 1, column 1. |
| Sandstone wall | Cannot be crossed. |
| Secret door (Eye of Horus) | A wall that can be opened with one Ankh key. Always a **shortcut** to the exit. |
| Ankh key (turquoise) | Opens one secret door. There is one key per secret door. |
| Treasure (gold coin) | +75 points (before the difficulty bonus). |
| Scarab trap | Hidden until your torch lights it. Costs one life, then it is harmless. Traps are never on the true route, only in side halls. |
| Mummy (Hard only) | Wanders the tomb and hunts you when you are within 12 steps. Touching it costs one life. |
| Sun Gate | The exit. Reach it to win. |

**You win** when you reach the Sun Gate. **You lose** when you have no lives left or when the curse timer runs out.

### Controls

| Key | Action |
|---|---|
| Arrow keys / W A S D | Move (hold to keep walking) |
| P or Esc | Pause (Resume, New Tomb, Abandon Expedition) |
| Enter / mouse | Choose in menus |
| Tab | Next text field |

### Difficulties

| | Easy - Explorer | Medium - Archaeologist | Hard - Tomb Raider |
|---|---|---|---|
| Light | Whole tomb lit | Torch, radius 6.5 | Torch, radius 4 |
| Traps | None | Some | Many |
| Curse timer | None | 40 s + 1.5 s per route tile | 15 s + 0.8 s per route tile |
| Mummy | No | No | Yes |
| Lives | 3 | 3 | 2 |
| Score bonus | x1.0 | x1.5 | x2.2 |

Outside the torch light the walls, corridors and the exit stay faintly visible, so you always see the layout. Keys and treasure you have already seen stay on the map.

The player also chooses the **tomb size from 10x10 to 50x50**. Bigger tombs have more secret doors, treasure and traps.

### Scoring

`score = (route length x 10 + treasure x 75 + lives left x 100 + time bonus) x difficulty bonus`

The time bonus is 8 points for every second you finish faster than `route length x 1.2 + 10` seconds, so using secret doors pays off. Only successful escapes are saved to the **Hall of Fame**.

## Screens

1. **Log In / Sign Up** - accounts are saved in `data/users.txt`.
2. **Main Menu** - New Expedition, How to Play, Hall of Fame, Log Out, Quit.
3. **New Expedition** - tomb size (10-50) and difficulty.
4. **Game** - the tomb on the left; on the right the explorer's info, position, lives, keys, treasure, moves, time, curse timer, latest event and a legend.
5. **Pause menu**, **Results** (victory or defeat with statistics), **How to Play**, **Hall of Fame** (top 10, filter by difficulty).

## Project structure

```
src/
  main.cpp                 window + main loop
  core/                    GAME LOGIC - no graphics, no cin/cout
    config.h               constants (sizes, limits, file paths)
    maze.h/.cpp            Maze class: dynamic 2D array Cell**, path search (BFS)
    maze_generator.h/.cpp  random tomb generation, secret doors, items, traps
    game.h/.cpp            rules: movement, keys, traps, mummy, timer, score
    auth.h/.cpp            sign up / log in, salted password hashes
    leaderboard.h/.cpp     Hall of Fame: save, load, sort, filter
    validation.h/.cpp      checks for username, password and tomb size
    utils.h/.cpp           small helpers (split, lower-case, mm:ss)
  ui/                      USER INTERFACE - raylib drawing and input
    theme.h                window size and colour palette
    widgets.h/.cpp         background, buttons, text fields, icons
    maze_renderer.h/.cpp   draws the tomb, items, torch light, sprites
    app.h/.cpp             screen state and screen switching
    screens.h              one function per screen
    screens_auth.cpp       Log In, Sign Up
    screens_menu.cpp       Main Menu, New Expedition, How to Play, Hall of Fame
    screens_game.cpp       Game, side panel, Pause, Results
tests/
  test_main.cpp            22 QA test functions (T_001 - T_022) for the logic
data/                      created at runtime: users.txt, leaderboard.txt
```

## How the maze is built

1. `Maze::allocate(size)` creates a `size x size` grid with `new Cell*[size]` and `new Cell[size]` for each row. `Maze::release()` frees it with `delete[]`; the destructor calls it.
2. A **randomized depth-first search** (with an explicit stack) carves corridors. This gives a "perfect" maze: exactly one path between any two tiles.
3. For even sizes the last row/column is linked to the exit.
4. A **breadth-first search** measures distances from the entrance. Walls that separate two corridors far apart along the maze become **secret doors**, so each one is a real shortcut.
5. One Ankh key is placed per secret door, then treasure. Traps go only on tiles that are **not** on the true route, so the game is always winnable.
6. The seed comes from `std::random_device` + the clock, so every run creates a new tomb.

## Building

### Visual Studio 2022 (Windows)

1. Install Visual Studio with **Desktop development with C++** (it includes CMake).
2. *File -> Open -> Folder...* and choose this project folder.
3. Visual Studio configures CMake and downloads raylib 5.5 automatically (first time only, needs internet).
4. Select **tomb.exe** as the startup item and press **F5**. Select **tomb_tests.exe** to run the QA tests.

### Command line (Windows, Linux, macOS)

```
cmake -S . -B build
cmake --build build --config Release
```

Run `build/tomb` (or `build/Release/tomb.exe`) **from the project folder**, so the `data/` folder is created there. Run `build/tomb_tests` for the tests.

On Linux, raylib needs: `sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev`.

## QA tests

`tests/test_main.cpp` contains 22 test functions that check the logic directly:

| ID | Test |
|---|---|
| T_001 | Tomb sizes 10x10 to 50x50 (start, exit, reachable) |
| T_002 | Invalid tomb sizes are rejected |
| T_003 | A new tomb is generated every run |
| T_004 | Secret doors are shortcuts with one key per door |
| T_005 | Traps stay off the true route |
| T_006 | Walls block movement |
| T_007 | Player movement |
| T_008 | Secret door needs an Ankh key |
| T_009 | Collecting Ankh keys and treasure |
| T_010 | Scarab trap costs a life |
| T_011 | Losing all lives ends the game |
| T_012 | Reaching the Sun Gate wins |
| T_013 | Curse timer |
| T_014 | Mummy catches the player |
| T_015 | Torch visibility per difficulty |
| T_016 | Score calculation |
| T_017 | Username validation |
| T_018 | Password validation |
| T_019 | Tomb size input validation |
| T_020 | Sign up and log in |
| T_021 | Hall of Fame save, load, sort, filter |
| T_022 | Dynamic memory is reused and released |

Example output: `Tests: 22 / 22 passed`.

## Data files

- `data/users.txt` - one account per line: `username;salt;passwordHash`. The password itself is never stored.
- `data/leaderboard.txt` - one result per line: `username;difficulty;size;score;timeSeconds;moves`. Damaged lines are skipped.

The password hash (FNV-1a, salted, 2000 rounds) is fine for a school project but is **not** real-world security.

## Validation

- Username: 3-16 characters, letters, digits or `_`, must start with a letter, unique (not case-sensitive).
- Password: 6-24 characters, at least one letter and one digit, no spaces. Must be typed twice when signing up.
- Tomb size: digits only, from 10 to 50; out-of-range values show an error.

## How the assignment requirements are met

| # | Requirement | Where |
|---|---|---|
| 1 | Maze game the player must get through to win | `game.cpp` - `movePlayer`, win at the Sun Gate |
| 2 | Dynamic arrays, size 10x10 to 50x50 chosen by the player | `Maze` (`Cell**`), New Expedition screen |
| 3 | Start at (0,0), exit at the bottom-right | `Maze::getStart`, `Maze::getExit` |
| 4-6 | Show the maze top-down with the player after every move; walls, openings, exit visible | `maze_renderer.cpp` |
| 7 | New maze every run | `maze_generator.cpp`, random seed in `app.cpp` |
| 8 | Arrow keys; no walking through walls except special shortcut doors | `screens_game.cpp`, `movePlayer` |
| 9 | Functions for keeping and drawing the maze | `maze.cpp`, `maze_generator.cpp`, `maze_renderer.cpp` |
| 10 | Logic functions without cin/cout | everything in `src/core` |
| 11 | Menu | Main Menu, Pause menu |
| 12 | Validation of user input | `validation.cpp`, Log In / Sign Up / New Expedition |

Suggested functions from the assignment: menu and introduction (Main Menu), game help (How to Play), drawing the maze (`drawMazeScene`), getting the direction (`readMovementInput`), info about the current location (Position line in the side panel), info about an element (Tomb Guide legend), current inventory (Ankh keys and treasure), menu / prompt for the next command (Pause menu).

## Suggested team split

- **Backend Developer** - `src/core` (maze, generator, game rules, accounts, Hall of Fame).
- **Frontend Developer** - `src/ui` (screens, drawing, input).
- **QA** - `tests/test_main.cpp` and the QA test-case document.
- **Scrum Trainer** - GitHub board, issues, commits, presentation.
