#pragma once

// Explorer skins that can be bought with coins in the Trading Post.
// The pictures are drawn in ui/sprites.cpp; here are only the names and prices.

struct SkinInfo
{
    const char* name;
    const char* description;
    int price;  // in coins, 0 = free
};

bool isValidSkin(int skinId);
const SkinInfo& getSkinInfo(int skinId);
