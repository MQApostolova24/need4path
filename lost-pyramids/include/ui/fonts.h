#pragma once

#include "core/location.h"
#include "raylib.h"

// Themed fonts loaded from assets/fonts (all under the SIL Open Font License):
//   Title    - Cinzel Decorative Bold (game title and big headings)
//   Body     - Alegreya Sans (normal text)
//   Bold     - Alegreya Sans Bold (buttons, numbers)
//   Egypt    - Rakkas      Mexico - Macondo      Cambodia - Koulen
// If a font file is missing, raylib's built-in font is used instead.

enum class FontStyle
{
    Title,
    Body,
    Bold,
    Egypt,
    Mexico,
    Cambodia
};

// Must be called after InitWindow and before the first frame.
void loadGameFonts();
void unloadGameFonts();

// The heading font of a location.
FontStyle locationFont(Location location);

void drawString(const char* text, float x, float y, float size, Color color, FontStyle style = FontStyle::Body);
void drawStringCentered(const char* text, float centerX, float y, float size, Color color,
                        FontStyle style = FontStyle::Body);
void drawStringRight(const char* text, float rightX, float y, float size, Color color,
                     FontStyle style = FontStyle::Body);
// Text with a soft dark shadow - for titles drawn over pictures.
void drawStringShadowCentered(const char* text, float centerX, float y, float size, Color color, FontStyle style);

float measureString(const char* text, float size, FontStyle style = FontStyle::Body);
