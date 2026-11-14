#include "ui/sprites.h"

#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/theme.h"

#include <cmath>

// Every picture is built from a few circles, rectangles, lines and triangles.
// Sizes are written as parts of "size" (the tile size), so a picture fits any tile.

namespace
{
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
const Color STICK = {112, 74, 42, 255};

// Line thickness that never gets thinner than 1 pixel.
float thick(float size, float part)
{
    return std::fmax(1.0f, size * part);
}

// A soft shadow under an object.
void drawShadow(Vector2 center, float size)
{
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.3f), size * 0.24f, size * 0.07f,
                Fade(BLACK, 0.35f));
}

// A glow that slowly grows and shrinks.
void drawPulse(Vector2 center, float radius, Color color, float time)
{
    const float pulse = 0.8f + 0.2f * std::sin(time * 3.0f);
    DrawCircleGradient(center, radius * pulse, Fade(color, 0.5f), Fade(color, 0.0f));
}

// A small twinkling star that makes rewards stand out. "offset" lets every item twinkle at its own time.
void drawGlint(Vector2 center, float size, float time, float offset)
{
    const float phase = std::fmod(time * 0.8f + offset, 1.0f);  // 0..1, repeats every 1.25 s
    if (phase > 0.25f)
    {
        return;  // visible only during the first quarter
    }
    const float length = std::sin(phase / 0.25f * PI) * size * 0.22f;  // grows, then shrinks
    const Vector2 c = {center.x + size * 0.18f, center.y - size * 0.18f};
    DrawLineEx({c.x - length, c.y}, {c.x + length, c.y}, thick(size, 0.04f), WHITE);
    DrawLineEx({c.x, c.y - length}, {c.x, c.y + length}, thick(size, 0.04f), WHITE);
}

// Light rays that slowly turn around a centre (relic and exit gate).
void drawRays(Vector2 center, float length, int count, Color color, float time)
{
    for (int i = 0; i < count; ++i)
    {
        const float angle = time * 0.7f + static_cast<float>(i) * 2.0f * PI / static_cast<float>(count);
        DrawLineEx(center, {center.x + std::cos(angle) * length, center.y + std::sin(angle) * length},
                   thick(length, 0.06f), Fade(color, 0.3f));
    }
}

void drawAnkh(Vector2 c, float size, Color color)
{
    DrawRing({c.x, c.y - size * 0.24f}, size * 0.09f, size * 0.17f, 0.0f, 360.0f, 20, color);
    DrawRectangleV({c.x - size * 0.27f, c.y - size * 0.08f}, {size * 0.54f, size * 0.11f}, color);
    DrawRectangleV({c.x - size * 0.06f, c.y - size * 0.08f}, {size * 0.12f, size * 0.54f}, color);
}

void drawScarab(Vector2 c, float size, Color color)
{
    DrawLineEx({c.x - size * 0.4f, c.y - size * 0.1f}, {c.x + size * 0.4f, c.y + size * 0.1f}, thick(size, 0.07f), color);
    DrawLineEx({c.x - size * 0.4f, c.y + size * 0.1f}, {c.x + size * 0.4f, c.y - size * 0.1f}, thick(size, 0.07f), color);
    DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y + size * 0.05f), size * 0.25f, size * 0.32f, color);
    DrawCircleV({c.x, c.y - size * 0.32f}, size * 0.13f, color);
}

void drawLotus(Vector2 c, float size, Color petal, Color heart)
{
    drawTriangle({c.x, c.y - size * 0.45f}, {c.x - size * 0.18f, c.y + size * 0.1f}, {c.x + size * 0.18f, c.y + size * 0.1f}, petal);
    drawTriangle({c.x - size * 0.42f, c.y - size * 0.2f}, {c.x - size * 0.1f, c.y + size * 0.15f}, {c.x, c.y}, petal);
    drawTriangle({c.x + size * 0.42f, c.y - size * 0.2f}, {c.x + size * 0.1f, c.y + size * 0.15f}, {c.x, c.y}, petal);
    DrawCircleV({c.x, c.y + size * 0.02f}, size * 0.08f, heart);
}

void drawSunDisc(Vector2 c, float size, Color color)
{
    for (int i = 0; i < 8; ++i)
    {
        const float angle = static_cast<float>(i) * PI / 4.0f;
        DrawLineEx(c, {c.x + std::cos(angle) * size * 0.45f, c.y + std::sin(angle) * size * 0.45f}, thick(size, 0.08f), color);
    }
    DrawCircleV(c, size * 0.28f, color);
}

void drawWaterDrop(Vector2 c, float size)
{
    DrawCircleV({c.x, c.y + size * 0.1f}, size * 0.24f, WATER);
    drawTriangle({c.x, c.y - size * 0.38f}, {c.x - size * 0.22f, c.y + size * 0.02f}, {c.x + size * 0.22f, c.y + size * 0.02f}, WATER);
}

void drawHourglass(Vector2 c, float size, Color frame, Color sand)
{
    DrawRectangleRec({c.x - size * 0.24f, c.y - size * 0.36f, size * 0.48f, size * 0.07f}, frame);
    DrawRectangleRec({c.x - size * 0.24f, c.y + size * 0.29f, size * 0.48f, size * 0.07f}, frame);
    drawTriangle({c.x - size * 0.19f, c.y - size * 0.29f}, {c.x + size * 0.19f, c.y - size * 0.29f}, c, sand);
    drawTriangle({c.x - size * 0.19f, c.y + size * 0.29f}, {c.x + size * 0.19f, c.y + size * 0.29f}, c, sand);
}
}  // namespace

// ---------------------------------------------------------------- items

void drawKeySprite(Location location, Vector2 center, float size)
{
    const LocationPalette& p = getPalette(location);
    switch (location)
    {
    case Location::Egypt:
        drawAnkh(center, size, p.accentSoft);
        break;
    case Location::Mexico:
        DrawRectangleRounded({center.x - size * 0.3f, center.y - size * 0.3f, size * 0.6f, size * 0.6f}, 0.4f, 6, p.accent);
        DrawRing(center, size * 0.1f, size * 0.15f, 0.0f, 270.0f, 16, p.accentDark);
        DrawCircleV({center.x - size * 0.2f, center.y - size * 0.2f}, size * 0.035f, p.accentDark);
        DrawCircleV({center.x + size * 0.2f, center.y + size * 0.2f}, size * 0.035f, p.accentDark);
        break;
    case Location::Cambodia:
        DrawRectangleV({center.x - size * 0.04f, center.y}, {size * 0.08f, size * 0.38f}, p.accentSoft);
        drawLotus({center.x, center.y - size * 0.06f}, size * 0.8f, p.accent, p.accentSoft);
        break;
    }
}

void drawDoorSprite(Location location, Rectangle tile, float time)
{
    const LocationPalette& p = getPalette(location);
    const Color color = Fade(p.accent, 0.65f + 0.35f * std::sin(time * 3.0f));  // the carving pulses
    const Vector2 c = {tile.x + tile.width / 2.0f, tile.y + tile.height / 2.0f};
    const float size = tile.width;

    DrawRectangleLinesEx(tile, thick(size, 0.08f), color);
    switch (location)
    {
    case Location::Egypt:  // Eye of Horus
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y), size * 0.34f, size * 0.17f, color);
        DrawCircleV(c, size * 0.09f, theme::INK);
        DrawLineEx({c.x - size * 0.3f, c.y - size * 0.26f}, {c.x + size * 0.3f, c.y - size * 0.26f}, thick(size, 0.06f), color);  // brow
        DrawLineEx({c.x - size * 0.04f, c.y + size * 0.16f}, {c.x - size * 0.14f, c.y + size * 0.34f}, thick(size, 0.06f), color);  // tear line
        break;
    case Location::Mexico:  // Serpent glyph
        DrawRectangleRounded({c.x - size * 0.28f, c.y - size * 0.2f, size * 0.56f, size * 0.4f}, 0.5f, 6, color);
        DrawCircleV({c.x + size * 0.08f, c.y - size * 0.05f}, size * 0.06f, theme::INK);
        for (int i = 0; i < 3; ++i)  // feathers
        {
            const float x = c.x - size * 0.2f + static_cast<float>(i) * size * 0.12f;
            DrawLineEx({x, c.y - size * 0.2f}, {x - size * 0.06f, c.y - size * 0.38f}, thick(size, 0.06f), p.accentSoft);
        }
        break;
    case Location::Cambodia:  // Smiling stone face
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y), size * 0.25f, size * 0.32f, color);
        DrawLineEx({c.x - size * 0.14f, c.y - size * 0.05f}, {c.x - size * 0.04f, c.y - size * 0.05f}, thick(size, 0.05f), theme::INK);
        DrawLineEx({c.x + size * 0.04f, c.y - size * 0.05f}, {c.x + size * 0.14f, c.y - size * 0.05f}, thick(size, 0.05f), theme::INK);
        DrawRing({c.x, c.y + size * 0.08f}, size * 0.08f, size * 0.11f, 20.0f, 160.0f, 10, theme::INK);  // smile
        drawTriangle({c.x, c.y - size * 0.46f}, {c.x - size * 0.18f, c.y - size * 0.24f}, {c.x + size * 0.18f, c.y - size * 0.24f},
                     p.accentSoft);  // crown
        break;
    }
}

void drawTreasureSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    drawShadow(center, size);
    switch (location)
    {
    case Location::Egypt:  // gold scarab
        drawScarab(center, size * 0.6f, p.accent);
        break;
    case Location::Mexico:  // gold bead
        DrawCircleV(center, size * 0.22f, theme::GOLD_BRIGHT);
        DrawRing(center, size * 0.12f, size * 0.16f, 0.0f, 360.0f, 16, theme::GOLD_DEEP);
        break;
    case Location::Cambodia:  // silver coin with a square hole
        DrawCircleV(center, size * 0.22f, SILVER);
        DrawRing(center, size * 0.18f, size * 0.22f, 0.0f, 360.0f, 16, SILVER_DARK);
        DrawRectangleRec({center.x - size * 0.06f, center.y - size * 0.06f, size * 0.12f, size * 0.12f}, SILVER_DARK);
        break;
    }
    drawGlint(center, size, time, center.x * 0.01f);
}

void drawGemSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    drawShadow(center, size);
    switch (location)
    {
    case Location::Egypt:  // lapis amulet
        DrawCircleV({center.x, center.y + size * 0.06f}, size * 0.26f, p.accent);  // gold setting
        DrawCircleV({center.x, center.y + size * 0.06f}, size * 0.2f, p.gem);
        drawTriangle({center.x, center.y - size * 0.34f}, {center.x - size * 0.2f, center.y}, {center.x + size * 0.2f, center.y}, p.gem);
        break;
    case Location::Mexico:  // jade mask
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.26f, size * 0.32f, p.gem);
        DrawRectangleRec({center.x - size * 0.16f, center.y - size * 0.1f, size * 0.12f, size * 0.05f}, theme::INK);
        DrawRectangleRec({center.x + size * 0.04f, center.y - size * 0.1f, size * 0.12f, size * 0.05f}, theme::INK);
        DrawRectangleRec({center.x - size * 0.1f, center.y + size * 0.12f, size * 0.2f, size * 0.05f}, theme::INK);
        DrawCircleV({center.x - size * 0.29f, center.y}, size * 0.06f, p.accentDark);  // ear plugs
        DrawCircleV({center.x + size * 0.29f, center.y}, size * 0.06f, p.accentDark);
        break;
    case Location::Cambodia:  // sapphire with a lighter top
        drawDiamond(center, size * 0.3f, p.gem);
        drawTriangle({center.x, center.y - size * 0.3f}, {center.x - size * 0.15f, center.y - size * 0.15f},
                     {center.x + size * 0.15f, center.y - size * 0.15f}, ColorLerp(p.gem, WHITE, 0.35f));
        break;
    }
    DrawCircleV({center.x - size * 0.07f, center.y - size * 0.05f}, size * 0.045f, Fade(WHITE, 0.6f));  // shine
    drawGlint(center, size, time, center.y * 0.01f);
}

void drawRelicSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    drawPulse(center, size * 0.9f, p.accent, time);
    drawRays(center, size * 0.75f, 8, p.accent, time);
    const Vector2 c = {center.x, center.y + std::sin(time * 2.5f) * size * 0.04f};  // floats up and down

    switch (location)
    {
    case Location::Egypt:  // golden death mask
        drawTriangle({c.x, c.y - size * 0.42f}, {c.x - size * 0.38f, c.y + size * 0.36f}, {c.x + size * 0.38f, c.y + size * 0.36f},
                     theme::GOLD_BRIGHT);
        for (int i = 0; i < 3; ++i)  // blue stripes of the headdress
        {
            const float y = c.y - size * 0.05f + static_cast<float>(i) * size * 0.12f;
            DrawLineEx({c.x - size * 0.3f, y}, {c.x + size * 0.3f, y}, thick(size, 0.04f), p.gem);
        }
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y + size * 0.06f), size * 0.16f, size * 0.23f,
                    ColorLerp(theme::GOLD_BRIGHT, WHITE, 0.25f));  // the face
        DrawLineEx({c.x - size * 0.11f, c.y}, {c.x - size * 0.03f, c.y}, thick(size, 0.04f), theme::INK);
        DrawLineEx({c.x + size * 0.03f, c.y}, {c.x + size * 0.11f, c.y}, thick(size, 0.04f), theme::INK);
        break;
    case Location::Mexico:  // sun disc
        drawSunDisc(c, size * 1.1f, theme::GOLD_BRIGHT);
        DrawCircleV(c, size * 0.15f, p.accent);
        break;
    case Location::Cambodia:  // golden lotus
        drawLotus(c, size * 1.1f, theme::GOLD_BRIGHT, p.accent);
        break;
    }
}

void drawTimePickupSprite(Location location, Vector2 center, float size, float time)
{
    const LocationPalette& p = getPalette(location);
    drawPulse(center, size * 0.5f, p.glow, time);
    switch (location)
    {
    case Location::Egypt:
        drawHourglass(center, size, theme::GOLD_DEEP, p.groundLight);
        break;
    case Location::Mexico:
        drawSunDisc(center, size * 0.8f, theme::GOLD_BRIGHT);
        break;
    case Location::Cambodia:
        drawWaterDrop(center, size * 0.8f);
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
        return;  // an empty pit: already safe
    }

    switch (location)
    {
    case Location::Egypt:  // scarabs in the pit
        drawScarab(center, size * 0.45f, p.accentSoft);
        break;
    case Location::Mexico:  // spikes
        for (int i = -1; i <= 1; ++i)
        {
            const float x = center.x + static_cast<float>(i) * size * 0.15f;
            drawTriangle({x, center.y - size * 0.22f}, {x - size * 0.07f, center.y + size * 0.15f}, {x + size * 0.07f, center.y + size * 0.15f},
                         SILVER);
        }
        break;
    case Location::Cambodia:  // a cobra
        DrawRing({center.x, center.y + size * 0.08f}, size * 0.1f, size * 0.17f, 0.0f, 360.0f, 16, NAGA);
        DrawCircleV({center.x, center.y - size * 0.1f}, size * 0.12f, NAGA);
        break;
    }
}

void drawGuardianSprite(Location location, Vector2 center, float size, float time)
{
    const float sway = std::sin(time * 6.0f) * size * 0.03f;  // moves a little from side to side
    const Vector2 c = {center.x + sway, center.y};
    DrawCircleGradient(center, size * 0.7f, Fade(GUARDIAN_EYES, 0.25f), Fade(GUARDIAN_EYES, 0.0f));

    switch (location)
    {
    case Location::Egypt:  // mummy: a wrapped body with bandage lines
        DrawRectangleRounded({c.x - size * 0.3f, c.y - size * 0.4f, size * 0.6f, size * 0.8f}, 0.5f, 6, BANDAGE);
        DrawLineEx({c.x - size * 0.28f, c.y + size * 0.05f}, {c.x + size * 0.28f, c.y + size * 0.13f}, thick(size, 0.05f), BANDAGE_DARK);
        DrawLineEx({c.x - size * 0.28f, c.y + size * 0.22f}, {c.x + size * 0.28f, c.y + size * 0.3f}, thick(size, 0.05f), BANDAGE_DARK);
        DrawLineEx({c.x - size * 0.28f, c.y - size * 0.25f}, {c.x + size * 0.28f, c.y - size * 0.3f}, thick(size, 0.05f), BANDAGE_DARK);
        break;
    case Location::Mexico:  // jaguar head with ears and spots
        drawTriangle({c.x - size * 0.34f, c.y - size * 0.38f}, {c.x - size * 0.34f, c.y}, {c.x, c.y - size * 0.2f}, JAGUAR);
        drawTriangle({c.x + size * 0.34f, c.y - size * 0.38f}, {c.x + size * 0.34f, c.y}, {c.x, c.y - size * 0.2f}, JAGUAR);
        DrawCircleV(c, size * 0.34f, JAGUAR);
        for (int i = 0; i < 6; ++i)  // spots in a ring around the face
        {
            const float angle = static_cast<float>(i) * 1.05f + 0.4f;
            DrawCircleV({c.x + std::cos(angle) * size * 0.24f, c.y + std::sin(angle) * size * 0.2f}, size * 0.035f, theme::INK);
        }
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y + size * 0.1f), size * 0.13f, size * 0.09f, theme::PARCHMENT);  // muzzle
        DrawCircleV({c.x, c.y + size * 0.06f}, size * 0.035f, theme::INK);
        break;
    case Location::Cambodia:  // naga: a coiled serpent with a raised hood
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y + size * 0.28f), size * 0.36f, size * 0.13f, NAGA);
        DrawLineEx({c.x, c.y}, {c.x, c.y + size * 0.26f}, thick(size, 0.14f), NAGA);
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y - size * 0.12f), size * 0.3f, size * 0.26f, NAGA);
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y - size * 0.02f), size * 0.12f, size * 0.16f, NAGA_LIGHT);
        break;
    }
    // Glowing red eyes
    DrawCircleV({c.x - size * 0.11f, c.y - size * 0.15f}, thick(size, 0.065f), GUARDIAN_EYES);
    DrawCircleV({c.x + size * 0.11f, c.y - size * 0.15f}, thick(size, 0.065f), GUARDIAN_EYES);
}

void drawExitSprite(Location location, Rectangle tile, float time)
{
    const LocationPalette& p = getPalette(location);
    const Vector2 center = {tile.x + tile.width / 2.0f, tile.y + tile.height / 2.0f};

    drawPulse(center, tile.width * 2.2f, p.accent, time);  // shines through the dark
    drawRays(center, tile.width * 1.3f, 6, p.accent, time);
    DrawRectangleGradientV(static_cast<int>(tile.x), static_cast<int>(tile.y), static_cast<int>(tile.width),
                           static_cast<int>(tile.height), ColorLerp(p.accent, WHITE, 0.55f), p.accent);
    DrawRectangleLinesEx(tile, thick(tile.width, 0.12f), p.accentDark);
    drawLocationEmblem(location, center, tile.height * 0.6f, Fade(theme::INK, 0.85f));
}

// ---------------------------------------------------------------- the explorer (skins)

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

// The colours and hat of every skin (names and prices are in core/skins.cpp).
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

void drawHeadwear(const SkinLook& look, Vector2 c, float size)
{
    switch (look.headwear)
    {
    case Headwear::Fedora:  // brim + crown + band
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y - size * 0.1f), size * 0.44f, size * 0.12f, look.hat);
        DrawRectangleRec({c.x - size * 0.19f, c.y - size * 0.36f, size * 0.38f, size * 0.27f}, look.hat);
        DrawRectangleRec({c.x - size * 0.19f, c.y - size * 0.17f, size * 0.38f, thick(size, 0.05f)}, look.accent);
        break;
    case Headwear::Headwrap:
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y - size * 0.16f), size * 0.33f, size * 0.2f, look.hat);
        DrawLineEx({c.x + size * 0.2f, c.y - size * 0.1f}, {c.x + size * 0.36f, c.y + size * 0.22f}, thick(size, 0.08f), look.hat);  // tail
        DrawCircleV({c.x, c.y - size * 0.24f}, size * 0.05f, look.accent);
        break;
    case Headwear::PithHelmet:  // a half circle on a brim
        DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y - size * 0.1f), size * 0.42f, size * 0.1f, look.hat);
        DrawCircleSector({c.x, c.y - size * 0.1f}, size * 0.26f, 180.0f, 360.0f, 16, look.hat);
        break;
    case Headwear::StrawHat:  // a wide cone
        drawTriangle({c.x, c.y - size * 0.42f}, {c.x - size * 0.48f, c.y - size * 0.06f}, {c.x + size * 0.48f, c.y - size * 0.06f}, look.hat);
        break;
    case Headwear::Goggles:
        DrawRectangleRec({c.x - size * 0.3f, c.y - size * 0.2f, size * 0.6f, size * 0.12f}, look.hat);
        DrawCircleV({c.x - size * 0.11f, c.y + size * 0.05f}, size * 0.08f, look.accent);
        DrawCircleV({c.x + size * 0.11f, c.y + size * 0.05f}, size * 0.08f, look.accent);
        break;
    case Headwear::Nemes:  // pharaoh headdress around the face
        drawTriangle({c.x, c.y - size * 0.42f}, {c.x - size * 0.4f, c.y + size * 0.3f}, {c.x + size * 0.4f, c.y + size * 0.3f}, look.hat);
        for (int i = 0; i < 3; ++i)  // gold stripes on both sides
        {
            const float y = c.y - size * 0.1f + static_cast<float>(i) * size * 0.13f;
            DrawLineEx({c.x - size * 0.36f, y}, {c.x - size * 0.22f, y}, thick(size, 0.04f), look.accent);
            DrawLineEx({c.x + size * 0.22f, y}, {c.x + size * 0.36f, y}, thick(size, 0.04f), look.accent);
        }
        DrawCircleV({c.x, c.y + size * 0.08f}, size * 0.25f, look.body);
        break;
    }
}
}  // namespace

void drawExplorerSprite(Vector2 center, float size, float time, int skinId, Direction facing, bool flashlight)
{
    const SkinLook& look = SKIN_LOOKS[(skinId >= 0 && skinId < SKIN_COUNT) ? skinId : 0];
    drawShadow({center.x, center.y + size * 0.08f}, size);

    // The torch or flashlight is held on the side the explorer faces.
    const float side = facing == Direction::Left ? -1.0f : 1.0f;
    const Vector2 hand = {center.x + side * size * 0.36f, center.y - size * 0.12f};
    DrawLineEx({hand.x - side * size * 0.08f, center.y + size * 0.18f}, hand, thick(size, 0.09f), flashlight ? DARKGRAY : STICK);
    if (flashlight)
    {
        DrawCircleV(hand, size * 0.07f, {255, 244, 200, 255});
    }
    else
    {
        const float flicker = 1.0f + 0.15f * std::sin(time * 23.0f);  // the flame flickers
        DrawCircleV({hand.x, hand.y - size * 0.05f}, size * 0.1f * flicker, FLAME);
        DrawCircleV({hand.x, hand.y - size * 0.04f}, size * 0.05f * flicker, FLAME_CORE);
    }

    // Round body, the hat of the skin, then the eyes
    const Vector2 body = {center.x, center.y + size * 0.08f};
    DrawCircleV(body, size * 0.32f, look.body);
    DrawCircleLinesV(body, size * 0.32f, theme::INK);
    drawHeadwear(look, center, size);
    if (look.headwear != Headwear::Goggles)  // goggles already cover the eyes
    {
        DrawCircleV({center.x - size * 0.1f, center.y + size * 0.06f}, thick(size, 0.045f), theme::INK);
        DrawCircleV({center.x + size * 0.1f, center.y + size * 0.06f}, thick(size, 0.045f), theme::INK);
    }
}

// ---------------------------------------------------------------- timer pictures

void drawTimerEmblem(Location location, Vector2 center, float size, float fraction, float time)
{
    const LocationPalette& p = getPalette(location);
    const float half = size / 2.0f;

    switch (location)
    {
    case Location::Egypt:
    {
        // Hourglass: the sand in the top half shrinks as time runs out
        drawHourglass(center, size, theme::GOLD_DEEP, Fade(theme::PARCHMENT, 0.25f));
        const float sand = size * 0.29f * fraction;
        drawTriangle({center.x - sand * 0.65f, center.y - sand}, {center.x + sand * 0.65f, center.y - sand}, center, p.groundLight);
        const float pile = size * 0.29f * (1.0f - fraction);
        drawTriangle({center.x - size * 0.19f, center.y + size * 0.29f}, {center.x + size * 0.19f, center.y + size * 0.29f},
                     {center.x, center.y + size * 0.29f - pile}, p.groundLight);
        break;
    }
    case Location::Mexico:
    {
        // Eclipse: the moon slides over the sun as time runs out
        const float radius = half * 0.62f;
        DrawCircleGradient(center, radius * 1.6f, Fade(p.sun, 0.4f + 0.3f * (1.0f - fraction)), Fade(p.sun, 0.0f));
        DrawCircleV(center, radius, p.sun);
        const float offset = radius * 2.1f * fraction;
        DrawCircleV({center.x + offset, center.y}, radius * 1.02f, {38, 22, 54, 255});
        break;
    }
    case Location::Cambodia:
    {
        // Flood: the water rises inside a stone basin
        const Rectangle basin = {center.x - half * 0.7f, center.y - half * 0.8f, half * 1.4f, half * 1.6f};
        DrawRectangleRec(basin, theme::PANEL_LIGHT);
        const float level = (basin.height - 6.0f) * (1.0f - fraction);
        const float wave = std::sin(time * 4.0f) * 2.0f * (level > 4.0f ? 1.0f : 0.0f);
        DrawRectangleRec({basin.x + 3.0f, basin.y + basin.height - 3.0f - level + wave, basin.width - 6.0f, level - wave}, WATER);
        DrawRectangleLinesEx(basin, 2.0f, p.wallLight);
        for (int i = 0; i < 3; ++i)  // rain drops falling into the basin
        {
            const float fall = std::fmod(time * 1.4f + static_cast<float>(i) * 0.33f, 1.0f);
            const float x = center.x - half * 0.4f + static_cast<float>(i) * half * 0.4f;
            const float y = basin.y - half * 0.3f + fall * half * 0.3f;
            DrawLineEx({x, y}, {x - 1.0f, y + half * 0.2f}, 1.5f, WATER);
        }
        break;
    }
    }
}
