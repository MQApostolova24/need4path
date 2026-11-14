#pragma once

#include "core/leaderboard.h"

#include <string>
#include <vector>

// The Hall of Fame in data/leaderboard.txt, one line per escape:
//   "username;location;difficulty;size;score;timeSeconds;moves;stars"

// Loads and sorts all entries. A missing file means an empty Hall of Fame.
std::vector<ScoreEntry> loadLeaderboard(const std::string& filePath);

// Appends one entry to the file.
bool saveScore(const std::string& filePath, const ScoreEntry& entry);
