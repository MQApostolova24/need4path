#include "core/skins.h"

#include "core/config.h"

namespace
{
const SkinInfo SKINS[SKIN_COUNT] = {
    {"Field Explorer", "The classic khaki outfit.", 0},
    {"Desert Nomad", "Light robes for the desert sun.", 100},
    {"Jungle Ranger", "Pith helmet and jungle greens.", 200},
    {"Monsoon Wanderer", "A rain cape and a wide straw hat.", 350},
    {"Night Raider", "Dark gear and glowing goggles.", 500},
    {"Golden Pharaoh", "Worthy of a king. Pure gold.", 800},
};
}  // namespace

bool isValidSkin(int skinId)
{
    return skinId >= 0 && skinId < SKIN_COUNT;
}

const SkinInfo& getSkinInfo(int skinId)
{
    return SKINS[isValidSkin(skinId) ? skinId : 0];
}
