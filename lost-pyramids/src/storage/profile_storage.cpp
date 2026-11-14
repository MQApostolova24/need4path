#include "storage/profile_storage.h"

#include "core/utils.h"
#include "storage/file_utils.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <vector>

namespace
{
const std::size_t PROFILE_FIELDS = 7;

// Returns false for a damaged line, which is then ignored instead of crashing the game.
bool parseProfileLine(const std::string& line, PlayerProfile& profile)
{
    const std::vector<std::string> parts = splitString(line, ';');
    if (parts.size() != PROFILE_FIELDS || parts[0].empty())
    {
        return false;
    }

    const std::vector<std::string> counts = splitString(parts[5], ',');
    if (counts.size() != static_cast<std::size_t>(RELIC_TYPES))
    {
        return false;
    }

    try
    {
        profile.username = parts[0];
        profile.coins = std::max(0, std::stoi(parts[1]));
        profile.equippedSkin = std::stoi(parts[2]);
        profile.ownedSkins = static_cast<unsigned int>(std::stoul(parts[3])) | 1u;
        profile.discoveredRelics = static_cast<unsigned int>(std::stoul(parts[4]));
        for (int i = 0; i < RELIC_TYPES; ++i)
        {
            profile.relicCounts[i] = std::max(0, std::stoi(counts[static_cast<std::size_t>(i)]));
        }
        profile.expeditionsWithoutRelic = std::max(0, std::stoi(parts[6]));
    }
    catch (const std::exception&)
    {
        return false;
    }

    if (!profile.ownsSkin(profile.equippedSkin))
    {
        profile.equippedSkin = 0;
    }
    return true;
}

std::string profileToLine(const PlayerProfile& profile)
{
    std::string counts;
    for (int i = 0; i < RELIC_TYPES; ++i)
    {
        counts += (i > 0 ? "," : "") + std::to_string(profile.relicCounts[i]);
    }
    return profile.username + ';' + std::to_string(profile.coins) + ';' + std::to_string(profile.equippedSkin) + ';' +
           std::to_string(profile.ownedSkins) + ';' + std::to_string(profile.discoveredRelics) + ';' + counts + ';' +
           std::to_string(profile.expeditionsWithoutRelic);
}

bool isLineOf(const std::string& line, const std::string& lowerUsername)
{
    const std::vector<std::string> parts = splitString(line, ';');
    return !parts[0].empty() && toLowerCopy(parts[0]) == lowerUsername;
}
}  // namespace

PlayerProfile loadProfile(const std::string& filePath, const std::string& username)
{
    std::vector<std::string> lines;
    readLines(filePath, lines);

    const std::string wanted = toLowerCopy(username);
    for (const std::string& line : lines)
    {
        PlayerProfile loaded;
        if (isLineOf(line, wanted) && parseProfileLine(line, loaded))
        {
            loaded.username = username;
            return loaded;
        }
    }

    PlayerProfile fresh;
    fresh.username = username;
    return fresh;
}

bool saveProfile(const std::string& filePath, const PlayerProfile& profile)
{
    // Keep everyone else's lines, replace this player's line.
    std::vector<std::string> lines;
    readLines(filePath, lines);
    const std::string wanted = toLowerCopy(profile.username);
    lines.erase(std::remove_if(lines.begin(), lines.end(),
                               [&](const std::string& line) { return line.empty() || isLineOf(line, wanted); }),
                lines.end());
    lines.push_back(profileToLine(profile));

    if (!ensureParentDirectory(filePath))
    {
        return false;
    }
    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }
    for (const std::string& line : lines)
    {
        file << line << '\n';
    }
    return file.good();
}
