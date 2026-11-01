#include "ui/draw_utils.h"

#include <cmath>
#include <cstdint>

float hash01(int a, int b, int salt)
{
    std::uint32_t value = static_cast<std::uint32_t>(a) * 73856093u ^ static_cast<std::uint32_t>(b) * 19349663u ^
                          static_cast<std::uint32_t>(salt) * 83492791u;
    value ^= value >> 13;
    value *= 0x5bd1e995u;
    value ^= value >> 15;
    return static_cast<float>(value % 10000u) / 10000.0f;
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

Color mixColor(Color first, Color second, float amount)
{
    return ColorLerp(first, second, amount);
}

void drawTriangleAnyOrder(Vector2 a, Vector2 b, Vector2 c, Color color)
{
    // raylib needs counter-clockwise corners (as seen on screen, where y grows downwards).
    // The sign of the cross product tells us the current order.
    const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross < 0.0f)
    {
        DrawTriangle(a, b, c, color);
    }
    else
    {
        DrawTriangle(a, c, b, color);
    }
}

void drawDiamond(Vector2 center, float radius, Color color)
{
    const Vector2 top = {center.x, center.y - radius};
    const Vector2 bottom = {center.x, center.y + radius};
    drawTriangleAnyOrder(top, {center.x - radius, center.y}, {center.x + radius, center.y}, color);
    drawTriangleAnyOrder(bottom, {center.x - radius, center.y}, {center.x + radius, center.y}, color);
}
