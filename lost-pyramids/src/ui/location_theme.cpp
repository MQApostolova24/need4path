#include "ui/location_theme.h"

#include "ui/draw_utils.h"
#include "ui/theme.h"

#include <cmath>

namespace
{
const LocationPalette PALETTES[3] = {
    // Egypt - moonlit desert, sandstone and gold
    {
        {14, 16, 44, 255}, {112, 58, 78, 255}, {250, 238, 206, 255}, {96, 62, 58, 255}, {150, 106, 62, 255},
        {166, 118, 64, 255}, {214, 170, 104, 255}, {222, 176, 104, 255}, {140, 96, 58, 255},
        {172, 128, 76, 255}, {212, 170, 110, 255}, {110, 76, 44, 255}, {92, 62, 36, 255},
        {62, 45, 31, 255}, {70, 51, 35, 255}, {46, 33, 22, 255},
        {244, 196, 64, 255}, {168, 124, 30, 255}, {64, 200, 190, 255}, {48, 96, 208, 255},
        {255, 196, 120, 255}, {236, 204, 148, 255},
    },
    // Mexico - jungle sunset, grey limestone and jade
    {
        {48, 24, 74, 255}, {240, 124, 62, 255}, {255, 214, 128, 255}, {58, 52, 84, 255}, {24, 58, 40, 255},
        {18, 44, 30, 255}, {40, 86, 50, 255}, {176, 168, 146, 255}, {112, 108, 94, 255},
        {120, 122, 108, 255}, {162, 162, 142, 255}, {76, 78, 68, 255}, {78, 128, 62, 255},
        {42, 47, 39, 255}, {49, 55, 45, 255}, {32, 36, 29, 255},
        {72, 196, 136, 255}, {32, 120, 80, 255}, {214, 84, 56, 255}, {60, 186, 120, 255},
        {255, 184, 104, 255}, {214, 255, 140, 255},
    },
    // Cambodia - monsoon dawn, red laterite and lotus pink
    {
        {46, 78, 108, 255}, {246, 176, 146, 255}, {255, 230, 196, 255}, {84, 110, 118, 255}, {32, 70, 56, 255},
        {28, 62, 52, 255}, {58, 104, 78, 255}, {176, 122, 98, 255}, {114, 78, 66, 255},
        {148, 98, 78, 255}, {186, 134, 106, 255}, {94, 62, 50, 255}, {92, 132, 86, 255},
        {53, 44, 40, 255}, {61, 51, 46, 255}, {40, 32, 29, 255},
        {240, 146, 176, 255}, {170, 82, 112, 255}, {232, 190, 92, 255}, {80, 96, 232, 255},
        {255, 208, 150, 255}, {176, 206, 236, 255},
    },
};

const float MENU_SCENE_SECONDS = 9.0f;
const float MENU_FADE_SECONDS = 1.2f;

// ---------------------------------------------------------------- shared scenery

void drawSky(const Frame& f, const LocationPalette& p)
{
    DrawRectangleGradientV(static_cast<int>(f.area.x), static_cast<int>(f.area.y), static_cast<int>(f.area.width),
                           static_cast<int>(f.area.height), p.skyTop, p.skyBottom);
}

void drawGlowDisc(Vector2 center, float radius, Color color, Color glow)
{
    DrawCircleGradient(center, radius * 3.2f, Fade(glow, 0.22f), Fade(glow, 0.0f));
    DrawCircleGradient(center, radius * 1.7f, Fade(glow, 0.35f), Fade(glow, 0.0f));
    DrawCircleV(center, radius, color);
}

// A palm silhouette: curved trunk and drooping leaves.
void drawPalm(Vector2 base, float height, Color color, float sway)
{
    Vector2 previous = base;
    const int segments = 6;
    for (int i = 1; i <= segments; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const Vector2 point = {base.x + sway * height * 0.25f * t * t, base.y - height * t};
        DrawLineEx(previous, point, height * (0.06f - 0.025f * t), color);
        previous = point;
    }

    const Vector2 top = previous;
    for (int leaf = 0; leaf < 7; ++leaf)
    {
        const float angle = -2.9f + static_cast<float>(leaf) * 0.48f + sway * 0.1f;
        const float length = height * 0.42f;
        const Vector2 middle = {top.x + std::cos(angle) * length * 0.55f, top.y + std::sin(angle) * length * 0.35f};
        const Vector2 tip = {top.x + std::cos(angle) * length, top.y + std::sin(angle) * length * 0.35f + length * 0.3f};
        DrawLineEx(top, middle, height * 0.035f, color);
        DrawLineEx(middle, tip, height * 0.022f, color);
    }
}

// Tiered pyramid (Mexico and Cambodia) built from rectangles.
// "mirror" draws an upside-down copy below "baseY" (used for the reflection in the moat).
void drawTieredPyramid(float centerX, float baseY, float baseWidth, float tierHeight, int tiers, float shrink,
                       Color lit, Color shade, bool mirror, float alpha)
{
    for (int i = 0; i < tiers; ++i)
    {
        const float width = baseWidth * (1.0f - static_cast<float>(i) * shrink);
        float top = baseY - static_cast<float>(i + 1) * tierHeight;
        if (mirror)
        {
            top = baseY + static_cast<float>(i) * tierHeight;
        }
        const float left = centerX - width / 2.0f;

        DrawRectangleRec({left, top, width, tierHeight}, Fade(lit, alpha));
        // Shadow on the right side and under the ledge give the tier depth.
        DrawRectangleRec({left + width * 0.78f, top, width * 0.22f, tierHeight}, Fade(shade, alpha));
        const float ledgeY = mirror ? top : top + tierHeight * 0.72f;
        DrawRectangleRec({left, ledgeY, width, tierHeight * 0.28f}, Fade(shade, alpha * 0.85f));
    }
}

void drawStairs(float centerX, float baseY, float width, float height, int steps, Color color, Color line,
                bool mirror, float alpha)
{
    const float top = mirror ? baseY : baseY - height;
    DrawRectangleRec({centerX - width / 2.0f, top, width, height}, Fade(color, alpha));
    for (int i = 0; i < steps; ++i)
    {
        const float y = top + height * static_cast<float>(i) / static_cast<float>(steps);
        DrawLineEx({centerX - width / 2.0f, y}, {centerX + width / 2.0f, y}, 1.0f, Fade(line, alpha * 0.7f));
    }
    // Side rails
    DrawRectangleRec({centerX - width / 2.0f - 2.0f, top, 3.0f, height}, Fade(line, alpha));
    DrawRectangleRec({centerX + width / 2.0f - 1.0f, top, 3.0f, height}, Fade(line, alpha));
}

// ---------------------------------------------------------------- Egypt

void drawSmoothPyramid(Vector2 apex, float halfWidth, float baseY, const LocationPalette& p)
{
    const Vector2 left = {apex.x - halfWidth, baseY};
    const Vector2 middle = {apex.x + halfWidth * 0.28f, baseY};
    const Vector2 right = {apex.x + halfWidth, baseY};

    drawTriangleAnyOrder(apex, left, middle, p.monumentLit);    // moon-lit side
    drawTriangleAnyOrder(apex, middle, right, p.monumentShade); // side in shadow

    // Rows of stone blocks
    const int rows = 9;
    for (int i = 1; i <= rows; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(rows + 1);
        const float y = apex.y + (baseY - apex.y) * t;
        const float xLeft = apex.x + (left.x - apex.x) * t;
        const float xRight = apex.x + (right.x - apex.x) * t;
        DrawLineEx({xLeft, y}, {xRight, y}, 1.0f, Fade(p.wallDark, 0.35f));
    }

    // Gold capstone
    const float capT = 0.09f;
    drawTriangleAnyOrder(apex, {apex.x + (left.x - apex.x) * capT, apex.y + (baseY - apex.y) * capT},
                         {apex.x + (right.x - apex.x) * capT, apex.y + (baseY - apex.y) * capT}, p.accent);
}

void drawEgyptScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);

    // Stars
    for (int i = 0; i < 80; ++i)
    {
        const float twinkle = 0.5f + 0.5f * std::sin(time * (1.0f + hash01(i, 3, 1)) + static_cast<float>(i));
        DrawCircleV(f.at(hash01(i, 1, 1), hash01(i, 2, 1) * 0.55f), 1.0f + hash01(i, 4, 1),
                    Fade(p.sun, 0.2f + 0.6f * twinkle));
    }

    // Moon
    const float scale = std::fmin(f.area.width, f.area.height);
    drawGlowDisc(f.at(0.82f, 0.2f), scale * 0.055f, p.sun, p.sun);
    DrawCircleV({f.x(0.82f) + scale * 0.015f, f.y(0.2f) - scale * 0.01f}, scale * 0.012f, Fade(p.hillFar, 0.25f));

    const float horizon = f.y(0.7f);
    DrawEllipse(static_cast<int>(f.x(0.25f)), static_cast<int>(horizon + f.h(0.01f)), f.w(0.6f), f.h(0.06f),
                p.hillFar);

    drawSmoothPyramid(f.at(0.3f, 0.34f), f.w(0.23f), horizon + f.h(0.02f), p);
    drawSmoothPyramid(f.at(0.58f, 0.47f), f.w(0.15f), horizon + f.h(0.015f), p);
    drawSmoothPyramid(f.at(0.79f, 0.55f), f.w(0.1f), horizon + f.h(0.02f), p);

    // Sand dunes
    DrawRectangleGradientV(static_cast<int>(f.area.x), static_cast<int>(horizon), static_cast<int>(f.area.width),
                           static_cast<int>(f.area.y + f.area.height - horizon) + 1, p.ground, p.hillNear);
    DrawEllipse(static_cast<int>(f.x(0.2f)), static_cast<int>(f.y(0.86f)), f.w(0.55f), f.h(0.1f), p.groundLight);
    DrawEllipse(static_cast<int>(f.x(0.85f)), static_cast<int>(f.y(0.95f)), f.w(0.6f), f.h(0.12f),
                ColorLerp(p.groundLight, p.ground, 0.4f));

    drawPalm(f.at(0.06f, 0.92f), f.h(0.3f), Fade(p.skyTop, 0.9f), 0.6f);
    drawPalm(f.at(0.95f, 0.98f), f.h(0.24f), Fade(p.skyTop, 0.9f), -0.5f);

    // Drifting sand
    for (int i = 0; i < 40; ++i)
    {
        const float speed = 0.02f + 0.03f * hash01(i, 5, 1);
        const float fx = std::fmod(hash01(i, 6, 1) + time * speed, 1.0f);
        const float fy = 0.65f + 0.33f * hash01(i, 7, 1) + 0.01f * std::sin(time * 2.0f + static_cast<float>(i));
        DrawCircleV(f.at(fx, fy), 1.5f, Fade(p.particle, 0.5f));
    }
}

// ---------------------------------------------------------------- Mexico

void drawMexicoScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);

    // The sun during an eclipse - the timer of this location
    const float scale = std::fmin(f.area.width, f.area.height);
    const Vector2 sun = f.at(0.76f, 0.3f);
    drawGlowDisc(sun, scale * 0.09f, p.sun, p.sun);
    DrawCircleV({sun.x + scale * 0.045f, sun.y - scale * 0.01f}, scale * 0.088f, ColorLerp(p.skyTop, p.skyBottom, 0.45f));

    // Distant mountains
    for (int i = 0; i < 5; ++i)
    {
        const float cx = f.x(-0.05f + 0.27f * static_cast<float>(i));
        DrawEllipse(static_cast<int>(cx), static_cast<int>(f.y(0.74f)), f.w(0.2f), f.h(0.16f + 0.05f * hash01(i, 1, 2)),
                    p.hillFar);
    }

    // El Castillo style step pyramid
    const float baseY = f.y(0.8f);
    const float centerX = f.x(0.42f);
    const float baseWidth = f.w(0.46f);
    const float tierHeight = f.h(0.036f);
    const int tiers = 9;
    drawTieredPyramid(centerX, baseY, baseWidth, tierHeight, tiers, 0.075f, p.monumentLit, p.monumentShade, false, 1.0f);
    drawStairs(centerX, baseY, baseWidth * 0.12f, tierHeight * static_cast<float>(tiers), 18,
               ColorLerp(p.monumentLit, WHITE, 0.15f), p.monumentShade, false, 1.0f);

    // Temple on top with three dark doorways
    const float templeWidth = baseWidth * 0.22f;
    const float templeTop = baseY - tierHeight * static_cast<float>(tiers) - tierHeight * 2.3f;
    DrawRectangleRec({centerX - templeWidth / 2.0f, templeTop, templeWidth, tierHeight * 2.3f}, p.monumentLit);
    DrawRectangleRec({centerX - templeWidth / 2.0f, templeTop, templeWidth, tierHeight * 0.6f}, p.accentSoft);
    for (int i = -1; i <= 1; ++i)
    {
        const float doorX = centerX + static_cast<float>(i) * templeWidth * 0.3f;
        DrawRectangleRec({doorX - templeWidth * 0.07f, templeTop + tierHeight * 1.0f, templeWidth * 0.14f,
                          tierHeight * 1.3f},
                         p.skyTop);
    }

    // Serpent heads at the bottom of the stairs
    DrawCircleV({centerX - baseWidth * 0.07f, baseY - tierHeight * 0.4f}, tierHeight * 0.5f, p.accent);
    DrawCircleV({centerX + baseWidth * 0.07f, baseY - tierHeight * 0.4f}, tierHeight * 0.5f, p.accent);

    // Jungle
    DrawRectangleRec({f.area.x, baseY, f.area.width, f.area.y + f.area.height - baseY}, p.ground);
    for (int i = 0; i < 22; ++i)
    {
        const float fx = static_cast<float>(i) / 21.0f;
        const float radius = f.h(0.06f + 0.05f * hash01(i, 2, 2));
        const Color leaf = (i % 2 == 0) ? p.hillNear : p.groundLight;
        DrawCircleV(f.at(fx, 0.85f + 0.06f * hash01(i, 3, 2)), radius, leaf);
    }
    drawPalm(f.at(0.86f, 0.92f), f.h(0.42f), p.ground, -0.7f);
    drawPalm(f.at(0.05f, 0.95f), f.h(0.36f), p.ground, 0.8f);
    drawPalm(f.at(0.95f, 0.98f), f.h(0.3f), p.hillNear, -0.3f);

    // Fireflies
    for (int i = 0; i < 30; ++i)
    {
        const float blink = std::fmax(0.0f, std::sin(time * (1.5f + hash01(i, 4, 2)) + static_cast<float>(i) * 1.7f));
        const float fx = hash01(i, 5, 2) + 0.015f * std::sin(time * 0.7f + static_cast<float>(i));
        const float fy = 0.62f + 0.35f * hash01(i, 6, 2) + 0.015f * std::cos(time * 0.9f + static_cast<float>(i));
        DrawCircleV(f.at(fx, fy), 4.0f, Fade(p.particle, 0.2f * blink));
        DrawCircleV(f.at(fx, fy), 1.6f, Fade(p.particle, 0.9f * blink));
    }
}

// ---------------------------------------------------------------- Cambodia

// An Angkor-style temple tower: stacked tiers with a lotus-bud top.
void drawPrasat(float centerX, float baseY, float width, float height, const LocationPalette& p, bool mirror,
                float alpha)
{
    const int tiers = 5;
    const float bodyHeight = height * 0.75f;
    const float tierHeight = bodyHeight / static_cast<float>(tiers);
    drawTieredPyramid(centerX, baseY, width, tierHeight, tiers, 0.14f, p.monumentLit, p.monumentShade, mirror, alpha);

    const float budWidth = width * 0.34f;
    const float budHeight = height - bodyHeight;
    const float sign = mirror ? -1.0f : 1.0f;
    const float budBase = baseY - sign * bodyHeight;
    const Vector2 tip = {centerX, budBase - sign * budHeight};
    DrawEllipse(static_cast<int>(centerX), static_cast<int>(budBase - sign * budHeight * 0.3f), budWidth * 0.5f,
                budHeight * 0.4f, Fade(p.monumentLit, alpha));
    drawTriangleAnyOrder(tip, {centerX - budWidth * 0.35f, budBase - sign * budHeight * 0.45f},
                         {centerX + budWidth * 0.35f, budBase - sign * budHeight * 0.45f}, Fade(p.monumentLit, alpha));
}

// Sugar palm: thin straight trunk with a round crown.
void drawSugarPalm(Vector2 base, float height, Color color)
{
    const Vector2 top = {base.x, base.y - height};
    DrawLineEx(base, top, height * 0.035f, color);
    for (int leaf = 0; leaf < 12; ++leaf)
    {
        const float angle = static_cast<float>(leaf) * PI / 6.0f;
        DrawLineEx(top, {top.x + std::cos(angle) * height * 0.22f, top.y + std::sin(angle) * height * 0.16f},
                   height * 0.02f, color);
    }
    DrawCircleV(top, height * 0.06f, color);
}

void drawCambodiaScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);

    const float scale = std::fmin(f.area.width, f.area.height);
    drawGlowDisc(f.at(0.22f, 0.4f), scale * 0.07f, p.sun, p.sun);

    // Monsoon clouds
    for (int i = 0; i < 7; ++i)
    {
        const float fx = std::fmod(hash01(i, 1, 3) + time * 0.004f, 1.2f) - 0.1f;
        DrawEllipse(static_cast<int>(f.x(fx)), static_cast<int>(f.y(0.08f + 0.12f * hash01(i, 2, 3))),
                    f.w(0.12f + 0.08f * hash01(i, 3, 3)), f.h(0.05f), Fade(p.skyTop, 0.55f));
    }

    const float waterLine = f.y(0.7f);

    // Forest hills
    for (int i = 0; i < 6; ++i)
    {
        DrawEllipse(static_cast<int>(f.x(-0.05f + 0.22f * static_cast<float>(i))), static_cast<int>(waterLine),
                    f.w(0.16f), f.h(0.09f + 0.04f * hash01(i, 4, 3)), p.hillFar);
    }

    // Koh Ker style seven-tiered pyramid and three temple towers
    const float pyramidX = f.x(0.62f);
    const float pyramidWidth = f.w(0.36f);
    const float tierHeight = f.h(0.05f);
    drawTieredPyramid(pyramidX, waterLine, pyramidWidth, tierHeight, 7, 0.1f, p.monumentLit, p.monumentShade, false,
                      1.0f);
    drawStairs(pyramidX, waterLine, pyramidWidth * 0.09f, tierHeight * 7.0f, 21,
               ColorLerp(p.monumentLit, WHITE, 0.12f), p.monumentShade, false, 1.0f);
    DrawRectangleRec({pyramidX - pyramidWidth * 0.06f, waterLine - tierHeight * 8.2f, pyramidWidth * 0.12f,
                      tierHeight * 1.2f},
                     p.monumentShade);

    const float towerX[3] = {0.14f, 0.24f, 0.34f};
    const float towerHeight[3] = {0.26f, 0.34f, 0.26f};
    for (int i = 0; i < 3; ++i)
    {
        drawPrasat(f.x(towerX[i]), waterLine, f.w(0.07f), f.h(towerHeight[i]), p, false, 1.0f);
    }

    // The moat with a reflection
    const Color water = ColorLerp(p.skyTop, p.hillNear, 0.35f);
    DrawRectangleGradientV(static_cast<int>(f.area.x), static_cast<int>(waterLine), static_cast<int>(f.area.width),
                           static_cast<int>(f.area.y + f.area.height - waterLine) + 1, ColorLerp(water, p.skyBottom, 0.3f),
                           water);
    drawTieredPyramid(pyramidX, waterLine, pyramidWidth, tierHeight * 0.7f, 7, 0.1f, p.monumentLit, p.monumentShade,
                      true, 0.25f);
    for (int i = 0; i < 3; ++i)
    {
        drawPrasat(f.x(towerX[i]), waterLine, f.w(0.07f), f.h(towerHeight[i] * 0.7f), p, true, 0.25f);
    }
    for (int i = 0; i < 26; ++i)
    {
        const float fx = std::fmod(hash01(i, 5, 3) + time * 0.01f * (hash01(i, 6, 3) - 0.5f) + 1.0f, 1.0f);
        const float fy = 0.72f + 0.26f * hash01(i, 7, 3);
        DrawLineEx(f.at(fx, fy), f.at(fx + 0.04f, fy), 1.5f, Fade(p.sun, 0.25f));
    }

    // Lotus flowers floating on the water
    for (int i = 0; i < 6; ++i)
    {
        const Vector2 lotus = f.at(0.08f + 0.17f * static_cast<float>(i), 0.9f + 0.05f * hash01(i, 8, 3));
        DrawEllipse(static_cast<int>(lotus.x), static_cast<int>(lotus.y + 3.0f), f.w(0.025f), f.h(0.012f), p.groundLight);
        DrawCircleV(lotus, f.h(0.012f), p.accent);
    }

    drawSugarPalm(f.at(0.9f, 0.74f), f.h(0.42f), p.ground);
    drawSugarPalm(f.at(0.96f, 0.76f), f.h(0.32f), p.ground);
    drawSugarPalm(f.at(0.03f, 0.74f), f.h(0.36f), p.ground);

    // Monsoon rain
    for (int i = 0; i < 70; ++i)
    {
        const float fall = std::fmod(hash01(i, 9, 3) + time * (0.5f + 0.3f * hash01(i, 10, 3)), 1.0f);
        const float fx = std::fmod(hash01(i, 11, 3) - fall * 0.1f + 1.0f, 1.0f);
        const Vector2 top = f.at(fx, fall);
        DrawLineEx(top, {top.x - 3.0f, top.y + f.h(0.03f)}, 1.0f, Fade(p.particle, 0.35f));
    }
}
}  // namespace

const LocationPalette& getPalette(Location location)
{
    return PALETTES[static_cast<int>(location)];
}

void drawLocationScene(Location location, Rectangle area, float time)
{
    const Frame frame{area};
    const LocationPalette& palette = getPalette(location);

    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(area.width),
                     static_cast<int>(area.height));
    switch (location)
    {
    case Location::Egypt:
        drawEgyptScene(frame, palette, time);
        break;
    case Location::Mexico:
        drawMexicoScene(frame, palette, time);
        break;
    case Location::Cambodia:
        drawCambodiaScene(frame, palette, time);
        break;
    }
    EndScissorMode();
}

void drawMenuBackdrop(float time)
{
    const int scene = static_cast<int>(time / MENU_SCENE_SECONDS) % 3;
    const float local = std::fmod(time, MENU_SCENE_SECONDS);
    drawLocationScene(static_cast<Location>(scene), SCREEN_AREA, time);

    // Fade through black between two locations
    float darkness = 0.0f;
    if (local < MENU_FADE_SECONDS)
    {
        darkness = 1.0f - local / MENU_FADE_SECONDS;
    }
    else if (local > MENU_SCENE_SECONDS - MENU_FADE_SECONDS)
    {
        darkness = (local - (MENU_SCENE_SECONDS - MENU_FADE_SECONDS)) / MENU_FADE_SECONDS;
    }
    // Keep the menus readable: never fully black, and a soft dark layer at all times.
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.18f + 0.6f * darkness));
}

void drawLocationEmblem(Location location, Vector2 center, float size, Color color)
{
    const float half = size / 2.0f;
    const float baseY = center.y + half * 0.75f;

    switch (location)
    {
    case Location::Egypt:
        drawTriangleAnyOrder({center.x, center.y - half * 0.85f}, {center.x - half, baseY}, {center.x + half, baseY},
                             color);
        break;
    case Location::Mexico:
        for (int i = 0; i < 4; ++i)
        {
            const float width = size * (1.0f - static_cast<float>(i) * 0.2f);
            const float height = size * 0.17f;
            DrawRectangleRec({center.x - width / 2.0f, baseY - static_cast<float>(i + 1) * height, width, height - 1.0f},
                             color);
        }
        DrawRectangleRec({center.x - size * 0.13f, baseY - size * 0.9f, size * 0.26f, size * 0.22f}, color);
        break;
    case Location::Cambodia:
        for (int i = 0; i < 3; ++i)
        {
            const float width = size * (1.0f - static_cast<float>(i) * 0.25f);
            const float height = size * 0.18f;
            DrawRectangleRec({center.x - width / 2.0f, baseY - static_cast<float>(i + 1) * height, width, height - 1.0f},
                             color);
        }
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(baseY - size * 0.68f), size * 0.13f, size * 0.16f,
                    color);
        drawTriangleAnyOrder({center.x, baseY - size * 1.0f}, {center.x - size * 0.09f, baseY - size * 0.74f},
                             {center.x + size * 0.09f, baseY - size * 0.74f}, color);
        break;
    }
}
