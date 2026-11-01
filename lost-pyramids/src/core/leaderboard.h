#pragma once

#include "core/game.h"

#include <string>
#include <vector>

// The Hall of Fame, stored in data/leaderboard.txt as
// "username;location;difficulty;size;score;timeSeconds;moves;stars".
// Older lines without location and stars are still read (as Egypt, 0 stars).

const int FILTER_ALL = -1;

struct ScoreEntry
{
    std::string username;
    Location location = Location::Egypt;
    Difficulty difficulty = Difficulty::Easy;
    int mazeSize = 0;
    int score = 0;
    float timeSeconds = 0.0f;
    int moves = 0;
    int stars = 0;
};

// Builds an entry from a finished game.
ScoreEntry createScoreEntry(const GameState& state, const std::string& username);

// Loads and sorts all entries. Returns false if the file does not exist yet.
bool loadLeaderboard(const std::string& filePath, std::vector<ScoreEntry>& entries);

// Appends one entry to the file.
bool saveScore(const std::string& filePath, const ScoreEntry& entry);

// Highest score first; equal scores - faster time first.
void sortLeaderboard(std::vector<ScoreEntry>& entries);

// The best "limit" entries. Pass FILTER_ALL or a Location / Difficulty value (as int).
std::vector<ScoreEntry> getTopScores(const std::vector<ScoreEntry>& entries, int locationFilter,
                                     int difficultyFilter, int limit);

// The player's fastest escape and most stars in one location (any difficulty).
// Returns false if the player has never escaped from there.
bool getPersonalBest(const std::vector<ScoreEntry>& entries, const std::string& username, Location location,
                     float& bestTime, int& bestStars);
