#include "storage/score_storage.h"

#include "core/utils.h"
#include "storage/file_utils.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>

namespace
{
const std::size_t SCORE_FIELDS = 8;

// Returns false for a damaged line, which is then skipped instead of crashing the game.
bool parseScoreLine(const std::string& line, ScoreEntry& entry)
{
    const std::vector<std::string> parts = splitString(line, ';');
    if (parts.size() != SCORE_FIELDS || parts[0].empty() || !locationFromName(parts[1], entry.location) ||
        !difficultyFromName(parts[2], entry.difficulty))
    {
        return false;
    }

    try
    {
        entry.username = parts[0];
        entry.mazeSize = std::stoi(parts[3]);
        entry.score = std::stoi(parts[4]);
        entry.timeSeconds = std::stof(parts[5]);
        entry.moves = std::stoi(parts[6]);
        entry.stars = std::clamp(std::stoi(parts[7]), 0, 3);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}
}  // namespace

std::vector<ScoreEntry> loadLeaderboard(const std::string& filePath)
{
    std::vector<std::string> lines;
    std::vector<ScoreEntry> entries;
    readLines(filePath, lines);

    for (const std::string& line : lines)
    {
        ScoreEntry entry;
        if (parseScoreLine(line, entry))
        {
            entries.push_back(entry);
        }
    }

    sortLeaderboard(entries);
    return entries;
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

    file << entry.username << ';' << locationName(entry.location) << ';' << difficultyName(entry.difficulty) << ';'
         << entry.mazeSize << ';' << entry.score << ';' << std::fixed << std::setprecision(2) << entry.timeSeconds
         << ';' << entry.moves << ';' << entry.stars << '\n';
    return file.good();
}
