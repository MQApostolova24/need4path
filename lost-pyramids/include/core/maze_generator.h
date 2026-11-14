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

// What was actually placed (small pyramids may have room for fewer).
struct GenerationResult
{
    int secretDoorsPlaced = 0;
    int treasuresPlaced = 0;
    int gemsPlaced = 0;
    bool relicPlaced = false;
};

// Builds a brand-new random pyramid:
//  1. carves corridors with a "recursive backtracker" (a stack: dig forward,
//     step back at dead ends), so every corridor is connected,
//  2. opens burial chambers and adds loops, so there is more than one way around,
//  3. places the exit gate in the bottom-right corner,
//  4. turns some walls into secret doors that are real shortcuts,
//  5. places keys (one per door), the relic, time pickups, traps and treasure.
// Traps are never placed on a shortest route from the entrance to the exit.
bool generateMaze(Maze& maze, int size, const GenerationSettings& settings,
                  std::mt19937& rng, GenerationResult& result);
