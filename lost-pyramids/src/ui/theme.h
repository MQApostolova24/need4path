#pragma once

#include "raylib.h"

// Window size and the neutral "expedition journal" colours used by menus and panels.
// Location-specific colours (sand, jungle, monsoon) live in location_theme.h.

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

namespace theme
{
const Color INK = {34, 26, 20, 255};
const Color PARCHMENT = {242, 230, 200, 255};
const Color PARCHMENT_DARK = {214, 196, 156, 255};

const Color PANEL = {22, 18, 16, 228};
const Color PANEL_LIGHT = {58, 46, 34, 236};

const Color GOLD_BRIGHT = {242, 194, 64, 255};
const Color GOLD_DEEP = {170, 124, 30, 255};
const Color BRONZE = {96, 72, 46, 255};
const Color BRONZE_LIGHT = {132, 102, 66, 255};

const Color TEXT_LIGHT = {240, 230, 208, 255};
const Color TEXT_MUTED = {186, 170, 142, 255};
const Color DANGER = {232, 96, 80, 255};
const Color SUCCESS = {110, 210, 162, 255};
const Color HEART = {214, 52, 52, 255};
const Color LOCKED = {92, 84, 76, 255};
}  // namespace theme
