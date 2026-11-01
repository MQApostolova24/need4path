#include "core/profile.h"

#include "core/skins.h"
#include "core/utils.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <vector>

namespace
{
const int RELIC_SELL_PRICES[DIFFICULTY_COUNT] = {60, 120, 250};
const std::size_t PROFILE_FIELDS = 7;

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
        return false;  // a damaged line is ignored instead of crashing the game
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
}  // namespace

bool PlayerProfile::ownsSkin(int skinId) const
{
    return isValidSkin(skinId) && (ownedSkins & (1u << skinId)) != 0;
}

bool PlayerProfile::hasDiscovered(Location location, Difficulty difficulty) const
{
    return (discoveredRelics & (1u << relicIndex(location, difficulty))) != 0;
}

int PlayerProfile::relicCount(Location location, Difficulty difficulty) const
{
    return relicCounts[relicIndex(location, difficulty)];
}

int PlayerProfile::totalRelics() const
{
    int total = 0;
    for (int count : relicCounts)
    {
        total += count;
    }
    return total;
}

int PlayerProfile::discoveredCount() const
{
    int total = 0;
    for (int i = 0; i < RELIC_TYPES; ++i)
    {
        total += (discoveredRelics & (1u << i)) != 0 ? 1 : 0;
    }
    return total;
}

int relicIndex(Location location, Difficulty difficulty)
{
    return static_cast<int>(location) * DIFFICULTY_COUNT + static_cast<int>(difficulty);
}

bool loadProfile(const std::string& filePath, const std::string& username, PlayerProfile& profile)
{
    profile = PlayerProfile{};
    profile.username = username;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return true;  // no file yet = a fresh profile
    }

    const std::string wanted = toLowerCopy(username);
    std::string line;
    while (std::getline(file, line))
    {
        PlayerProfile loaded;
        if (parseProfileLine(line, loaded) && toLowerCopy(loaded.username) == wanted)
        {
            profile = loaded;
            profile.username = username;
            return true;
        }
    }
    return true;
}

bool saveProfile(const std::string& filePath, const PlayerProfile& profile)
{
    // Read all lines, replace this player's line, write everything back.
    std::vector<std::string> lines;
    {
        std::ifstream file(filePath);
        std::string line;
        const std::string wanted = toLowerCopy(profile.username);
        while (std::getline(file, line))
        {
            const std::vector<std::string> parts = splitString(line, ';');
            if (!parts.empty() && !parts[0].empty() && toLowerCopy(parts[0]) != wanted)
            {
                lines.push_back(line);
            }
        }
    }
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

bool rollRelicSpawn(PlayerProfile& profile, std::mt19937& rng)
{
    bool spawn = profile.expeditionsWithoutRelic >= RELIC_GUARANTEED_AFTER;
    if (!spawn)
    {
        std::uniform_int_distribution<int> roll(1, RELIC_CHANCE_ONE_IN);
        spawn = roll(rng) == 1;
    }
    profile.expeditionsWithoutRelic = spawn ? 0 : profile.expeditionsWithoutRelic + 1;
    return spawn;
}

int coinsEarned(const GameState& state)
{
    if (state.status != GameStatus::Won)
    {
        return 0;
    }
    return state.treasuresCollected * COINS_PER_TREASURE + state.gemsCollected * COINS_PER_GEM;
}

int collectRewards(PlayerProfile& profile, const GameState& state)
{
    const int coins = coinsEarned(state);
    profile.coins += coins;
    if (state.status == GameStatus::Won && state.relicFound)
    {
        const int index = relicIndex(state.location, state.difficulty);
        profile.relicCounts[index]++;
        profile.discoveredRelics |= 1u << index;
    }
    return coins;
}

int relicSellPrice(Difficulty difficulty)
{
    return RELIC_SELL_PRICES[static_cast<int>(difficulty)];
}

bool sellRelic(PlayerProfile& profile, Location location, Difficulty difficulty)
{
    const int index = relicIndex(location, difficulty);
    if (profile.relicCounts[index] <= 0)
    {
        return false;
    }
    profile.relicCounts[index]--;
    profile.coins += relicSellPrice(difficulty);
    return true;
}

PurchaseResult buySkin(PlayerProfile& profile, int skinId)
{
    if (!isValidSkin(skinId))
    {
        return PurchaseResult::InvalidSkin;
    }
    if (profile.ownsSkin(skinId))
    {
        return PurchaseResult::AlreadyOwned;
    }
    const int price = getSkinInfo(skinId).price;
    if (profile.coins < price)
    {
        return PurchaseResult::NotEnoughCoins;
    }
    profile.coins -= price;
    profile.ownedSkins |= 1u << skinId;
    profile.equippedSkin = skinId;  // wear it right away
    return PurchaseResult::Bought;
}

bool equipSkin(PlayerProfile& profile, int skinId)
{
    if (!profile.ownsSkin(skinId))
    {
        return false;
    }
    profile.equippedSkin = skinId;
    return true;
}
