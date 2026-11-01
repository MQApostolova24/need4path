#pragma once

#include "raylib.h"
#include "ui/fonts.h"

#include <cstddef>
#include <string>

// Reusable user-interface pieces: panels, buttons, tabs, text fields,
// the game title, stars and hearts. Everything here draws with raylib.

struct TextField
{
    std::string text;
    std::string label;
    std::size_t maxLength = 16;
    bool masked = false;      // show * instead of the characters (passwords)
    bool digitsOnly = false;  // accept only 0-9 (pyramid size)
    bool active = false;      // has keyboard focus
};

// A dark journal panel with a double golden border. "accent" colours the border.
void drawPanel(Rectangle bounds, Color accent);

// The game title "LOST PYRAMIDS" with an ornament line and an optional subtitle.
void drawGameTitle(float y, const char* subtitle);

// A big heading in any font with an ornament line underneath.
void drawHeading(const char* text, float y, float size, Color color, FontStyle style);

// A golden ornament line with diamonds, centred on centerX.
void drawOrnamentLine(float centerX, float y, float halfWidth, Color color);

// Draws a button and returns true when it is clicked with the mouse.
bool drawButton(Rectangle bounds, const char* text, bool selected);

// A tab for a row of filters. Returns true when clicked.
bool drawTab(Rectangle bounds, const char* text, bool active, Color accent);

// Keyboard navigation for a vertical/horizontal list of buttons.
// Returns the index activated with Enter, or -1.
int handleMenuNavigation(int& selection, int count);

// Typing, Backspace, Tab between fields and mouse focus for a group of fields.
void updateTextFields(TextField* fields[], const Rectangle boxes[], int count);
void drawTextField(const TextField& field, Rectangle bounds);

// Rewards and lives
void drawStar(Vector2 center, float radius, bool filled, Color color);
void drawStarRow(Vector2 firstCenter, float radius, int filled, int total, Color color);
void drawHeart(Vector2 center, float size, Color color);

// A gold coin (the shop currency).
void drawCoinIcon(Vector2 center, float radius);

// The player's coin balance in a rounded badge; x/y = top-right corner.
void drawCoinBadge(int coins, float rightX, float topY);

// Text inside a small rounded label ("NEW!", "LOCKED").
void drawBadge(const char* text, Vector2 center, float size, Color background, Color textColor);
