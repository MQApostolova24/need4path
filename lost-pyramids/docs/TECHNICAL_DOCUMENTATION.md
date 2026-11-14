# Lost Pyramids - Technical Documentation

Project: **"Labyrinth"** (C++, raylib 6.0)
Team: _fill in the four names and classes_

| Role | Name | Parts of the project |
|---|---|---|
| SCRUM Trainer | _name_ | planning, GitHub board, presentation |
| Backend Developer | _name_ | `core` and `storage` - game logic and data files |
| Frontend Developer | _name_ | `src/ui` - screens and graphics |
| QA | _name_ | `tests/`, `docs/QA_Test_Cases.xlsx` |

---

## 1. The game

**Lost Pyramids** is a maze game. The player is an explorer sent to three pyramids: in **Egypt**, **Mexico** and **Cambodia**. Every expedition builds a new labyrinth. The explorer starts in the **top-left corner (0, 0)** and must reach the **exit gate in the bottom-right corner** before the location's timer runs out.

### Rules

1. Move with the **arrow keys** (W A S D also work). Walls cannot be crossed.
2. **Secret doors** are walls with a carving. A **key** (Ankh / Jade glyph / Lotus key) opens one door, and every door is a shortcut to the exit.
3. Every location has a **timer**: Sands of the Curse (Egypt), Solar Eclipse (Mexico, less time), Monsoon Flood (Cambodia, more time). Time pickups add 15 seconds. **P** pauses the game and the timer. The game also pauses when its window loses focus or is minimized.
4. **Traps** cost a life. They are only placed off the shortest route, so every pyramid can be won.
5. On **Hard** a **guardian** (mummy, jaguar spirit or naga) hunts the explorer. Touching it costs a life.
6. **Win:** reach the exit gate. **Lose:** no lives left, or the timer reaches 0.
7. **T** switches between the picture view and the **text view** (`#` wall, `.` corridor, `P` player, `E` exit ...).

### Difficulties

| | Easy | Medium | Hard |
|---|---|---|---|
| Light | the whole pyramid | torch circle | flashlight beam (walls stay faintly visible) |
| Traps | none | some | many |
| Guardian | no | no | yes |
| Lives | 3 | 3 | 2 |
| Timer | 90 s + 2.5 s per route tile | 45 s + 1.5 s per tile | 25 s + 0.9 s per tile |
| Score bonus | x1.0 | x1.5 | x2.2 |

### Rewards

- **Score** = route length x 10 + treasure x 50 + gems x 150 + relic 500 + lives x 100 + seconds left x 5, multiplied by the difficulty bonus.
- **Stars:**
  - 1 star for escaping.
  - +1 for the relic or for half of the treasure.
  - +1 for keeping all lives with a third of the time left.
- **Coins:** escaping turns treasure into coins (2 per treasure, 8 per gem).
- **Relics** are rare. Each expedition has a 1 in 5 chance of hiding one, and one is guaranteed after 3 expeditions without a relic. That works out to about one relic every 3 expeditions.
  - A relic counts only if you escape with it.
  - Relics sell for 60 / 120 / 250 coins (Easy / Medium / Hard).
- **Trading Post:** sell relics and buy explorer skins (100 - 800 coins).

### Screens

Log In / Sign Up -> Main Menu -> Choose Location -> Prepare Expedition -> Game (Pause) -> Results.
The main menu also leads to the Trading Post, How to Play and the Hall of Fame.
Screenshots of every screen are in `docs/screenshots`.

---

## 2. Architecture

Headers (`.h`) are in `include/`, source files (`.cpp`) in `src/`, in matching sub-folders. A file includes a header by its path inside `include/`, e.g. `#include "core/maze.h"`. `CMakeLists.txt` adds `include/` to the include path.

```
include/core/    + src/core/      GAME LOGIC - no graphics, no files, no cin/cout (requirement 10)
include/storage/ + src/storage/   SAVING / LOADING the text files in data/
include/ui/      + src/ui/        USER INTERFACE - raylib drawing and keyboard/mouse input
src/main.cpp                      window and main loop (60 frames per second)
tests/                            QA test functions
assets/fonts/                     themed fonts (SIL Open Font License)
docs/                             this document, the QA workbook, screenshots
data/                             created at runtime: users.txt, leaderboard.txt, profiles.txt
```

The code is built as three layers: `core` (logic), then `storage` (uses core), then the game itself (uses both, plus raylib). Each layer only calls the layers below it, so the UI calls the logic and the logic never calls the UI. For example, the game screen reads a key, calls `movePlayer(state, Direction::Up)` and gets back a `GameEvent` (`BlockedByWall`, `KeyCollected`, `ReachedExit` ...), which it turns into a message. Because of this split, the QA tests can check the logic without opening a window.

**Main loop:** every frame `runCurrentScreen(app)` calls the function of the open screen (`runLoginScreen`, `runPlayingScreen` ...). The `App` structure keeps everything between frames. `changeScreen` switches screens and loads the data each screen needs.

---

## 3. Backend Developer - game logic (`core`) and data files (`storage`)

| File | What it does |
|---|---|
| `maze.h/.cpp` | `Maze` class: the dynamic 2D array, path search (BFS), line of sight |
| `maze_generator.h/.cpp` | builds a new pyramid every game |
| `game.h/.cpp` | rules: movement, doors, rewards, traps, guardian, timer, light, score, stars |
| `location.h/.cpp` | data of the three locations (names, timers, guardians, relics) |
| `profile.h/.cpp` | coins, relics, skins, relic chance, Trading Post rules |
| `skins.h/.cpp` | names and prices of the six skins |
| `auth.h/.cpp` | salted password hashes, finding a user |
| `leaderboard.h/.cpp` | Hall of Fame: sort, filter, personal bests |
| `validation.h/.cpp` | checks for username, password and pyramid size |
| `config.h`, `utils.h/.cpp` | constants and small helpers |

The files in `data/` are read and written only by the **storage** layer:

| File | What it does |
|---|---|
| `storage/account_storage.h/.cpp` | `users.txt`: load accounts, sign up, log in |
| `storage/score_storage.h/.cpp` | `leaderboard.txt`: load and save Hall of Fame entries |
| `storage/profile_storage.h/.cpp` | `profiles.txt`: load and save coins, skins and relics |
| `storage/file_utils.h/.cpp` | read all lines of a file, create the `data/` folder |

### Dynamic memory (requirement 2)

The pyramid is a **dynamic two-dimensional array** whose size the player chooses at runtime (10 - 50):

```cpp
cells = new Cell*[size];              // array of row pointers
for (int row = 0; row < size; ++row)
    cells[row] = new Cell[size];      // every row is its own array
...
for (int row = 0; row < size; ++row)
    delete[] cells[row];              // Maze::release(), called by the destructor
delete[] cells;
```

Copying a `Maze` is disabled, so two objects can never free the same memory. Test T_033 builds and frees 200 mazes of different sizes. The tests also run with AddressSanitizer, which reports any leak or invalid access.

### Data types

- `enum class` for fixed choices: `TileType`, `ItemType`, `Direction`, `Difficulty`, `Location`, `GameEvent`.
- `struct` for records: `Cell`, `GridPos`, `GameState`, `ScoreEntry`, `PlayerProfile`.
- `float` for time, `int` for counts.
- `std::vector<int>` for the BFS distance tables, `std::string` for text.
- `std::mt19937` for random numbers. It is seeded from `std::random_device` + the clock, so every run is different (requirement 7).

### Building a pyramid (`generateMaze`)

1. **Growing tree algorithm.** A list of cells that can still grow is kept.
   - 75% of the time the newest cell is used, which makes long winding corridors.
   - 25% of the time a random cell is used, which makes branches.
   - Walls are knocked down two tiles at a time, so walls always stay between corridors.
2. **Even sizes:** the last row and column get small nooks and passages, so that edge is not a solid band.
3. **Burial chambers:** 0 - 4 open 3x3 or 5x3 rooms, depending on the size.
4. **Loops:** some dead ends get a second way out, so there is more than one route.
   - Easy 35%, Medium 20%, Hard 12%. Fewer loops make the pyramid harder.
5. **Exit** in the bottom-right corner (requirement 3).
6. **Secret doors:** a BFS measures the walking distance from the start to every tile. A wall that separates two corridors which are far apart along the maze becomes a door, so every door is a real shortcut (requirement 8).
7. **Items:**
   - The relic (if rolled) goes to the end of the longest detour from the true route.
   - Keys (one per door), time pickups and treasure go anywhere; traps and gems go only on tiles that are not on a shortest route.
   - A tile is on a shortest route when `distanceFromStart + distanceFromExit == routeLength`.

### Movement and rules (`movePlayer`, `updateGame`)

`movePlayer`:
1. Turns the explorer to the new direction.
2. Blocks walls and locked doors.
3. Uses a key on a secret door.
4. Picks up the item on the new tile.
5. Checks the exit and the guardian.

`updateGame(deltaSeconds)` advances the timer and moves the guardian. One call never advances more than 0.1 s, so a frozen window (for example while it is dragged) cannot make the timer jump or the guardian sprint. The guardian uses a BFS from the player: within its chase range it steps closer, otherwise it wanders.

### Light (`getLightLevel`, `isCellLit`)

- **Easy:** everything is lit.
- **Medium:** a smooth circle around the explorer.
- **Hard:** a flashlight cone (about 31 degrees each side, 7.5 tiles long) plus a small glow.
  - `hasLineOfSight` walks along the line from the explorer to a tile in 0.2-tile steps. Any wall on the way blocks the light.
  - A wall is lit when the corridor tile next to it is lit, so the beam shows the walls of the corridor.

### Accounts and files

- `users.txt`: `username;salt;hash`. The password is hashed with salted FNV-1a, 2000 rounds. The password itself is never saved.
- `leaderboard.txt`: `username;location;difficulty;size;score;time;moves;stars`.
- `profiles.txt`: `username;coins;equippedSkin;ownedSkins;discoveredRelics;r0..r8;expeditionsWithoutRelic`.
  - `ownedSkins` and `discoveredRelics` are bit masks: bit *i* = skin *i* / relic *i*.
- Damaged lines are skipped instead of crashing the game.

### Validation (requirement 12)

- **Username:** 3 - 16 letters, digits or `_`, starting with a letter, unique.
- **Password:** 6 - 24 characters with a letter and a digit, no spaces. It is typed twice when signing up.
- **Pyramid size:** digits only, 10 - 50.

---

## 4. Frontend Developer - user interface (`src/ui`)

| File | What it does |
|---|---|
| `app.h/.cpp` | `App` state, `changeScreen`, `beginExpedition` (rolls the relic chance and saves the profile) |
| `screens_auth.cpp` | Log In, Sign Up |
| `screens_menu.cpp` | Main Menu (requirement 11), How to Play, Hall of Fame |
| `screens_expedition.cpp` | Choose Location, Prepare Expedition (size + difficulty) |
| `screens_game.cpp` | Game screen, side panel, Pause, Results |
| `screens_trading.cpp` | Trading Post (sell relics, buy skins) |
| `maze_renderer.h/.cpp` | draws the maze: tiles, items, light, weather, text view |
| `sprites.h/.cpp` | keys, doors, rewards, traps, guardians, exits, explorer skins, timer icons |
| `location_theme.h/.cpp` | colours and landscapes of the three locations |
| `widgets.h/.cpp` | panels, buttons, tabs, text fields, stars, hearts, coins |
| `fonts.h/.cpp` | loads the six fonts, text drawing helpers |
| `theme.h`, `draw_utils.h/.cpp` | colours, small drawing helpers |

### Showing the maze (requirements 4 - 6)

- **Picture view:** the maze is drawn from above after every move. Walls are 3D-looking stone blocks; floors are slabs with shadows. Items, the guardian and the explorer are drawn on top.
- **Text view (T):** the same maze as characters (`drawTextMaze`), e.g. `#` wall, `.` corridor, `P` player, `E` exit, `D` secret door.
- **Darkness:** on Medium and Hard, the darkness of every tile is blended between its four corners, so the light has soft edges. The layout and the exit gate always stay visible.

### Input

- `readMovementInput` turns arrow keys into a `Direction`. Holding a key repeats the move every 0.085 s.
- Menus use the arrow keys + Enter or the mouse (`handleMenuNavigation`).
- Text fields accept typing, Backspace and Tab (`updateTextFields`).
- Esc goes back or pauses.

### Graphics without image files

Everything is drawn with raylib shapes: triangles, circles, rectangles and lines. Each location has its own palette and landscape (`drawLocationScene`). The fonts are loaded with mipmaps for smooth scaling, and raylib's own font is used if a font file is missing.

### The suggested functions from the assignment

| Suggested function | In this project |
|---|---|
| Menu and introduction | `runMainMenuScreen`, `runChooseLocationScreen` |
| Game help | `runRulesScreen` |
| Showing the maze | `drawMazeScene`, `drawTextMaze` |
| Getting the direction | `readMovementInput` |
| Info about the current location | side panel: location, "Row r, Column c" |
| Info about an element | element list in How to Play |
| Current inventory | side panel: keys, treasure, gems, relic |
| Prompt for the next command | `runPauseMenu`, the results buttons |

---

## 5. QA - testing

There are two kinds of testing.

**1. Automated test functions** (`tests/test_main.cpp`, program `lost_pyramids_tests`)

- 33 test functions (T_001 - T_033) with 238 checks. They call the logic directly.
- Each prints PASSED or FAILED, and the program ends with a summary (`Tests: 33 / 33 passed`).
- They cover:
  - pyramid sizes, the corner exit, generation quality (connected, loops, chambers);
  - doors, keys, traps, rewards, timers, guardians and light;
  - score and stars, validation, accounts, the Hall of Fame;
  - profiles, relic rarity, selling and skins, and dynamic memory.
- They are also run with AddressSanitizer and UndefinedBehaviorSanitizer to catch memory errors.

**2. Manual test cases** (`docs/QA_Test_Cases.xlsx`)

- One tab per screen, in the team's test-case format: Test Details, steps, test data, expectations, actual result, status.
- 20 test cases with 69 steps.
- The Summary tab counts PASSED / FAILED / NOT RUN automatically.
- Fill in "Created by", "Actual Result" and "Status" while testing the game.

---

## 6. SCRUM Trainer - planning and delivery

- **Timeline:** 21.10.2026 - 15.11.2026, defense 16 - 17.11.
- **GitHub:**
  - one repository, with access for every member and the teacher;
  - a project board with columns To Do / In Progress / Review / Done;
  - one issue per feature, assigned to its owner.
- **Commits:** a short title in the imperative plus a description, e.g. `Add flashlight light to Hard mode`.
- **Suggested sprints:**
  1. Maze class and generator, console prototype.
  2. raylib screens, accounts, menu.
  3. Locations, timers, rewards, Trading Post.
  4. Testing, documentation, presentation.
- **Presentation:** game idea, rules, demo, maze generation algorithm (diagram), code structure, tests, what each member did.

---

## 7. Building

1. Install Visual Studio 2022/2026 with "Desktop development with C++".
2. *File -> Open -> Folder* and choose the project folder. raylib 6.0 is downloaded automatically.
3. Run **lost_pyramids.exe** for the game and **lost_pyramids_tests.exe** for the tests.

The code compiles without warnings with `-Wall -Wextra -Wpedantic -Wconversion -Wshadow` (GCC).
