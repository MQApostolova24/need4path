#pragma once

#include "raylib.h"

// Window size and the "Egyptian tomb" colour palette used across the whole UI.

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

namespace theme
{
const Color NIGHT_SKY = {18, 14, 32, 255};
const Color DUSK = {92, 46, 58, 255};

const Color SAND_LIGHT = {232, 196, 128, 255};
const Color SAND = {201, 160, 92, 255};
const Color SAND_DARK = {140, 100, 52, 255};

const Color STONE = {120, 88, 56, 255};
const Color STONE_DARK = {74, 52, 34, 255};
const Color FLOOR = {50, 36, 26, 255};
const Color FLOOR_LIGHT = {60, 44, 31, 255};

const Color BRIGHT_GOLD = {244, 196, 48, 255};
const Color DARK_GOLD = {176, 132, 24, 255};
const Color TURQUOISE = {64, 200, 190, 255};
const Color LAPIS = {38, 70, 160, 255};
const Color BLOOD = {190, 40, 40, 255};
const Color BLOOD_LIGHT = {235, 96, 84, 255};

const Color PAPYRUS = {240, 226, 190, 255};
const Color INK = {40, 28, 20, 255};
const Color PANEL = {30, 22, 18, 235};
const Color PANEL_LIGHT = {64, 46, 30, 240};

const Color KHAKI = {196, 170, 120, 255};
const Color HAT_BROWN = {110, 72, 40, 255};
const Color BANDAGE = {226, 218, 196, 255};
const Color BANDAGE_DARK = {150, 140, 118, 255};
const Color PIT = {22, 16, 12, 255};
}  // namespace theme
