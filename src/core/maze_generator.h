#pragma once

#include "core/maze.h"

#include <random>

// How many special elements the generator should try to place.
struct GenerationSettings
{
    int secretDoors = 0;
    int treasures = 0;
    int traps = 0;
};

// How many were actually placed (small tombs may have room for fewer).
struct GenerationResult
{
    int secretDoorsPlaced = 0;
    int ankhsPlaced = 0;
    int treasuresPlaced = 0;
    int trapsPlaced = 0;
};

// Builds a brand-new random tomb:
//  1. carves corridors with a randomized depth-first search (perfect maze),
//  2. connects the Sun Gate in the bottom-right corner,
//  3. turns some walls into secret doors that are real shortcuts,
//  4. places one Ankh key per secret door, treasure and traps.
// Traps are never placed on the true route from the entrance to the exit.
bool generateMaze(Maze& maze, int size, const GenerationSettings& settings,
                  std::mt19937& rng, GenerationResult& result);
