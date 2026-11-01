# Lost Pyramids

A maze game in C++ with **raylib 6.0** (project "Labyrinth").
You are an explorer sent to three pyramids, in **Egypt**, **Mexico** and **Cambodia**. Every expedition builds a new labyrinth. You start in the **top-left corner** and must reach the **exit gate in the bottom-right corner** before the location's timer runs out.

Full description, rules and the technical details per team role: **[docs/TECHNICAL_DOCUMENTATION.md](docs/TECHNICAL_DOCUMENTATION.md)**
QA test cases: **docs/QA_Test_Cases.xlsx**. Screenshots: **docs/screenshots/**

## Features

- **3 locations**, each with its own story, colours, font, timer, guardian, trap, key, door, rewards and relics.
  - Egypt: Sands of the Curse, the Mummy.
  - Mexico: Solar Eclipse, the Jaguar spirit.
  - Cambodia: Monsoon Flood, the Naga serpent.
- **3 difficulties:**
  - Easy: fully lit.
  - Medium: torch light and traps.
  - Hard: flashlight only, more traps, a guardian and 2 lives.
- **Better generation:**
  - A growing-tree maze that mixes long corridors with branches.
  - Burial chambers, and loops so there is more than one route.
  - Secret doors that are always real shortcuts.
  - Traps never on a shortest route.
- **Rewards:**
  - Treasure and gems turn into **coins** when you escape.
  - Rare **relics** (about one every 3 expeditions).
  - Up to 3 stars per run.
- **Trading Post:** sell relics for coins and buy **explorer skins** (6 skins).
- **Text view (T):** the classic text maze (`#`, `.`, `P`, `E` ...) next to the picture view.
- Log In / Sign Up, Hall of Fame with filters, How to Play, pause menu.

## Controls

| Key | Action |
|---|---|
| Arrow keys (or W A S D) | Move (hold to keep walking); in menus - choose |
| Enter / mouse | Confirm |
| P or Esc | Pause / back |
| T | Text view on/off |
| Tab | Next field, switch Trading Post tab |

## Building

**Visual Studio 2022 / 2026:**
1. *File -> Open -> Folder*, choose this folder. CMake downloads raylib 6.0 the first time, so it needs internet.
2. Run **lost_pyramids.exe** to play and **lost_pyramids_tests.exe** to run the tests.

**Command line:**

```
cmake -S . -B build
cmake --build build --config Release
```

The fonts are copied next to the program automatically. Player data is saved in `data/`, in the folder the game runs from.
On Linux, install first: `sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev`.

## Project structure

```
src/main.cpp     window + main loop
src/core/        game logic (maze, generator, rules, locations, profile, skins, accounts,
                 Hall of Fame, validation) - no graphics, no cin/cout
src/ui/          screens, drawing, fonts, input (raylib)
tests/           33 QA test functions (T_001 - T_033)
assets/fonts/    6 fonts + licences (SIL Open Font License)
docs/            technical documentation, QA workbook, screenshots
```

## Assignment requirements

| # | Requirement | How it is met |
|---|---|---|
| 1 | Maze the player must get through to win | Reach the exit gate (`movePlayer`, `GameEvent::ReachedExit`) |
| 2 | Dynamic arrays, size 10x10 - 50x50 chosen by the player | `Maze` uses `Cell**` with `new[]`/`delete[]`; size chosen on the Prepare Expedition screen |
| 3 | Start at (0,0), exit in the bottom-right corner | Always: start (0,0), exit (size-1, size-1) |
| 4 | Maze shown with the player's position after every move | Redrawn every frame; side panel shows "Row r, Column c" |
| 5 | Text maze, top-down view | Top-down picture view + **text view (T)** with characters |
| 6 | Player sees walls, openings and exit | Easy: all lit. Medium/Hard: the layout stays visible in the dark and the exit always shines |
| 7 | New maze every run | Random seed from `std::random_device` + clock |
| 8 | Arrow keys; no walking through walls except special shortcut doors | Arrow keys; secret doors open with a key and are always shortcuts |
| 9 | Functions for keeping and drawing the maze | `maze.cpp`, `maze_generator.cpp`, `maze_renderer.cpp` |
| 10 | Logic functions without cin/cout | Everything in `src/core` |
| 11 | Menu | Main Menu, Pause menu |
| 12 | Validation of user input | Username, password, pyramid size (`validation.cpp`) |

**Documentation (section 3):** presentation (to do), technical documentation per role with the game rules (`docs/TECHNICAL_DOCUMENTATION.md`), QA test functions and their description (`tests/`, `docs/QA_Test_Cases.xlsx`), and publishing on GitHub (to do). Working language: English.

## Fonts

From Google Fonts, SIL Open Font License 1.1 (licences in `assets/fonts`):

| Font | Used for |
|---|---|
| Cinzel Decorative | Title |
| Alegreya Sans | Normal text |
| Rakkas | Egypt |
| Macondo | Mexico |
| Koulen | Cambodia |
