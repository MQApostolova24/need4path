#include "ui/sprites.h"

#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/theme.h"

#include <cmath>

namespace
{
const Color KHAKI = {200, 172, 120, 255};
const Color HAT_BROWN = {112, 74, 42, 255};
const Color BANDAGE = {228, 220, 198, 255};
const Color BANDAGE_DARK = {150, 140, 118, 255};
const Color GUARDIAN_EYES = {230, 48, 40, 255};
const Color JAGUAR = {232, 166, 58, 255};
const Color NAGA = {72, 152, 96, 255};
const Color NAGA_LIGHT = {136, 200, 120, 255};
const Color SILVER = {214, 218, 226, 255};
const Color SILVER_DARK = {140, 146, 160, 255};
const Color PIT = {20, 15, 12, 255};
const Color WATER = {70, 150, 220, 255};
const Color FLAME = {255, 150, 40, 255};
const Color FLAME_CORE = {255, 236, 140, 255};

float thick(float size, float part)
{
    return std::fmax(1.0f, size * part);
}

// A small twinkling glint that makes rewards stand out.
void drawGlint(Vector2 center, float size, float time, int seed)
{
    const float phase = std::fmod(time * 0.8f + hash01(seed, 1, 9), 1.0f);
    if (phase > 0.25f)
    {
        return;
    }
    const float scale = std::sin(phase / 0.25f * PI) * size * 0.22f;
    const Vector2 c = {center.x + size * 0.18f, center.y - size * 0.18f};
    DrawLineEx({c.x - scale, c.y}, {c.x + scale, c.y}, thick(size, 0.04f), WHITE);
    DrawLineEx({c.x, c.y - scale}, {c.x, c.y + scale}, thick(size, 0.04f), WHITE);
}

void drawAnkh(Vector2 center, float size, Color color)
{
    const Vector2 loop = {center.x, center.y - size * 0.24f};
    DrawRing(loop, size * 0.09f, size * 0.17f, 0.0f, 360.0f, 20, color);
    DrawRectangleV({center.x - size * 0.27f, center.y - size * 0.08f}, {size * 0.54f, size * 0.11f}, color);
    DrawRectangleV({center.x - size * 0.06f, center.y - size * 0.08f}, {size * 0.12f, size * 0.54f}, color);
}

void drawEyeOfHorus(Vector2 center, float size, Color color)
{
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.40f, size * 0.20f, color);
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.31f, size * 0.12f, theme::INK);
    DrawCircleV(center, size * 0.10f, color);
    const float t = thick(size, 0.06f);
    DrawLineEx({center.x - size * 0.05f, center.y + size * 0.18f}, {center.x - size * 0.16f, center.y + size * 0.38f}, t,
               color);
    DrawLineEx({center.x - size * 0.36f, center.y - size * 0.30f}, {center.x + size * 0.38f, center.y - size * 0.30f}, t,
               color);
}

void drawScarab(Vector2 center, float size, Color color)
{
    const float t = thick(size, 0.07f);
    for (int i = -1; i <= 1; ++i)
    {
        const float legY = center.y + size * 0.16f * static_cast<float>(i);
        DrawLineEx({center.x - size * 0.42f, legY + size * 0.08f}, {center.x + size * 0.42f, legY - size * 0.08f}, t,
                   color);
        DrawLineEx({center.x - size * 0.42f, legY - size * 0.08f}, {center.x + size * 0.42f, legY + size * 0.08f}, t,
                   color);
    }
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.05f), size * 0.25f, size * 0.32f,
                color);
    DrawCircleV({center.x, center.y - size * 0.32f}, size * 0.13f, color);
    DrawLineEx({center.x, center.y - size * 0.18f}, {center.x, center.y + size * 0.35f}, t, Fade(BLACK, 0.45f));
}

void drawLotus(Vector2 center, float size, Color petal, Color heart)
{
    // Back petals, side petals, front petal
    drawTriangleAnyOrder({center.x, center.y - size * 0.45f}, {center.x - size * 0.16f, center.y + size * 0.1f},
                         {center.x + size * 0.16f, center.y + size * 0.1f}, petal);
    drawTriangleAnyOrder({center.x - size * 0.42f, center.y - size * 0.2f}, {center.x - size * 0.1f, center.y + size * 0.15f},
                         {center.x - size * 0.02f, center.y - size * 0.05f}, petal);
    drawTriangleAnyOrder({center.x + size * 0.42f, center.y - size * 0.2f}, {center.x + size * 0.1f, center.y + size * 0.15f},
                         {center.x + size * 0.02f, center.y - size * 0.05f}, petal);
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.12f), size * 0.3f, size * 0.1f, petal);
    DrawCircleV({center.x, center.y + size * 0.02f}, size * 0.08f, heart);
}

void drawWaterDrop(Vector2 center, float size, Color color)
{
    DrawCircleV({center.x, center.y + size * 0.1f}, size * 0.24f, color);
    drawTriangleAnyOrder({center.x, center.y - size * 0.38f}, {center.x - size * 0.22f, center.y + size * 0.02f},
                         {center.x + size * 0.22f, center.y + size * 0.02f}, color);
    DrawCircleV({center.x - size * 0.08f, center.y + size * 0.04f}, size * 0.06f, Fade(WHITE, 0.7f));
}

void drawSunDisc(Vector2 center, float size, Color color, Color face)
{
    for (int i = 0; i < 8; ++i)
    {
        const float angle = static_cast<float>(i) * PI / 4.0f;
        const Vector2 tip = {center.x + std::cos(angle) * size * 0.46f, center.y + std::sin(angle) * size * 0.46f};
        const Vector2 left = {center.x + std::cos(angle - 0.3f) * size * 0.26f, center.y + std::sin(angle - 0.3f) * size * 0.26f};
        const Vector2 right = {center.x + std::cos(angle + 0.3f) * size * 0.26f, center.y + std::sin(angle + 0.3f) * size * 0.26f};
        drawTriangleAnyOrder(tip, left, right, color);
    }
    DrawCircleV(center, size * 0.28f, color);
    DrawCircleV({center.x - size * 0.09f, center.y - size * 0.04f}, size * 0.04f, face);
    DrawCircleV({center.x + size * 0.09f, center.y - size * 0.04f}, size * 0.04f, face);
    DrawLineEx({center.x - size * 0.08f, center.y + size * 0.1f}, {center.x + size * 0.08f, center.y + size * 0.1f},
               thick(size, 0.04f), face);
}

// Rotating light rays behind the relic.
void drawAura(Vector2 center, float size, Color color, float time)
{
    const float pulse = 0.75f + 0.25f * std::sin(time * 3.0f);
    DrawCircleGradient(center, size * 0.9f * pulse, Fade(color, 0.55f), Fade(color, 0.0f));
    for (int i = 0; i < 8; ++i)
    {
        const float angle = time * 0.8f + static_cast<float>(i) * PI / 4.0f;
        const Vector2 tip = {center.x + std::cos(angle) * size * 0.75f, center.y + std::sin(angle) * size * 0.75f};
        const Vector2 left = {center.x + std::cos(angle - 0.12f) * size * 0.2f, center.y + std::sin(angle - 0.12f) * size * 0.2f};
        const Vector2 right = {center.x + std::cos(angle + 0.12f) * size * 0.2f, center.y + std::sin(angle + 0.12f) * size * 0.2f};
        drawTriangleAnyOrder(tip, left, right, Fade(color, 0.35f));
    }
}
}  // namespace

void drawKeySprite(Location location, Vector2 center, float size)
{
    const LocationPalette& p = getPalette(location);
    const Vector2 shadow = {center.x + size * 0.04f, center.y + size * 0.05f};

    switch (location)
    {
    case Location::Egypt:
        drawAnkh(shadow, size, Fade(BLACK, 0.4f));
        drawAnkh(center, size, p.accentSoft);
        break;
    case Location::Mexico:
    {
        const Rectangle glyph = {center.x - size * 0.3f, center.y - size * 0.3f, size * 0.6f, size * 0.6f};
        DrawRectangleRounded({glyph.x + size * 0.04f, glyph.y + size * 0.05f, glyph.width, glyph.height}, 0.4f, 6,
                             Fade(BLACK, 0.4f));
        DrawRectangleRounded(glyph, 0.4f, 6, p.accent);
        DrawRing(center, size * 0.1f, size * 0.15f, 0.0f, 270.0f, 16, p.accentDark);
        DrawCircleV(center, size * 0.05f, p.accentDark);
        DrawCircleV({glyph.x + size * 0.1f, glyph.y + size * 0.1f}, size * 0.035f, p.accentDark);
        DrawCircleV({glyph.x + glyph.width - size * 0.1f, glyph.y + glyph.height - size * 0.1f}, size * 0.035f,
                    p.accentDark);
        break;
    }
    case Location::Cambodia:
        DrawRectangleV({center.x - size * 0.04f, center.y}, {size * 0.08f, size * 0.38f}, p.accentSoft);
        DrawRectangleV({center.x, center.y + size * 0.24f}, {size * 0.14f, size * 0.06f}, p.accentSoft);
        DrawRectangleV({center.x, center.y + size * 0.32f}, {size * 0.1f, size * 0.06f}, p.accentSoft);
        drawLotus({center.x, center.y - size * 0.06f}, size * 0.8f, p.accent, p.accentSoft);
        break;
    }
}

void drawDoorSprite(Location location, Rectangle tile, float time)
{
    const LocationPalette& p = getPalette(location);
    const float pulse = 0.65f + 0.35f * std::sin(time * 3.0f);
    const Vector2 center = {tile.x + tile.width / 2.0f, tile.y + tile.height / 2.0f};
    const float size = tile.width;

    DrawRectangleLinesEx(tile, thick(size, 0.08f), Fade(p.accent, pulse));

    switch (location)
    {
    case Location::Egypt:
        drawEyeOfHorus(center, size * 0.85f, Fade(p.accent, pulse));
        break;
    case Location::Mexico:
        // Feathered serpent head facing right
        DrawRectangleRounded({center.x - size * 0.32f, center.y - size * 0.22f, size * 0.62f, size * 0.42f}, 0.5f, 6,
                             Fade(p.accent, pulse));
        drawTriangleAnyOrder({center.x + size * 0.32f, center.y + size * 0.02f}, {center.x + size * 0.02f, center.y + size * 0.04f},
                             {center.x + size * 0.3f, center.y + size * 0.2f}, theme::INK);
        DrawCircleV({center.x + size * 0.04f, center.y - size * 0.07f}, size * 0.08f, theme::PARCHMENT);
        DrawCircleV({center.x + size * 0.06f, center.y - size * 0.07f}, size * 0.04f, theme::INK);
        for (int i = 0; i < 3; ++i)
        {
            const float x = center.x - size * 0.22f + static_cast<float>(i) * size * 0.12f;
            DrawLineEx({x, center.y - size * 0.22f}, {x - size * 0.06f, center.y - size * 0.4f}, thick(size, 0.06f),
                       p.accentSoft);
        }
        break;
    case Location::Cambodia:
        // Smiling stone face, like the towers of the Bayon temple
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.05f), size * 0.27f, size * 0.33f,
                    p.wallLight);
        drawTriangleAnyOrder({center.x, center.y - size * 0.45f}, {center.x - size * 0.2f, center.y - size * 0.2f},
                             {center.x + size * 0.2f, center.y - size * 0.2f}, Fade(p.accentSoft, pulse));
        DrawLineEx({center.x - size * 0.16f, center.y}, {center.x - size * 0.05f, center.y}, thick(size, 0.05f), theme::INK);
        DrawLineEx({center.x + size * 0.05f, center.y}, {center.x + size * 0.16f, center.y}, thick(size, 0.05f), theme::INK);
        DrawRing({center.x, center.y + size * 0.1f}, size * 0.08f, size * 0.11f, 20.0f, 160.0f, 10, theme::INK);
        break;
    }
}

void drawTreasureSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.24f), size * 0.22f, size * 0.07f,
                Fade(BLACK, 0.35f));

    switch (location)
    {
    case Location::Egypt:
        drawScarab(center, size * 0.6f, p.accent);
        break;
    case Location::Mexico:
        DrawCircleV(center, size * 0.22f, theme::GOLD_BRIGHT);
        DrawRing(center, size * 0.13f, size * 0.17f, 0.0f, 360.0f, 16, theme::GOLD_DEEP);
        for (int i = 0; i < 4; ++i)
        {
            const float angle = static_cast<float>(i) * PI / 2.0f + PI / 4.0f;
            DrawCircleV({center.x + std::cos(angle) * size * 0.07f, center.y + std::sin(angle) * size * 0.07f},
                        size * 0.025f, theme::GOLD_DEEP);
        }
        break;
    case Location::Cambodia:
        DrawCircleV(center, size * 0.22f, SILVER);
        DrawRing(center, size * 0.18f, size * 0.22f, 0.0f, 360.0f, 16, SILVER_DARK);
        DrawRectangleRec({center.x - size * 0.06f, center.y - size * 0.06f, size * 0.12f, size * 0.12f}, SILVER_DARK);
        break;
    }
    drawGlint(center, size, time, static_cast<int>(center.x * 7.0f + center.y));
}

void drawGemSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.3f), size * 0.24f, size * 0.07f,
                Fade(BLACK, 0.35f));

    switch (location)
    {
    case Location::Egypt:
        // Lapis amulet in a gold setting
        DrawCircleV({center.x, center.y + size * 0.06f}, size * 0.24f, p.accent);
        drawTriangleAnyOrder({center.x, center.y - size * 0.36f}, {center.x - size * 0.22f, center.y},
                             {center.x + size * 0.22f, center.y}, p.accent);
        DrawCircleV({center.x, center.y + size * 0.06f}, size * 0.18f, p.gem);
        drawTriangleAnyOrder({center.x, center.y - size * 0.26f}, {center.x - size * 0.15f, center.y},
                             {center.x + size * 0.15f, center.y}, p.gem);
        DrawCircleV({center.x - size * 0.06f, center.y}, size * 0.05f, Fade(WHITE, 0.7f));
        break;
    case Location::Mexico:
        // Jade mask
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.26f, size * 0.32f, p.gem);
        DrawCircleV({center.x - size * 0.29f, center.y}, size * 0.06f, p.accentDark);
        DrawCircleV({center.x + size * 0.29f, center.y}, size * 0.06f, p.accentDark);
        DrawRectangleRec({center.x - size * 0.17f, center.y - size * 0.1f, size * 0.12f, size * 0.05f}, theme::INK);
        DrawRectangleRec({center.x + size * 0.05f, center.y - size * 0.1f, size * 0.12f, size * 0.05f}, theme::INK);
        DrawRectangleRec({center.x - size * 0.1f, center.y + size * 0.12f, size * 0.2f, size * 0.05f}, theme::INK);
        DrawCircleV({center.x - size * 0.1f, center.y - size * 0.2f}, size * 0.05f, Fade(WHITE, 0.5f));
        break;
    case Location::Cambodia:
    {
        // Cut sapphire
        const Vector2 top = {center.x, center.y - size * 0.32f};
        const Vector2 bottom = {center.x, center.y + size * 0.32f};
        const Vector2 left = {center.x - size * 0.26f, center.y - size * 0.05f};
        const Vector2 right = {center.x + size * 0.26f, center.y - size * 0.05f};
        drawTriangleAnyOrder(top, left, right, mixColor(p.gem, WHITE, 0.3f));
        drawTriangleAnyOrder(left, bottom, right, p.gem);
        drawTriangleAnyOrder(center, bottom, right, mixColor(p.gem, BLACK, 0.25f));
        DrawLineEx(left, right, thick(size, 0.03f), Fade(WHITE, 0.5f));
        break;
    }
    }
    drawGlint(center, size, time, static_cast<int>(center.x * 3.0f + center.y * 5.0f));
}

void drawRelicSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    drawAura(center, size, p.accent, time);
    const float bob = std::sin(time * 2.5f) * size * 0.04f;
    const Vector2 c = {center.x, center.y + bob};

    switch (location)
    {
    case Location::Egypt:
    {
        // Golden death mask with a striped headdress
        drawTriangleAnyOrder({c.x, c.y - size * 0.42f}, {c.x - size * 0.4f, c.y + size * 0.36f},
                             {c.x + size * 0.4f, c.y + size * 0.36f}, theme::GOLD_BRIGHT);
        for (int i = 0; i < 4; ++i)
        {
            const float y = c.y - size * 0.1f + static_cast<float>(i) * size * 0.11f;
            const float half = size * (0.17f + 0.05f * static_cast<float>(i));
            DrawLineEx({c.x - half - size * 0.08f, y}, {c.x - half + size * 0.02f, y}, thick(size, 0.04f), p.gem);
            DrawLineEx({c.x + half - size * 0.02f, y}, {c.x + half + size * 0.08f, y}, thick(size, 0.04f), p.gem);
        }
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y + size * 0.06f), size * 0.17f, size * 0.24f,
                    mixColor(theme::GOLD_BRIGHT, WHITE, 0.25f));
        DrawLineEx({c.x - size * 0.12f, c.y}, {c.x - size * 0.03f, c.y}, thick(size, 0.04f), theme::INK);
        DrawLineEx({c.x + size * 0.03f, c.y}, {c.x + size * 0.12f, c.y}, thick(size, 0.04f), theme::INK);
        DrawRectangleRec({c.x - size * 0.03f, c.y + size * 0.28f, size * 0.06f, size * 0.12f}, p.gem);
        break;
    }
    case Location::Mexico:
        // Obsidian sun disc
        drawSunDisc(c, size * 1.1f, theme::GOLD_BRIGHT, theme::INK);
        DrawCircleV(c, size * 0.24f, {30, 28, 36, 255});
        DrawRing(c, size * 0.16f, size * 0.2f, 0.0f, 360.0f, 24, p.accent);
        DrawCircleV(c, size * 0.08f, theme::GOLD_BRIGHT);
        break;
    case Location::Cambodia:
        drawLotus({c.x, c.y + size * 0.05f}, size * 1.1f, theme::GOLD_BRIGHT, p.accent);
        break;
    }
}

void drawTimePickupSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    const float bob = std::sin(time * 3.0f + center.x) * size * 0.05f;
    const Vector2 c = {center.x, center.y + bob};
    DrawCircleGradient(c, size * 0.5f, Fade(p.glow, 0.35f), Fade(p.glow, 0.0f));

    switch (location)
    {
    case Location::Egypt:
        DrawRectangleRec({c.x - size * 0.24f, c.y - size * 0.36f, size * 0.48f, size * 0.07f}, theme::GOLD_DEEP);
        DrawRectangleRec({c.x - size * 0.24f, c.y + size * 0.29f, size * 0.48f, size * 0.07f}, theme::GOLD_DEEP);
        drawTriangleAnyOrder({c.x - size * 0.19f, c.y - size * 0.29f}, {c.x + size * 0.19f, c.y - size * 0.29f}, c,
                             Fade(theme::PARCHMENT, 0.8f));
        drawTriangleAnyOrder({c.x - size * 0.19f, c.y + size * 0.29f}, {c.x + size * 0.19f, c.y + size * 0.29f}, c,
                             Fade(theme::PARCHMENT, 0.8f));
        drawTriangleAnyOrder({c.x - size * 0.15f, c.y + size * 0.29f}, {c.x + size * 0.15f, c.y + size * 0.29f},
                             {c.x, c.y + size * 0.1f}, p.groundLight);
        break;
    case Location::Mexico:
        drawSunDisc(c, size * 0.8f, theme::GOLD_BRIGHT, p.accentSoft);
        break;
    case Location::Cambodia:
        drawWaterDrop(c, size * 0.8f, WATER);
        break;
    }
}

void drawTrapSprite(Location location, Vector2 center, float size, bool sprung)
{
    const LocationPalette& p = getPalette(location);
    DrawCircleV(center, size * 0.36f, PIT);
    DrawRing(center, size * 0.33f, size * 0.38f, 0.0f, 360.0f, 20, p.wallDark);

    if (sprung)
    {
        DrawLineEx({center.x - size * 0.22f, center.y - size * 0.1f}, {center.x + size * 0.18f, center.y + size * 0.14f},
                   1.0f, p.wallDark);
        DrawLineEx({center.x - size * 0.05f, center.y + size * 0.2f}, {center.x + size * 0.1f, center.y - size * 0.2f},
                   1.0f, p.wallDark);
        return;
    }

    switch (location)
    {
    case Location::Egypt:
        drawScarab(center, size * 0.45f, p.accentSoft);
        break;
    case Location::Mexico:
        for (int i = -1; i <= 1; ++i)
        {
            const float x = center.x + static_cast<float>(i) * size * 0.15f;
            drawTriangleAnyOrder({x, center.y - size * 0.22f}, {x - size * 0.07f, center.y + size * 0.15f},
                                 {x + size * 0.07f, center.y + size * 0.15f}, SILVER);
        }
        break;
    case Location::Cambodia:
        DrawRing({center.x, center.y + size * 0.08f}, size * 0.1f, size * 0.17f, 0.0f, 360.0f, 16, NAGA);
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y - size * 0.1f), size * 0.12f, size * 0.15f,
                    NAGA_LIGHT);
        DrawCircleV({center.x - size * 0.04f, center.y - size * 0.12f}, size * 0.025f, GUARDIAN_EYES);
        DrawCircleV({center.x + size * 0.04f, center.y - size * 0.12f}, size * 0.025f, GUARDIAN_EYES);
        break;
    }
}

void drawGuardianSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    const float sway = std::sin(time * 6.0f) * size * 0.03f;
    DrawCircleGradient(center, size * 0.7f, Fade(GUARDIAN_EYES, 0.25f), Fade(GUARDIAN_EYES, 0.0f));

    switch (location)
    {
    case Location::Egypt:
    {
        const Rectangle body = {center.x - size * 0.3f + sway, center.y - size * 0.4f, size * 0.6f, size * 0.8f};
        DrawRectangleRounded(body, 0.5f, 6, BANDAGE);
        for (int i = 1; i <= 3; ++i)
        {
            const float y = body.y + body.height * (0.22f * static_cast<float>(i) + 0.1f);
            DrawLineEx({body.x + 1.0f, y}, {body.x + body.width - 1.0f, y + size * 0.08f}, thick(size, 0.05f), BANDAGE_DARK);
        }
        DrawCircleV({center.x - size * 0.11f + sway, center.y - size * 0.18f}, thick(size, 0.07f), GUARDIAN_EYES);
        DrawCircleV({center.x + size * 0.11f + sway, center.y - size * 0.18f}, thick(size, 0.07f), GUARDIAN_EYES);
        break;
    }
    case Location::Mexico:
    {
        const Vector2 head = {center.x + sway, center.y};
        drawTriangleAnyOrder({head.x - size * 0.34f, head.y - size * 0.38f}, {head.x - size * 0.34f, head.y - size * 0.05f},
                             {head.x - size * 0.08f, head.y - size * 0.2f}, JAGUAR);
        drawTriangleAnyOrder({head.x + size * 0.34f, head.y - size * 0.38f}, {head.x + size * 0.34f, head.y - size * 0.05f},
                             {head.x + size * 0.08f, head.y - size * 0.2f}, JAGUAR);
        DrawCircleV(head, size * 0.34f, JAGUAR);
        for (int i = 0; i < 6; ++i)
        {
            const float angle = static_cast<float>(i) * 1.05f + 0.4f;
            DrawCircleV({head.x + std::cos(angle) * size * 0.24f, head.y + std::sin(angle) * size * 0.2f - size * 0.04f},
                        size * 0.035f, theme::INK);
        }
        DrawEllipse(static_cast<int>(head.x), static_cast<int>(head.y + size * 0.12f), size * 0.15f, size * 0.1f,
                    theme::PARCHMENT);
        DrawCircleV({head.x, head.y + size * 0.07f}, size * 0.04f, theme::INK);
        DrawCircleV({head.x - size * 0.12f, head.y - size * 0.06f}, thick(size, 0.06f), p.particle);
        DrawCircleV({head.x + size * 0.12f, head.y - size * 0.06f}, thick(size, 0.06f), p.particle);
        break;
    }
    case Location::Cambodia:
    {
        // Three cobra hoods rising from one coiled body
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.28f), size * 0.36f, size * 0.13f,
                    NAGA);
        for (int i = -1; i <= 1; ++i)
        {
            const Vector2 head = {center.x + static_cast<float>(i) * size * 0.24f + sway,
                                  center.y - size * 0.08f - (i == 0 ? size * 0.12f : 0.0f)};
            DrawLineEx({head.x, head.y + size * 0.1f}, {center.x, center.y + size * 0.26f}, thick(size, 0.12f), NAGA);
            // Dark outline first, then the hood, so the serpent stands out from the floor
            DrawEllipse(static_cast<int>(head.x), static_cast<int>(head.y), size * 0.17f, size * 0.21f, theme::INK);
            DrawEllipse(static_cast<int>(head.x), static_cast<int>(head.y), size * 0.15f, size * 0.19f, NAGA);
            DrawEllipse(static_cast<int>(head.x), static_cast<int>(head.y + size * 0.03f), size * 0.08f, size * 0.12f,
                        NAGA_LIGHT);
            DrawCircleV({head.x - size * 0.045f, head.y - size * 0.06f}, thick(size, 0.04f), GUARDIAN_EYES);
            DrawCircleV({head.x + size * 0.045f, head.y - size * 0.06f}, thick(size, 0.04f), GUARDIAN_EYES);
        }
        break;
    }
    }
}

void drawExitSprite(Location location, Rectangle tile, float time)
{
    const LocationPalette& p = getPalette(location);
    const float glow = 0.5f + 0.5f * std::sin(time * 2.5f);
    const Vector2 center = {tile.x + tile.width / 2.0f, tile.y + tile.height / 2.0f};

    DrawCircleGradient(center, tile.width * (1.6f + 0.5f * glow), Fade(p.accent, 0.5f), Fade(p.accent, 0.0f));
    for (int i = 0; i < 6; ++i)
    {
        const float angle = time * 0.6f + static_cast<float>(i) * PI / 3.0f;
        DrawLineEx(center, {center.x + std::cos(angle) * tile.width * 1.3f, center.y + std::sin(angle) * tile.width * 1.3f},
                   thick(tile.width, 0.1f), Fade(p.accent, 0.25f));
    }

    DrawRectangleGradientV(static_cast<int>(tile.x), static_cast<int>(tile.y), static_cast<int>(tile.width),
                           static_cast<int>(tile.height), mixColor(p.accent, WHITE, 0.55f), p.accent);
    DrawRectangleLinesEx(tile, thick(tile.width, 0.12f), p.accentDark);
    drawLocationEmblem(location, center, tile.height * 0.6f, Fade(theme::INK, 0.85f));
}

namespace
{
enum class Headwear
{
    Fedora,
    Headwrap,
    PithHelmet,
    StrawHat,
    Goggles,
    Nemes
};

// The colours and headwear of every skin (names and prices are in core/skins.cpp).
struct SkinLook
{
    Color body;
    Color hat;
    Color accent;
    Headwear headwear;
};

const SkinLook SKIN_LOOKS[SKIN_COUNT] = {
    {{200, 172, 120, 255}, {112, 74, 42, 255}, {214, 52, 52, 255}, Headwear::Fedora},      // Field Explorer
    {{236, 228, 210, 255}, {54, 102, 176, 255}, {242, 194, 64, 255}, Headwear::Headwrap},  // Desert Nomad
    {{96, 128, 70, 255}, {226, 212, 170, 255}, {60, 84, 46, 255}, Headwear::PithHelmet},   // Jungle Ranger
    {{60, 112, 140, 255}, {214, 184, 112, 255}, {150, 120, 70, 255}, Headwear::StrawHat},  // Monsoon Wanderer
    {{44, 44, 54, 255}, {190, 36, 44, 255}, {120, 255, 170, 255}, Headwear::Goggles},     // Night Raider
    {{242, 194, 64, 255}, {38, 70, 160, 255}, {242, 194, 64, 255}, Headwear::Nemes},       // Golden Pharaoh
};

void drawHeadwear(const SkinLook& look, Vector2 center, float size, float time)
{
    switch (look.headwear)
    {
    case Headwear::Fedora:
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y - size * 0.1f), size * 0.44f, size * 0.12f, look.hat);
        DrawRectangleRounded({center.x - size * 0.19f, center.y - size * 0.36f, size * 0.38f, size * 0.27f}, 0.4f, 4, look.hat);
        DrawRectangleRec({center.x - size * 0.19f, center.y - size * 0.17f, size * 0.38f, thick(size, 0.05f)}, look.accent);
        break;
    case Headwear::Headwrap:
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y - size * 0.16f), size * 0.33f, size * 0.2f, look.hat);
        DrawLineEx({center.x + size * 0.2f, center.y - size * 0.1f}, {center.x + size * 0.36f, center.y + size * 0.22f},
                   thick(size, 0.08f), look.hat);
        DrawCircleV({center.x, center.y - size * 0.24f}, size * 0.05f, look.accent);
        break;
    case Headwear::PithHelmet:
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y - size * 0.1f), size * 0.42f, size * 0.1f, look.hat);
        DrawCircleSector({center.x, center.y - size * 0.1f}, size * 0.26f, 180.0f, 360.0f, 16, look.hat);
        DrawCircleV({center.x, center.y - size * 0.36f}, size * 0.04f, look.accent);
        break;
    case Headwear::StrawHat:
        drawTriangleAnyOrder({center.x, center.y - size * 0.42f}, {center.x - size * 0.48f, center.y - size * 0.06f},
                             {center.x + size * 0.48f, center.y - size * 0.06f}, look.hat);
        DrawLineEx({center.x - size * 0.48f, center.y - size * 0.06f}, {center.x + size * 0.48f, center.y - size * 0.06f},
                   thick(size, 0.04f), look.accent);
        break;
    case Headwear::Goggles:
    {
        DrawRectangleRec({center.x - size * 0.3f, center.y - size * 0.2f, size * 0.6f, size * 0.12f}, look.hat);
        const float glow = 0.75f + 0.25f * std::sin(time * 5.0f);
        DrawCircleV({center.x - size * 0.11f, center.y + size * 0.05f}, size * 0.08f, Fade(look.accent, glow));
        DrawCircleV({center.x + size * 0.11f, center.y + size * 0.05f}, size * 0.08f, Fade(look.accent, glow));
        break;
    }
    case Headwear::Nemes:
        drawTriangleAnyOrder({center.x, center.y - size * 0.42f}, {center.x - size * 0.4f, center.y + size * 0.3f},
                             {center.x + size * 0.4f, center.y + size * 0.3f}, look.hat);
        for (int i = 0; i < 3; ++i)
        {
            const float y = center.y - size * 0.12f + static_cast<float>(i) * size * 0.13f;
            DrawLineEx({center.x - size * 0.36f, y + size * 0.05f}, {center.x - size * 0.22f, y}, thick(size, 0.04f), look.accent);
            DrawLineEx({center.x + size * 0.22f, y}, {center.x + size * 0.36f, y + size * 0.05f}, thick(size, 0.04f), look.accent);
        }
        DrawCircleV({center.x, center.y + size * 0.08f}, size * 0.25f, look.body);
        break;
    }
}
}  // namespace

void drawExplorerSprite(Vector2 center, float size, float time, int skinId, Direction facing, bool flashlight)
{
    const SkinLook& look = SKIN_LOOKS[(skinId >= 0 && skinId < SKIN_COUNT) ? skinId : 0];
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.38f), size * 0.3f, size * 0.08f,
                Fade(BLACK, 0.4f));

    // The light is held on the side the explorer faces.
    const float side = facing == Direction::Left ? -1.0f : 1.0f;
    const Vector2 hand = {center.x + side * size * 0.36f, center.y - size * 0.12f};
    if (flashlight)
    {
        const Vector2 tail = {hand.x - side * size * 0.1f, center.y + size * 0.16f};
        DrawLineEx(tail, hand, thick(size, 0.12f), {70, 72, 80, 255});
        DrawCircleV(hand, size * 0.07f, {255, 244, 200, 255});
    }
    else
    {
        DrawLineEx({hand.x - side * size * 0.06f, center.y + size * 0.2f}, hand, thick(size, 0.07f), HAT_BROWN);
        const float flicker = 1.0f + 0.15f * std::sin(time * 23.0f) + 0.1f * std::sin(time * 37.0f);
        DrawCircleV({hand.x, hand.y - size * 0.06f}, size * 0.11f * flicker, FLAME);
        DrawCircleV({hand.x, hand.y - size * 0.04f}, size * 0.06f * flicker, FLAME_CORE);
    }

    const Vector2 body = {center.x, center.y + size * 0.08f};
    DrawCircleV(body, size * 0.32f, look.body);
    DrawCircleLinesV(body, size * 0.32f, theme::INK);
    if (look.headwear != Headwear::Goggles)
    {
        DrawCircleV({center.x - size * 0.1f, center.y + size * 0.06f}, thick(size, 0.045f), theme::INK);
        DrawCircleV({center.x + size * 0.1f, center.y + size * 0.06f}, thick(size, 0.045f), theme::INK);
    }
    drawHeadwear(look, center, size, time);
}

void drawTimerEmblem(Location location, Vector2 center, float size, float fraction, float time)
{
    const LocationPalette& p = getPalette(location);
    const float half = size / 2.0f;

    switch (location)
    {
    case Location::Egypt:
    {
        // Hourglass: sand moves from the top bulb to the bottom bulb
        const float bulb = half * 0.85f;
        DrawRectangleRec({center.x - half * 0.7f, center.y - half, half * 1.4f, half * 0.14f}, theme::GOLD_DEEP);
        DrawRectangleRec({center.x - half * 0.7f, center.y + half * 0.86f, half * 1.4f, half * 0.14f}, theme::GOLD_DEEP);
        drawTriangleAnyOrder({center.x - half * 0.58f, center.y - bulb}, {center.x + half * 0.58f, center.y - bulb},
                             center, Fade(theme::PARCHMENT, 0.25f));
        drawTriangleAnyOrder({center.x - half * 0.58f, center.y + bulb}, {center.x + half * 0.58f, center.y + bulb},
                             center, Fade(theme::PARCHMENT, 0.25f));
        if (fraction > 0.01f)
        {
            drawTriangleAnyOrder({center.x - half * 0.58f * fraction, center.y - bulb * fraction},
                                 {center.x + half * 0.58f * fraction, center.y - bulb * fraction}, center, p.groundLight);
            DrawLineEx(center, {center.x, center.y + bulb}, 1.5f, p.groundLight);
        }
        const float pile = 1.0f - fraction;
        drawTriangleAnyOrder({center.x - half * 0.56f, center.y + bulb}, {center.x + half * 0.56f, center.y + bulb},
                             {center.x, center.y + bulb - bulb * pile}, p.groundLight);
        break;
    }
    case Location::Mexico:
    {
        // Eclipse: the moon slides over the sun as time runs out
        const float radius = half * 0.62f;
        DrawCircleGradient(center, radius * 1.6f, Fade(p.sun, 0.4f + 0.3f * (1.0f - fraction)), Fade(p.sun, 0.0f));
        DrawCircleV(center, radius, p.sun);
        const float offset = radius * 2.1f * fraction;
        DrawCircleV({center.x + offset, center.y - offset * 0.15f}, radius * 1.02f, {38, 22, 54, 255});
        break;
    }
    case Location::Cambodia:
    {
        // Flood: the water rises inside a stone basin
        const Rectangle basin = {center.x - half * 0.7f, center.y - half * 0.8f, half * 1.4f, half * 1.6f};
        DrawRectangleRounded(basin, 0.25f, 6, Fade(theme::PANEL_LIGHT, 0.9f));
        const float level = basin.height * (1.0f - fraction);
        if (level > 1.0f)
        {
            const float wave = std::sin(time * 4.0f) * 2.0f;
            DrawRectangleRec({basin.x + 3.0f, basin.y + basin.height - level + wave, basin.width - 6.0f, level - wave - 3.0f},
                             Fade(WATER, 0.85f));
        }
        DrawRectangleRoundedLinesEx(basin, 0.25f, 6, 2.0f, p.wallLight);
        for (int i = 0; i < 3; ++i)
        {
            const float fall = std::fmod(time * 1.4f + static_cast<float>(i) * 0.33f, 1.0f);
            DrawLineEx({center.x - half * 0.4f + static_cast<float>(i) * half * 0.4f, center.y - half - half * 0.3f + fall * half * 0.5f},
                       {center.x - half * 0.45f + static_cast<float>(i) * half * 0.4f, center.y - half - half * 0.1f + fall * half * 0.5f},
                       1.5f, Fade(WATER, 0.8f));
        }
        break;
    }
    }
}
