#include "core/location.h"

#include <algorithm>

namespace
{
const LocationInfo LOCATIONS[3] = {
    {
        "Egypt",
        "Pyramid of Akhen-Ra",
        "Sands of the Curse",
        "The cursed sands buried the corridors.",
        1.0f,
        "Mummy",
        "slow, but never stops searching",
        0.42f,
        12,
        "Scarab pit",
        "Ankh key",
        "Eye of Horus door",
        "Sun Gate",
        "Gold scarab",
        "Lapis amulet",
        "Hourglass",
        {"Ankh of Eternal Life", "Golden Death Mask", "Crown of Akhen-Ra"},
    },
    {
        "Mexico",
        "Temple of the Feathered Serpent",
        "Solar Eclipse",
        "The eclipse swallowed the sun - darkness fell.",
        0.9f,
        "Jaguar spirit",
        "very fast, but loses interest quickly",
        0.30f,
        8,
        "Spike pit",
        "Jade glyph",
        "Serpent glyph door",
        "Serpent Gate",
        "Gold bead",
        "Jade mask",
        "Sun idol",
        {"Jade Jaguar Idol", "Obsidian Mirror", "Crown of Kukulkan"},
    },
    {
        "Cambodia",
        "Temple of the Naga King",
        "Monsoon Flood",
        "The monsoon flooded the temple halls.",
        1.1f,
        "Naga serpent",
        "slow, but senses you from far away",
        0.50f,
        16,
        "Cobra nest",
        "Lotus key",
        "Stone face door",
        "Lotus Gate",
        "Silver coin",
        "Sapphire",
        "Rain charm",
        {"Golden Lotus", "Apsara Statuette", "Sword of the Naga King"},
    },
};
}  // namespace

const LocationInfo& getLocationInfo(Location location)
{
    const int index = std::clamp(static_cast<int>(location), 0, 2);
    return LOCATIONS[index];
}

const char* locationName(Location location)
{
    return getLocationInfo(location).name;
}

bool locationFromName(const std::string& name, Location& location)
{
    for (int i = 0; i < 3; ++i)
    {
        if (name == LOCATIONS[i].name)
        {
            location = static_cast<Location>(i);
            return true;
        }
    }
    return false;
}
