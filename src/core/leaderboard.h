#pragma once

#include "core/game.h"

#include <string>
#include <vector>

// The Hall of Fame, stored in data/leaderboard.txt as
// "username;difficulty;size;score;timeSeconds;moves".

struct ScoreEntry
{
    std::string username;
    Difficulty difficulty = Difficulty::Easy;
    int mazeSize = 0;
    int score = 0;
    float timeSeconds = 0.0f;
    int moves = 0;
};

// Builds an entry from a finished game.
ScoreEntry createScoreEntry(const GameState& state, const std::string& username);

// Loads and sorts all entries. Returns false if the file does not exist yet.
bool loadLeaderboard(const std::string& filePath, std::vector<ScoreEntry>& entries);

// Appends one entry to the file.
bool saveScore(const std::string& filePath, const ScoreEntry& entry);

// Highest score first; equal scores - faster time first.
void sortLeaderboard(std::vector<ScoreEntry>& entries);

// The best "limit" entries, optionally only for one difficulty.
std::vector<ScoreEntry> getTopScores(const std::vector<ScoreEntry>& entries, bool filterByDifficulty,
                                     Difficulty difficulty, int limit);
