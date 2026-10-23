#pragma once

#include "raylib.h"

#include <cstddef>
#include <string>

// Reusable user-interface pieces: background, title, panels, buttons,
// text fields and small Egyptian icons. Everything here draws with raylib.

struct TextField
{
    std::string text;
    std::string label;
    std::size_t maxLength = 16;
    bool masked = false;      // show * instead of the characters (passwords)
    bool digitsOnly = false;  // accept only 0-9 (tomb size)
    bool active = false;      // has keyboard focus
};

void drawCenteredText(const char* text, int centerX, int y, int fontSize, Color color);
void drawDesertBackground(float time);
void drawTitle(const char* title, const char* subtitle, int y);
void drawPanel(Rectangle bounds);

// Draws a button and returns true when it is clicked with the mouse.
bool drawButton(Rectangle bounds, const char* text, bool selected);

// Keyboard navigation for a vertical/horizontal list of buttons.
// Returns the index activated with Enter, or -1.
int handleMenuNavigation(int& selection, int count);

// Typing, Backspace, Tab between fields and mouse focus for a group of fields.
void updateTextFields(TextField* fields[], const Rectangle boxes[], int count);
void drawTextField(const TextField& field, Rectangle bounds);

// Icons
void drawAnkhIcon(Vector2 center, float size, Color color);
void drawHeartIcon(Vector2 center, float size, Color color);
void drawEyeIcon(Vector2 center, float size, Color color);
void drawScarabIcon(Vector2 center, float size, Color color);
