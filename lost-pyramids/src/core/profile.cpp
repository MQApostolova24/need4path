#include "core/profile.h"

#include "core/skins.h"

namespace
{
const int RELIC_SELL_PRICES[DIFFICULTY_COUNT] = {60, 120, 250};
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

int relicIndex(Location location, Difficulty difficulty)
{
    return static_cast<int>(location) * DIFFICULTY_COUNT + static_cast<int>(difficulty);
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
