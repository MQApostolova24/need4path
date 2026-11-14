#include "ui/location_theme.h"

#include "ui/draw_utils.h"
#include "ui/theme.h"

#include <cmath>

namespace
{
// One palette per location, in the order of the Location enum (Egypt, Mexico, Cambodia).
const LocationPalette PALETTES[3] = {
    // Egypt - moonlit desert, sandstone and gold
    {
        {14, 16, 44, 255},    // skyTop
        {112, 58, 78, 255},   // skyBottom
        {250, 238, 206, 255}, // sun (the moon here)
        {166, 118, 64, 255},  // ground
        {214, 170, 104, 255}, // groundLight
        {222, 176, 104, 255}, // monumentLit
        {140, 96, 58, 255},   // monumentShade
        {172, 128, 76, 255},  // wall
        {212, 170, 110, 255}, // wallLight
        {110, 76, 44, 255},   // wallDark
        {62, 45, 31, 255},    // floorA
        {70, 51, 35, 255},    // floorB
        {46, 33, 22, 255},    // floorLine
        {244, 196, 64, 255},  // accent (gold)
        {168, 124, 30, 255},  // accentDark
        {64, 200, 190, 255},  // accentSoft (turquoise)
        {48, 96, 208, 255},   // gem (lapis)
        {255, 196, 120, 255}, // glow
        {236, 204, 148, 255}, // particle (sand)
    },
    // Mexico - jungle sunset, grey limestone and jade
    {
        {48, 24, 74, 255},    // skyTop
        {240, 124, 62, 255},  // skyBottom
        {255, 214, 128, 255}, // sun
        {18, 44, 30, 255},    // ground
        {40, 86, 50, 255},    // groundLight
        {176, 168, 146, 255}, // monumentLit
        {112, 108, 94, 255},  // monumentShade
        {120, 122, 108, 255}, // wall
        {162, 162, 142, 255}, // wallLight
        {76, 78, 68, 255},    // wallDark
        {42, 47, 39, 255},    // floorA
        {49, 55, 45, 255},    // floorB
        {32, 36, 29, 255},    // floorLine
        {72, 196, 136, 255},  // accent (jade)
        {32, 120, 80, 255},   // accentDark
        {214, 84, 56, 255},   // accentSoft (red ochre)
        {60, 186, 120, 255},  // gem
        {255, 184, 104, 255}, // glow
        {214, 255, 140, 255}, // particle (fireflies)
    },
    // Cambodia - monsoon dawn, red laterite and lotus pink
    {
        {46, 78, 108, 255},   // skyTop
        {246, 176, 146, 255}, // skyBottom
        {255, 230, 196, 255}, // sun
        {28, 62, 52, 255},    // ground
        {58, 104, 78, 255},   // groundLight
        {176, 122, 98, 255},  // monumentLit
        {114, 78, 66, 255},   // monumentShade
        {148, 98, 78, 255},   // wall
        {186, 134, 106, 255}, // wallLight
        {94, 62, 50, 255},    // wallDark
        {53, 44, 40, 255},    // floorA
        {61, 51, 46, 255},    // floorB
        {40, 32, 29, 255},    // floorLine
        {240, 146, 176, 255}, // accent (lotus pink)
        {170, 82, 112, 255},  // accentDark
        {232, 190, 92, 255},  // accentSoft (gold)
        {80, 96, 232, 255},   // gem (sapphire)
        {255, 208, 150, 255}, // glow
        {176, 206, 236, 255}, // particle (rain)
    },
};

const float MENU_SCENE_SECONDS = 9.0f;
const float MENU_FADE_SECONDS = 1.2f;

// ---------------------------------------------------------------- shared parts

void drawSky(const Frame& f, const LocationPalette& p)
{
    DrawRectangleGradientV(static_cast<int>(f.area.x), static_cast<int>(f.area.y), static_cast<int>(f.area.width),
                           static_cast<int>(f.area.height), p.skyTop, p.skyBottom);
}

// The sun or moon with a soft glow around it.
void drawGlowDisc(Vector2 center, float radius, Color color)
{
    DrawCircleGradient(center, radius * 3.0f, Fade(color, 0.3f), Fade(color, 0.0f));
    DrawCircleV(center, radius, color);
}

// A smooth pyramid (Egypt): a lit side, a shaded side and a gold top.
void drawSmoothPyramid(Vector2 top, float halfWidth, float baseY, const LocationPalette& p)
{
    const Vector2 left = {top.x - halfWidth, baseY};
    const Vector2 middle = {top.x + halfWidth * 0.3f, baseY};
    const Vector2 right = {top.x + halfWidth, baseY};
    drawTriangle(top, left, middle, p.monumentLit);
    drawTriangle(top, middle, right, p.monumentShade);
    for (int i = 1; i <= 8; ++i)  // rows of stone blocks
    {
        const float t = static_cast<float>(i) / 9.0f;
        const float y = top.y + (baseY - top.y) * t;
        DrawLineEx({top.x - halfWidth * t, y}, {top.x + halfWidth * t, y}, 1.0f, Fade(p.wallDark, 0.35f));
    }
    const float cap = 0.1f;  // the top 10% is gold
    drawTriangle(top, {top.x - halfWidth * cap, top.y + (baseY - top.y) * cap},
                 {top.x + halfWidth * cap, top.y + (baseY - top.y) * cap}, p.accent);
}

// A step pyramid (Mexico, Cambodia): rectangles that get narrower towards the top.
void drawStepPyramid(float centerX, float baseY, float baseWidth, float stepHeight, int steps, const LocationPalette& p)
{
    for (int i = 0; i < steps; ++i)
    {
        const float width = baseWidth * (1.0f - static_cast<float>(i) / static_cast<float>(steps + 1));
        const Rectangle step = {centerX - width / 2.0f, baseY - static_cast<float>(i + 1) * stepHeight, width, stepHeight};
        DrawRectangleRec(step, p.monumentLit);
        DrawRectangleRec({step.x + width * 0.75f, step.y, width * 0.25f, stepHeight}, p.monumentShade);  // shaded side
    }
}

// A palm tree: a slightly bent trunk with drooping leaves.
void drawPalm(Vector2 base, float height, Color color, float bend)
{
    const Vector2 top = {base.x + bend * height * 0.25f, base.y - height};
    DrawLineEx(base, {base.x + bend * height * 0.08f, base.y - height * 0.5f}, height * 0.05f, color);
    DrawLineEx({base.x + bend * height * 0.08f, base.y - height * 0.5f}, top, height * 0.04f, color);
    for (int leaf = 0; leaf < 7; ++leaf)
    {
        const float angle = -2.9f + static_cast<float>(leaf) * 0.48f;  // fanned out over the top
        const Vector2 tip = {top.x + std::cos(angle) * height * 0.42f, top.y + std::sin(angle) * height * 0.15f + height * 0.12f};
        DrawLineEx(top, tip, height * 0.03f, color);
    }
}

// A sugar palm (Cambodia): a straight trunk with a round crown of leaves.
void drawSugarPalm(Vector2 base, float height, Color color)
{
    const Vector2 top = {base.x, base.y - height};
    DrawLineEx(base, top, height * 0.035f, color);
    for (int leaf = 0; leaf < 12; ++leaf)
    {
        const float angle = static_cast<float>(leaf) * PI / 6.0f;  // 12 leaves all around
        DrawLineEx(top, {top.x + std::cos(angle) * height * 0.22f, top.y + std::sin(angle) * height * 0.16f}, height * 0.02f, color);
    }
}

// ---------------------------------------------------------------- the three scenes

void drawEgyptScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);
    for (int i = 0; i < 60; ++i)  // twinkling stars
    {
        const float twinkle = 0.5f + 0.5f * std::sin(time * 2.0f + static_cast<float>(i));
        DrawCircleV(f.at(spread(i), 0.55f * static_cast<float>(i) / 60.0f), 1.5f, Fade(p.sun, 0.2f + 0.6f * twinkle));
    }
    drawGlowDisc(f.at(0.82f, 0.2f), f.h(0.07f), p.sun);

    const float horizon = f.y(0.72f);
    drawSmoothPyramid(f.at(0.3f, 0.34f), f.w(0.23f), horizon, p);
    drawSmoothPyramid(f.at(0.6f, 0.48f), f.w(0.15f), horizon, p);
    drawSmoothPyramid(f.at(0.8f, 0.56f), f.w(0.1f), horizon, p);
    DrawRectangleRec({f.area.x, horizon, f.area.width, f.area.y + f.area.height - horizon}, p.ground);  // desert
    DrawEllipse(static_cast<int>(f.x(0.25f)), static_cast<int>(f.y(0.9f)), f.w(0.55f), f.h(0.1f), p.groundLight);
    DrawEllipse(static_cast<int>(f.x(0.85f)), static_cast<int>(f.y(0.97f)), f.w(0.5f), f.h(0.1f), ColorLerp(p.groundLight, p.ground, 0.4f));
    drawPalm(f.at(0.06f, 0.92f), f.h(0.3f), Fade(p.skyTop, 0.9f), 0.6f);
    drawPalm(f.at(0.95f, 0.98f), f.h(0.24f), Fade(p.skyTop, 0.9f), -0.5f);

    for (int i = 0; i < 30; ++i)  // sand blown by the wind
    {
        const float x = std::fmod(spread(i) + time * 0.03f, 1.0f);
        DrawCircleV(f.at(x, 0.72f + 0.26f * static_cast<float>(i) / 30.0f), 1.5f, Fade(p.particle, 0.5f));
    }
}

void drawMexicoScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);
    // The sun during an eclipse - the timer of this location
    const Vector2 sun = f.at(0.76f, 0.3f);
    drawGlowDisc(sun, f.h(0.1f), p.sun);
    DrawCircleV({sun.x + f.h(0.05f), sun.y}, f.h(0.1f), ColorLerp(p.skyTop, p.skyBottom, 0.45f));

    // Distant mountains
    for (int i = 0; i < 5; ++i)
    {
        DrawEllipse(static_cast<int>(f.x(-0.05f + 0.27f * static_cast<float>(i))), static_cast<int>(f.y(0.76f)), f.w(0.2f),
                    f.h(0.18f + 0.04f * static_cast<float>(i % 2)), ColorLerp(p.skyTop, p.monumentShade, 0.35f));
    }

    // El Castillo: a step pyramid with stairs and a temple on top
    const float baseY = f.y(0.8f);
    const float centerX = f.x(0.42f);
    const int steps = 8;
    const float stepHeight = f.h(0.04f);
    const float pyramidHeight = stepHeight * static_cast<float>(steps);
    drawStepPyramid(centerX, baseY, f.w(0.46f), stepHeight, steps, p);
    DrawRectangleRec({centerX - f.w(0.03f), baseY - pyramidHeight, f.w(0.06f), pyramidHeight}, p.monumentShade);
    const Rectangle temple = {centerX - f.w(0.05f), baseY - pyramidHeight - stepHeight * 2.0f, f.w(0.1f), stepHeight * 2.0f};
    DrawRectangleRec(temple, p.monumentLit);
    DrawRectangleRec({temple.x, temple.y, temple.width, stepHeight * 0.5f}, p.accentSoft);  // painted roof
    for (int i = -1; i <= 1; ++i)  // three dark doorways
    {
        DrawRectangleRec({centerX + static_cast<float>(i) * temple.width * 0.3f - temple.width * 0.07f,
                          temple.y + stepHeight * 0.8f, temple.width * 0.14f, stepHeight * 1.2f},
                         p.skyTop);
    }
    // Serpent heads at the bottom of the stairs
    DrawCircleV({centerX - f.w(0.04f), baseY - stepHeight * 0.4f}, stepHeight * 0.5f, p.accent);
    DrawCircleV({centerX + f.w(0.04f), baseY - stepHeight * 0.4f}, stepHeight * 0.5f, p.accent);

    // Jungle: the ground and round bushes
    DrawRectangleRec({f.area.x, baseY, f.area.width, f.area.y + f.area.height - baseY}, p.ground);
    for (int i = 0; i < 16; ++i)
    {
        DrawCircleV(f.at(static_cast<float>(i) / 15.0f, 0.86f), f.h(0.07f), i % 2 == 0 ? p.groundLight : p.ground);
    }
    drawPalm(f.at(0.86f, 0.92f), f.h(0.42f), p.ground, -0.7f);
    drawPalm(f.at(0.05f, 0.95f), f.h(0.36f), p.ground, 0.8f);

    for (int i = 0; i < 20; ++i)  // blinking fireflies
    {
        const float blink = std::fmax(0.0f, std::sin(time * 1.5f + static_cast<float>(i) * 1.7f));
        DrawCircleV(f.at(spread(i), 0.62f + 0.35f * static_cast<float>(i) / 20.0f), 2.0f, Fade(p.particle, 0.9f * blink));
    }
}

void drawCambodiaScene(const Frame& f, const LocationPalette& p, float time)
{
    drawSky(f, p);
    drawGlowDisc(f.at(0.22f, 0.4f), f.h(0.08f), p.sun);
    for (int i = 0; i < 6; ++i)  // monsoon clouds drifting slowly
    {
        const float x = std::fmod(spread(i) + time * 0.004f, 1.2f) - 0.1f;
        DrawEllipse(static_cast<int>(f.x(x)), static_cast<int>(f.y(0.08f + 0.03f * static_cast<float>(i))), f.w(0.14f), f.h(0.05f),
                    Fade(p.skyTop, 0.55f));
    }

    // Three temple towers like Angkor Wat, each with a pointed top
    const float waterLine = f.y(0.7f);
    for (int i = 0; i < 6; ++i)  // forest hills on the horizon
    {
        DrawEllipse(static_cast<int>(f.x(-0.05f + 0.22f * static_cast<float>(i))), static_cast<int>(waterLine), f.w(0.16f),
                    f.h(0.1f + 0.03f * static_cast<float>(i % 2)), ColorLerp(p.skyTop, p.ground, 0.5f));
    }
    const float towerX[3] = {0.42f, 0.6f, 0.78f};
    const float towerHeight[3] = {0.3f, 0.42f, 0.3f};
    for (int i = 0; i < 3; ++i)
    {
        const float x = f.x(towerX[i]);
        const float height = f.h(towerHeight[i]);
        drawStepPyramid(x, waterLine, f.w(0.14f), height * 0.75f / 5.0f, 5, p);
        drawTriangle({x, waterLine - height}, {x - f.w(0.035f), waterLine - height * 0.72f},
                     {x + f.w(0.035f), waterLine - height * 0.72f}, p.monumentLit);
    }

    // The moat, with lotus flowers floating on it
    DrawRectangleRec({f.area.x, waterLine, f.area.width, f.area.y + f.area.height - waterLine},
                     ColorLerp(p.skyTop, p.ground, 0.35f));
    for (int i = 0; i < 20; ++i)  // sunlight sparkling on the water
    {
        const float x = std::fmod(spread(i) + time * 0.005f, 1.0f);
        const Vector2 at = f.at(x, 0.72f + 0.26f * static_cast<float>(i) / 20.0f);
        DrawLineEx(at, {at.x + f.w(0.04f), at.y}, 1.5f, Fade(p.sun, 0.25f));
    }
    for (int i = 0; i < 6; ++i)  // lotus flowers on their leaves
    {
        const Vector2 lotus = f.at(0.08f + 0.17f * static_cast<float>(i), 0.9f);
        DrawEllipse(static_cast<int>(lotus.x), static_cast<int>(lotus.y + 3.0f), f.w(0.025f), f.h(0.012f), p.groundLight);
        DrawCircleV(lotus, f.h(0.012f), p.accent);
    }
    drawSugarPalm(f.at(0.1f, 0.74f), f.h(0.4f), p.ground);
    drawSugarPalm(f.at(0.96f, 0.76f), f.h(0.32f), p.ground);

    for (int i = 0; i < 50; ++i)  // monsoon rain falling down
    {
        const float y = std::fmod(spread(i * 7) + time * 0.6f, 1.0f);
        const Vector2 top = f.at(spread(i), y);
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

    // Nothing is drawn outside the area (the scene may sit inside a small card).
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
    // Every 9 seconds the next location is shown.
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
    case Location::Egypt:  // a smooth pyramid
        drawTriangle({center.x, center.y - half * 0.85f}, {center.x - half, baseY}, {center.x + half, baseY}, color);
        break;
    case Location::Mexico:  // a step pyramid with a temple on top
        for (int i = 0; i < 4; ++i)
        {
            const float width = size * (1.0f - static_cast<float>(i) * 0.2f);
            const float height = size * 0.17f;
            DrawRectangleRec({center.x - width / 2.0f, baseY - static_cast<float>(i + 1) * height, width, height - 1.0f}, color);
        }
        DrawRectangleRec({center.x - size * 0.13f, baseY - size * 0.9f, size * 0.26f, size * 0.22f}, color);
        break;
    case Location::Cambodia:  // a temple tower with a pointed top
        for (int i = 0; i < 3; ++i)
        {
            const float width = size * (1.0f - static_cast<float>(i) * 0.25f);
            const float height = size * 0.18f;
            DrawRectangleRec({center.x - width / 2.0f, baseY - static_cast<float>(i + 1) * height, width, height - 1.0f}, color);
        }
        drawTriangle({center.x, baseY - size * 1.0f}, {center.x - size * 0.13f, baseY - size * 0.54f},
                     {center.x + size * 0.13f, baseY - size * 0.54f}, color);
        break;
    }
}
