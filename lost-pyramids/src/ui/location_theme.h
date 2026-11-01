#pragma once

#include "core/location.h"
#include "raylib.h"

// Colours and backdrop pictures for each pyramid location.

struct LocationPalette
{
    Color skyTop;
    Color skyBottom;
    Color sun;
    Color hillFar;
    Color hillNear;
    Color ground;
    Color groundLight;
    Color monumentLit;
    Color monumentShade;

    Color wall;       // maze walls
    Color wallLight;
    Color wallDark;
    Color wallDetail; // moss, carvings
    Color floorA;     // maze floor (checkered)
    Color floorB;
    Color floorLine;

    Color accent;     // gold / jade / lotus pink
    Color accentDark;
    Color accentSoft; // second accent: turquoise / red ochre / gold
    Color gem;
    Color glow;       // torch light colour
    Color particle;   // sand, fireflies, rain
};

const LocationPalette& getPalette(Location location);

// Draws the location's landscape (sky, pyramid, scenery, weather) inside any rectangle.
void drawLocationScene(Location location, Rectangle area, float time);

// The landscapes of all three locations, slowly cross-fading - used behind the menus.
void drawMenuBackdrop(float time);

// A small one-colour pyramid icon for the location (tabs, HUD, Trading Post).
void drawLocationEmblem(Location location, Vector2 center, float size, Color color);
