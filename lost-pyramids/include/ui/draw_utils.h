#pragma once

#include "raylib.h"

// Small drawing helpers shared by the scenes, sprites and the maze renderer.

// A repeatable "random" number in [0, 1) for a pair of integers.
// The same inputs always give the same value, so decorations do not flicker.
float hash01(int a, int b, int salt);

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

// A filled triangle that does not care about the order of its corners.
void drawTriangleAnyOrder(Vector2 a, Vector2 b, Vector2 c, Color color);

// A small filled diamond (a square standing on its corner) - used for ornaments.
void drawDiamond(Vector2 center, float radius, Color color);
