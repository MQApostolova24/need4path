#pragma once

#include "core/profile.h"

#include <string>

// Player profiles in data/profiles.txt, one line per player:
//   username;coins;equippedSkin;ownedSkins;discoveredRelics;r0,r1,...,r8;expeditionsWithoutRelic
// r0..r8 are how many of each relic the player holds (index = location * 3 + difficulty).

// Loads a player's profile. A player without a saved line gets a fresh profile.
PlayerProfile loadProfile(const std::string& filePath, const std::string& username);

// Saves (adds or replaces) the player's line, keeping everyone else's lines.
bool saveProfile(const std::string& filePath, const PlayerProfile& profile);
