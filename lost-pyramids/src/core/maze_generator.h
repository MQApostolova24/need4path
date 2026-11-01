#pragma once

#include "core/maze.h"

#include <random>

// How the pyramid should be built.
struct GenerationSettings
{
    int secretDoors = 0;
    int treasures = 0;
    int gems = 0;
    int timePickups = 0;
    int traps = 0;
    int chambers = 0;          // open burial chambers (rooms)
    float loopFraction = 0.0f; // part of the dead ends that get a second way out (0 = none, 1 = all)
    bool relic = false;        // a legendary relic hidden in a far dead end
};

// What was actually built.
struct GenerationResult
{
    int secretDoorsPlaced = 0;
    int keysPlaced = 0;
    int treasuresPlaced = 0;
    int gemsPlaced = 0;
    int timePickupsPlaced = 0;
    int trapsPlaced = 0;
    int chambersPlaced = 0;
    int loopsAdded = 0;
    bool relicPlaced = false;
    GridPos exit;
};

// Builds a brand-new random pyramid:
//  1. carves corridors with the "growing tree" algorithm - a mix of long winding
//     corridors (newest cell) and branching halls (random cell),
//  2. opens burial chambers and adds loops, so there is more than one way around,
//  3. places the exit gate in the bottom-right corner,
//  4. turns some walls into secret doors that are real shortcuts,
//  5. places keys (one per door), the relic, time pickups, traps and treasure.
// Traps are never placed on a shortest route from the entrance to the exit.
bool generateMaze(Maze& maze, int size, const GenerationSettings& settings,
                  std::mt19937& rng, GenerationResult& result);
