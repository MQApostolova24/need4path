#include "core/leaderboard.h"

#include "core/utils.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>

namespace
{
bool parseScoreLine(const std::string& line, ScoreEntry& entry)
{
    const std::vector<std::string> parts = splitString(line, ';');
    if (parts.size() != 6 || parts[0].empty())
    {
        return false;
    }

    entry.username = parts[0];
    if (!difficultyFromName(parts[1], entry.difficulty))
    {
        return false;
    }

    try
    {
        entry.mazeSize = std::stoi(parts[2]);
        entry.score = std::stoi(parts[3]);
        entry.timeSeconds = std::stof(parts[4]);
        entry.moves = std::stoi(parts[5]);
    }
    catch (const std::exception&)
    {
        return false;  // a damaged line is skipped instead of crashing the game
    }
    return true;
}
}  // namespace

ScoreEntry createScoreEntry(const GameState& state, const std::string& username)
{
    ScoreEntry entry;
    entry.username = username;
    entry.difficulty = state.difficulty;
    entry.mazeSize = state.maze.getSize();
    entry.score = calculateScore(state);
    entry.timeSeconds = state.elapsedSeconds;
    entry.moves = state.moves;
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

    file << entry.username << ';' << difficultyName(entry.difficulty) << ';' << entry.mazeSize << ';'
         << entry.score << ';' << std::fixed << std::setprecision(2) << entry.timeSeconds << ';'
         << entry.moves << '\n';
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

std::vector<ScoreEntry> getTopScores(const std::vector<ScoreEntry>& entries, bool filterByDifficulty,
                                     Difficulty difficulty, int limit)
{
    std::vector<ScoreEntry> result;
    for (const ScoreEntry& entry : entries)
    {
        if (static_cast<int>(result.size()) >= limit)
        {
            break;
        }
        if (!filterByDifficulty || entry.difficulty == difficulty)
        {
            result.push_back(entry);
        }
    }
    return result;
}
