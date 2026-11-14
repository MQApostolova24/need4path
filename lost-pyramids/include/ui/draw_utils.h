#pragma once

#include "raylib.h"

// Small drawing helpers shared by the scenes, sprites and the maze renderer.

// Spreads objects (stars, raindrops...) evenly over 0..1 without random numbers:
// every next object moves 0.618 (the golden ratio) further and wraps around.
float spread(int index);

// A fixed number 0..9 for a maze tile, used to decorate some tiles and not others.
// The same tile always gets the same number, so decorations never flicker.
int tilePattern(int row, int col);

// A rectangle area with helpers that turn fractions (0..1) into screen positions.
// Scenes are drawn with fractions, so they fit any area: the full screen or a small card.
struct Frame
{
    Rectangle area;

    float x(float fraction) const;
    float y(float fraction) const;
    float w(float fraction) const;
    float h(float fraction) const;
    Vector2 at(float fx, float fy) const;
};

// A filled triangle whose corners can be given in any order.
void drawTriangle(Vector2 a, Vector2 b, Vector2 c, Color color);

// A small filled diamond (a square standing on its corner) - used for ornaments.
void drawDiamond(Vector2 center, float radius, Color color);
