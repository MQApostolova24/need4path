#include "core/leaderboard.h"

#include "core/utils.h"

#include <algorithm>

ScoreEntry createScoreEntry(const GameState& state, const std::string& username)
{
    ScoreEntry entry;
    entry.username = username;
    entry.location = state.location;
    entry.difficulty = state.difficulty;
    entry.mazeSize = state.maze.getSize();
    entry.score = calculateScore(state);
    entry.timeSeconds = state.elapsedSeconds;
    entry.moves = state.moves;
    entry.stars = calculateStars(state);
    return entry;
}

void sortLeaderboard(std::vector<ScoreEntry>& entries)
{
    std::sort(entries.begin(), entries.end(), [](const ScoreEntry& left, const ScoreEntry& right) {
        if (left.score != right.score)
        {
            return left.score > right.score;
        }
        return left.timeSeconds < right.timeSeconds;
    });
}

std::vector<ScoreEntry> getTopScores(const std::vector<ScoreEntry>& entries, int locationFilter,
                                     int difficultyFilter, int limit)
{
    std::vector<ScoreEntry> result;
    for (const ScoreEntry& entry : entries)
    {
        if (static_cast<int>(result.size()) >= limit)
        {
            break;
        }
        const bool locationMatches = locationFilter == FILTER_ALL || static_cast<int>(entry.location) == locationFilter;
        const bool difficultyMatches =
            difficultyFilter == FILTER_ALL || static_cast<int>(entry.difficulty) == difficultyFilter;
        if (locationMatches && difficultyMatches)
        {
            result.push_back(entry);
        }
    }
    return result;
}

bool getPersonalBest(const std::vector<ScoreEntry>& entries, const std::string& username, Location location,
                     float& bestTime, int& bestStars)
{
    bool found = false;
    const std::string wanted = toLowerCopy(username);

    for (const ScoreEntry& entry : entries)
    {
        if (entry.location != location || toLowerCopy(entry.username) != wanted)
        {
            continue;
        }
        if (!found)
        {
            bestTime = entry.timeSeconds;
            bestStars = entry.stars;
            found = true;
        }
        else
        {
            bestTime = std::min(bestTime, entry.timeSeconds);
            bestStars = std::max(bestStars, entry.stars);
        }
    }
    return found;
}
