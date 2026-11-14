#include "ui/draw_utils.h"

#include <cmath>

float spread(int index)
{
    return std::fmod(static_cast<float>(index) * 0.618f, 1.0f);
}

int tilePattern(int row, int col)
{
    // Mix the row and column with a few prime numbers; the remainders look scattered.
    return (row * row * 37 + col * col * 61 + row * col * 13) % 101 % 10;
}

float Frame::x(float fraction) const
{
    return area.x + fraction * area.width;
}

float Frame::y(float fraction) const
{
    return area.y + fraction * area.height;
}

float Frame::w(float fraction) const
{
    return fraction * area.width;
}

float Frame::h(float fraction) const
{
    return fraction * area.height;
}

Vector2 Frame::at(float fx, float fy) const
{
    return {x(fx), y(fy)};
}

void drawTriangle(Vector2 a, Vector2 b, Vector2 c, Color color)
{
    // raylib only draws a triangle whose corners go round in one direction
    // (the other side is "facing away" and skipped). Drawing both orders means
    // exactly one of them is shown, whatever order the corners came in.
    DrawTriangle(a, b, c, color);
    DrawTriangle(a, c, b, color);
}

void drawDiamond(Vector2 center, float radius, Color color)
{
    const Vector2 top = {center.x, center.y - radius};
    const Vector2 bottom = {center.x, center.y + radius};
    const Vector2 left = {center.x - radius, center.y};
    const Vector2 right = {center.x + radius, center.y};
    drawTriangle(top, left, right, color);
    drawTriangle(bottom, left, right, color);
}
