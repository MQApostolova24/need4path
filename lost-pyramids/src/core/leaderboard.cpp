#include "core/leaderboard.h"

#include "core/utils.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>

namespace
{
// Converts the number fields; returns false if any of them is damaged.
bool parseNumbers(const std::string& size, const std::string& score, const std::string& time,
                  const std::string& moves, ScoreEntry& entry)
{
    try
    {
        entry.mazeSize = std::stoi(size);
        entry.score = std::stoi(score);
        entry.timeSeconds = std::stof(time);
        entry.moves = std::stoi(moves);
    }
    catch (const std::exception&)
    {
        return false;  // a damaged line is skipped instead of crashing the game
    }
    return true;
}

bool parseScoreLine(const std::string& line, ScoreEntry& entry)
{
    const std::vector<std::string> parts = splitString(line, ';');
    if (parts.empty() || parts[0].empty())
    {
        return false;
    }
    entry.username = parts[0];

    if (parts.size() == 6)
    {
        // Old format: username;difficulty;size;score;time;moves
        entry.location = Location::Egypt;
        entry.stars = 0;
        return difficultyFromName(parts[1], entry.difficulty) &&
               parseNumbers(parts[2], parts[3], parts[4], parts[5], entry);
    }

    if (parts.size() != 8)
    {
        return false;
    }

    if (!locationFromId(parts[1], entry.location) || !difficultyFromName(parts[2], entry.difficulty) ||
        !parseNumbers(parts[3], parts[4], parts[5], parts[6], entry))
    {
        return false;
    }

    try
    {
        entry.stars = std::clamp(std::stoi(parts[7]), 0, 3);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}
}  // namespace

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

bool loadLeaderboard(const std::string& filePath, std::vector<ScoreEntry>& entries)
{
    entries.clear();

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        ScoreEntry entry;
        if (parseScoreLine(line, entry))
        {
            entries.push_back(entry);
        }
    }

    sortLeaderboard(entries);
    return true;
}

bool saveScore(const std::string& filePath, const ScoreEntry& entry)
{
    if (!ensureParentDirectory(filePath))
    {
        return false;
    }

    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open())
    {
        return false;
    }

    file << entry.username << ';' << locationId(entry.location) << ';' << difficultyName(entry.difficulty) << ';'
         << entry.mazeSize << ';' << entry.score << ';' << std::fixed << std::setprecision(2) << entry.timeSeconds
         << ';' << entry.moves << ';' << entry.stars << '\n';
    return file.good();
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
