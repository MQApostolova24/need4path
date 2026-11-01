#pragma once

#include "core/config.h"
#include "core/game.h"

#include <random>
#include <string>

// Everything a player owns, stored in data/profiles.txt, one line per player:
//   username;coins;equippedSkin;ownedSkins;discoveredRelics;r0,r1,...,r8;expeditionsWithoutRelic
// ownedSkins and discoveredRelics are bit masks (bit i = skin i / relic i).
// r0..r8 are how many of each relic the player holds (index = location * 3 + difficulty).

struct PlayerProfile
{
    std::string username;
    int coins = 0;
    int equippedSkin = 0;
    unsigned int ownedSkins = 1;         // the first skin is free
    unsigned int discoveredRelics = 0;   // relics the player has ever found (stay known after selling)
    int relicCounts[RELIC_TYPES] = {};   // relics in the player's stash right now
    int expeditionsWithoutRelic = 0;     // for the "guaranteed relic" rule

    bool ownsSkin(int skinId) const;
    bool hasDiscovered(Location location, Difficulty difficulty) const;
    int relicCount(Location location, Difficulty difficulty) const;
    int totalRelics() const;
    int discoveredCount() const;
};

int relicIndex(Location location, Difficulty difficulty);

// Loads a player's profile. A player without a saved line gets a fresh profile.
bool loadProfile(const std::string& filePath, const std::string& username, PlayerProfile& profile);

// Saves (adds or replaces) the player's line, keeping everyone else's lines.
bool saveProfile(const std::string& filePath, const PlayerProfile& profile);

// Decides if the next expedition hides a relic: a 1 in RELIC_CHANCE_ONE_IN chance,
// guaranteed after RELIC_GUARANTEED_AFTER expeditions in a row without one.
// Updates the counter in the profile.
bool rollRelicSpawn(PlayerProfile& profile, std::mt19937& rng);

// Coins for the treasure brought out of a pyramid (0 if the player did not escape).
int coinsEarned(const GameState& state);

// Adds the rewards of a finished expedition to the profile (coins and the relic).
// Returns the coins earned.
int collectRewards(PlayerProfile& profile, const GameState& state);

// The Trading Post
int relicSellPrice(Difficulty difficulty);
bool sellRelic(PlayerProfile& profile, Location location, Difficulty difficulty);

enum class PurchaseResult
{
    Bought,
    AlreadyOwned,
    NotEnoughCoins,
    InvalidSkin
};

PurchaseResult buySkin(PlayerProfile& profile, int skinId);
bool equipSkin(PlayerProfile& profile, int skinId);
