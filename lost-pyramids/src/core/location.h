#pragma once

#include <string>

// The three pyramid sites the player can explore.
// Every location has its own story, timer, guardian, trap, key, door,
// rewards and relics. The colours and pictures live in ui/location_theme.

enum class Location
{
    Egypt = 0,
    Mexico = 1,
    Cambodia = 2
};

struct LocationInfo
{
    const char* id;              // saved in the data files, e.g. "Egypt"
    const char* country;         // shown to the player
    const char* site;            // where the pyramid stands
    const char* temple;          // the name of the pyramid
    const char* legend[3];       // short story, three lines

    const char* timerName;       // what the countdown means here
    const char* timeUpMessage;   // shown when the timer reaches zero
    float timeMultiplier;        // more or less time than the default

    const char* guardianName;    // the creature that hunts you on Hard
    const char* guardianTrait;   // one short line for the rules screen
    float guardianStepSeconds;   // how often the guardian moves (smaller = faster)
    int guardianChaseRange;      // it hunts you within this many steps

    const char* trapName;
    const char* keyName;
    const char* doorName;
    const char* exitName;
    const char* coinName;        // common reward
    const char* gemName;         // rare reward
    const char* timePickupName;  // adds seconds to the timer
    const char* relics[3];       // the legendary relic for Easy, Medium and Hard
};

const LocationInfo& getLocationInfo(Location location);
const char* locationId(Location location);
bool locationFromId(const std::string& id, Location& location);
