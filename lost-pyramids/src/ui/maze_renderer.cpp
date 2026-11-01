#include "ui/maze_renderer.h"

#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/sprites.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
const float MAX_DARKNESS = 0.8f;            // Medium: the layout stays visible in the dark
const float FLASHLIGHT_MAX_DARKNESS = 0.88f; // Hard: only a faint outline of the walls (requirement 6)
const float URGENT_TIME_FRACTION = 0.35f;
const int PARTICLE_COUNT = 40;
const Color WATER = {70, 150, 220, 255};
const Color ECLIPSE_SHADOW = {20, 8, 30, 255};

Rectangle getCellRect(const MazeView& view, int row, int col)
{
    return {view.originX + static_cast<float>(col) * view.tileSize,
            view.originY + static_cast<float>(row) * view.tileSize, view.tileSize, view.tileSize};
}

Vector2 getRectCenter(Rectangle rect)
{
    return {rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f};
}

bool isWallLike(const Maze& maze, int row, int col)
{
    if (!maze.isInside(row, col))
    {
        return false;
    }
    const TileType tile = maze.at(row, col).tile;
    return tile == TileType::Wall || tile == TileType::SecretDoor;
}

// ---------------------------------------------------------------- tiles

void drawWallDetails(Location location, Rectangle top, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    const float roll = hash01(row, col, 11);
    const float size = top.width;

    switch (location)
    {
    case Location::Egypt:
        if (roll > 0.88f)
        {
            // A tiny carved hieroglyph
            const Vector2 c = {top.x + size * 0.5f, top.y + top.height * 0.55f};
            DrawRing({c.x, c.y - size * 0.1f}, size * 0.05f, size * 0.09f, 0.0f, 360.0f, 10, p.wallDetail);
            DrawLineEx({c.x, c.y - size * 0.02f}, {c.x, c.y + size * 0.2f}, std::fmax(1.0f, size * 0.05f), p.wallDetail);
        }
        break;
    case Location::Mexico:
        if (roll < 0.3f)
        {
            // Moss growing on the limestone
            const float mx = top.x + size * (0.25f + 0.5f * hash01(row, col, 12));
            DrawEllipse(static_cast<int>(mx), static_cast<int>(top.y + top.height * 0.3f), size * 0.22f, size * 0.12f,
                        Fade(p.wallDetail, 0.75f));
        }
        else if (roll > 0.9f)
        {
            // Step-fret carving
            const float x0 = top.x + size * 0.2f;
            const float y0 = top.y + top.height * 0.75f;
            const float s = size * 0.15f;
            const float t = std::fmax(1.0f, size * 0.06f);
            DrawLineEx({x0, y0}, {x0 + s, y0}, t, p.wallDark);
            DrawLineEx({x0 + s, y0}, {x0 + s, y0 - s}, t, p.wallDark);
            DrawLineEx({x0 + s, y0 - s}, {x0 + 2.0f * s, y0 - s}, t, p.wallDark);
            DrawLineEx({x0 + 2.0f * s, y0 - s}, {x0 + 2.0f * s, y0 - 2.0f * s}, t, p.wallDark);
            DrawLineEx({x0 + 2.0f * s, y0 - 2.0f * s}, {x0 + 3.0f * s, y0 - 2.0f * s}, t, p.wallDark);
        }
        break;
    case Location::Cambodia:
        // Laterite stone is full of small holes
        for (int i = 0; i < 3; ++i)
        {
            DrawCircleV({top.x + size * (0.15f + 0.7f * hash01(row, col, 20 + i)),
                         top.y + top.height * (0.25f + 0.6f * hash01(row, col, 30 + i))},
                        std::fmax(1.0f, size * 0.04f), Fade(p.wallDark, 0.6f));
        }
        if (roll < 0.22f)
        {
            DrawEllipse(static_cast<int>(top.x + size * 0.7f), static_cast<int>(top.y + top.height * 0.25f), size * 0.18f,
                        size * 0.1f, Fade(p.wallDetail, 0.7f));
        }
        break;
    }
}

void drawFloorDetails(Location location, Rectangle rect, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    const float roll = hash01(row, col, 41);
    const float size = rect.width;

    switch (location)
    {
    case Location::Egypt:
        if (roll < 0.5f)
        {
            DrawCircleV({rect.x + size * hash01(row, col, 42), rect.y + size * hash01(row, col, 43)},
                        std::fmax(1.0f, size * 0.04f), Fade(p.particle, 0.25f));
        }
        break;
    case Location::Mexico:
        if (roll < 0.18f)
        {
            DrawEllipse(static_cast<int>(rect.x + size * (0.2f + 0.6f * hash01(row, col, 44))),
                        static_cast<int>(rect.y + size * (0.3f + 0.5f * hash01(row, col, 45))), size * 0.12f, size * 0.06f,
                        Fade(p.wallDetail, 0.55f));
        }
        break;
    case Location::Cambodia:
        if (roll < 0.12f)
        {
            DrawEllipse(static_cast<int>(rect.x + size * 0.5f), static_cast<int>(rect.y + size * 0.6f), size * 0.32f,
                        size * 0.16f, Fade(WATER, 0.22f));
        }
        break;
    }

    if (roll > 0.92f)
    {
        // A crack in the slab
        DrawLineEx({rect.x + size * 0.2f, rect.y + size * 0.3f}, {rect.x + size * 0.55f, rect.y + size * 0.5f}, 1.0f,
                   p.floorLine);
        DrawLineEx({rect.x + size * 0.55f, rect.y + size * 0.5f}, {rect.x + size * 0.7f, rect.y + size * 0.8f}, 1.0f,
                   p.floorLine);
    }
}

// Soft shadows thrown by the walls above and to the left of a floor tile.
void drawFloorShadows(const Maze& maze, Rectangle rect, int row, int col)
{
    if (isWallLike(maze, row - 1, col))
    {
        DrawRectangleGradientV(static_cast<int>(rect.x), static_cast<int>(rect.y), static_cast<int>(rect.width),
                               static_cast<int>(rect.height * 0.4f), Fade(BLACK, 0.45f), Fade(BLACK, 0.0f));
    }
    if (isWallLike(maze, row, col - 1))
    {
        DrawRectangleGradientH(static_cast<int>(rect.x), static_cast<int>(rect.y), static_cast<int>(rect.width * 0.25f),
                               static_cast<int>(rect.height), Fade(BLACK, 0.3f), Fade(BLACK, 0.0f));
    }
}

// ---------------------------------------------------------------- items and creatures

void drawItems(const GameState& state, const MazeView& view, float time)
{
    const Maze& maze = state.maze;
    const Location location = state.location;
    const bool flashlightOnly = getDifficultySettings(state.difficulty).light == LightMode::Flashlight;

    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            const Cell& cell = maze.at(row, col);
            if (cell.item == ItemType::None)
            {
                continue;
            }

            // Easy/Medium: rewards are remembered once seen and live traps only show in the light.
            // Hard: nothing is remembered - only what the flashlight shows right now.
            const bool lit = isCellLit(state, row, col);
            const bool remembered = lit || (cell.explored && !flashlightOnly);
            const Vector2 center = getRectCenter(getCellRect(view, row, col));
            const float size = view.tileSize;

            switch (cell.item)
            {
            case ItemType::Key:
                if (remembered)
                {
                    drawKeySprite(location, center, size * 0.8f);
                }
                break;
            case ItemType::Treasure:
                if (remembered)
                {
                    drawTreasureSprite(location, center, size, time);
                }
                break;
            case ItemType::Gem:
                if (remembered)
                {
                    drawGemSprite(location, center, size, time);
                }
                break;
            case ItemType::Relic:
                if (remembered)
                {
                    drawRelicSprite(location, center, size, time);
                }
                break;
            case ItemType::TimePickup:
                if (remembered)
                {
                    drawTimePickupSprite(location, center, size, time);
                }
                break;
            case ItemType::Trap:
                if (lit)
                {
                    drawTrapSprite(location, center, size, false);
                }
                break;
            case ItemType::SprungTrap:
                if (remembered)
                {
                    drawTrapSprite(location, center, size, true);
                }
                break;
            case ItemType::None:
                break;
            }
        }
    }
}

// ---------------------------------------------------------------- light and atmosphere

// Darkens one tile with a smooth blend between the darkness at its four corners.
// The tile is split into small squares, each with its own shade (bilinear blend).
// This works the same on every graphics driver.
void drawDarkness(Rectangle rect, float topLeft, float bottomLeft, float bottomRight, float topRight)
{
    const int parts = rect.width >= 24.0f ? 3 : 2;
    const float step = rect.width / static_cast<float>(parts);
    for (int i = 0; i < parts; ++i)
    {
        for (int j = 0; j < parts; ++j)
        {
            const float u = (static_cast<float>(j) + 0.5f) / static_cast<float>(parts);  // left -> right
            const float v = (static_cast<float>(i) + 0.5f) / static_cast<float>(parts);  // top -> bottom
            const float top = topLeft + (topRight - topLeft) * u;
            const float bottom = bottomLeft + (bottomRight - bottomLeft) * u;
            const float darkness = top + (bottom - top) * v;
            if (darkness > 0.01f)
            {
                DrawRectangleRec({rect.x + step * static_cast<float>(j), rect.y + step * static_cast<float>(i), step + 0.5f,
                                  step + 0.5f},
                                 Fade(BLACK, darkness));
            }
        }
    }
}

float darknessAt(float row, float col, const GridPos& player, float radius)
{
    const float rowOffset = row - (static_cast<float>(player.row) + 0.5f);
    const float colOffset = col - (static_cast<float>(player.col) + 0.5f);
    const float distance = std::sqrt(rowOffset * rowOffset + colOffset * colOffset);
    const float inner = radius * 0.45f;
    const float outer = radius + 0.6f;
    const float t = std::clamp((distance - inner) / (outer - inner), 0.0f, 1.0f);
    return MAX_DARKNESS * t * t * (3.0f - 2.0f * t);  // smoothstep
}

// The darkness is worked out at the corners of every tile and blended across the tile,
// so the edge of the torch light is a smooth circle instead of square steps.
void drawTorchLight(const GameState& state, const MazeView& view, float time, float baseRadius)
{
    const int size = state.maze.getSize();
    const float radius = baseRadius + 0.15f * std::sin(time * 9.0f) + 0.08f * std::sin(time * 23.0f);

    std::vector<float> corners(static_cast<std::size_t>((size + 1) * (size + 1)));
    for (int row = 0; row <= size; ++row)
    {
        for (int col = 0; col <= size; ++col)
        {
            corners[static_cast<std::size_t>(row * (size + 1) + col)] =
                darknessAt(static_cast<float>(row), static_cast<float>(col), state.player, radius);
        }
    }

    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const float topLeft = corners[static_cast<std::size_t>(row * (size + 1) + col)];
            const float topRight = corners[static_cast<std::size_t>(row * (size + 1) + col + 1)];
            const float bottomLeft = corners[static_cast<std::size_t>((row + 1) * (size + 1) + col)];
            const float bottomRight = corners[static_cast<std::size_t>((row + 1) * (size + 1) + col + 1)];
            if (topLeft + topRight + bottomLeft + bottomRight < 0.02f)
            {
                continue;
            }
            drawDarkness(getCellRect(view, row, col), topLeft, bottomLeft, bottomRight, topRight);
        }
    }

    // Warm glow of the torch
    const Vector2 center = getRectCenter(getCellRect(view, state.player.row, state.player.col));
    const Color glow = getPalette(state.location).glow;
    DrawCircleGradient(center, radius * view.tileSize * 1.1f, Fade(glow, 0.16f), Fade(glow, 0.0f));
}

// Hard: the flashlight is the only real light. The light of every tile is blended at the
// corners so the beam has soft edges. Outside the beam the pyramid is very dark, but a faint
// outline of the walls stays visible, so the player always sees the layout (requirement 6).
void drawFlashlightDarkness(const GameState& state, const MazeView& view)
{
    const int size = state.maze.getSize();
    std::vector<float> light(static_cast<std::size_t>(size * size));
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            light[static_cast<std::size_t>(row * size + col)] = getLightLevel(state, row, col);
        }
    }

    // Light at a tile corner = average of the (up to) four tiles that share that corner.
    auto cornerDarkness = [&](int row, int col) {
        float sum = 0.0f;
        for (int r = row - 1; r <= row; ++r)
        {
            for (int c = col - 1; c <= col; ++c)
            {
                if (r >= 0 && r < size && c >= 0 && c < size)
                {
                    sum += light[static_cast<std::size_t>(r * size + c)];
                }
            }
        }
        return FLASHLIGHT_MAX_DARKNESS * (1.0f - std::clamp(sum / 4.0f * 1.6f, 0.0f, 1.0f));
    };

    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const float topLeft = cornerDarkness(row, col);
            const float topRight = cornerDarkness(row, col + 1);
            const float bottomLeft = cornerDarkness(row + 1, col);
            const float bottomRight = cornerDarkness(row + 1, col + 1);
            const Rectangle rect = getCellRect(view, row, col);
            if (topLeft + topRight + bottomLeft + bottomRight > 0.02f)
            {
                drawDarkness(rect, topLeft, bottomLeft, bottomRight, topRight);
            }
        }
    }
}

// Sand motes, fireflies or rain drifting over the pyramid.
void drawParticles(Location location, Rectangle area, float time)
{
    const Color color = getPalette(location).particle;
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        const float a = hash01(i, 1, 51);
        const float b = hash01(i, 2, 51);
        switch (location)
        {
        case Location::Egypt:
        {
            const float fx = std::fmod(a + time * (0.01f + 0.02f * b), 1.0f);
            const float fy = std::fmod(b + 0.02f * std::sin(time + static_cast<float>(i)) + 1.0f, 1.0f);
            DrawCircleV({area.x + fx * area.width, area.y + fy * area.height}, 1.5f, Fade(color, 0.3f));
            break;
        }
        case Location::Mexico:
        {
            const float blink = std::fmax(0.0f, std::sin(time * (1.2f + b) + static_cast<float>(i) * 2.1f));
            const Vector2 pos = {area.x + (a + 0.02f * std::sin(time * 0.6f + static_cast<float>(i))) * area.width,
                                 area.y + (b + 0.02f * std::cos(time * 0.8f + static_cast<float>(i))) * area.height};
            DrawCircleV(pos, 5.0f, Fade(color, 0.12f * blink));
            DrawCircleV(pos, 1.8f, Fade(color, 0.7f * blink));
            break;
        }
        case Location::Cambodia:
        {
            const float fall = std::fmod(b + time * (0.6f + 0.3f * a), 1.0f);
            const Vector2 top = {area.x + a * area.width - fall * 20.0f, area.y + fall * area.height};
            DrawLineEx(top, {top.x - 3.0f, top.y + 14.0f}, 1.0f, Fade(color, 0.3f));
            break;
        }
        }
    }
}

// As the timer runs low, the location itself closes in on the player.
void drawTimerPressure(const GameState& state, Rectangle area, float time)
{
    const float fraction = getTimeFraction(state);
    const float urgency = std::clamp((URGENT_TIME_FRACTION - fraction) / URGENT_TIME_FRACTION, 0.0f, 1.0f);
    if (urgency <= 0.0f)
    {
        return;
    }

    const LocationPalette& p = getPalette(state.location);
    const int x = static_cast<int>(area.x);
    const int y = static_cast<int>(area.y);
    const int width = static_cast<int>(area.width);
    const int height = static_cast<int>(area.height);

    switch (state.location)
    {
    case Location::Egypt:
    {
        // Sand pours in from the edges
        const int band = static_cast<int>(area.width * 0.14f * urgency);
        const Color sand = Fade(p.groundLight, 0.6f * urgency);
        DrawRectangleGradientV(x, y, width, band, sand, Fade(sand, 0.0f));
        DrawRectangleGradientV(x, y + height - band, width, band, Fade(sand, 0.0f), sand);
        DrawRectangleGradientH(x, y, band, height, sand, Fade(sand, 0.0f));
        DrawRectangleGradientH(x + width - band, y, band, height, Fade(sand, 0.0f), sand);
        for (int i = 0; i < 8; ++i)
        {
            const float sx = area.x + area.width * hash01(i, 3, 61);
            const float length = area.height * (0.1f + 0.2f * urgency) * (0.6f + 0.4f * std::sin(time * 3.0f + static_cast<float>(i)));
            DrawLineEx({sx, area.y}, {sx, area.y + length}, 2.0f, Fade(p.groundLight, 0.5f * urgency));
        }
        break;
    }
    case Location::Mexico:
        // The eclipse darkens the whole temple
        DrawRectangleRec(area, Fade(ECLIPSE_SHADOW, 0.45f * urgency));
        DrawRectangleLinesEx(area, 6.0f, Fade(p.accentSoft, 0.4f * urgency));
        break;
    case Location::Cambodia:
    {
        // Flood water rises from the bottom
        const float level = area.height * 0.45f * urgency;
        const float top = area.y + area.height - level;
        DrawRectangleRec({area.x, top, area.width, level}, Fade(WATER, 0.3f));
        for (float wx = area.x; wx < area.x + area.width; wx += 12.0f)
        {
            const float wave = std::sin(time * 3.0f + wx * 0.05f) * 3.0f;
            DrawCircleV({wx, top + wave}, 4.0f, Fade(WATER, 0.35f));
        }
        break;
    }
    }
}
}  // namespace

MazeView computeMazeView(int mazeSize, Rectangle area)
{
    MazeView view;
    const float side = std::fmin(area.width, area.height);
    view.tileSize = std::floor(side / static_cast<float>(mazeSize));
    const float total = view.tileSize * static_cast<float>(mazeSize);
    view.originX = std::floor(area.x + (area.width - total) / 2.0f);
    view.originY = std::floor(area.y + (area.height - total) / 2.0f);
    return view;
}

void drawWallTile(Location location, Rectangle rect, int row, int col, bool showFront)
{
    const LocationPalette& p = getPalette(location);
    const float frontHeight = showFront ? std::floor(rect.height * 0.28f) : 0.0f;
    const Rectangle top = {rect.x, rect.y, rect.width, rect.height - frontHeight};

    // Top surface of the block
    DrawRectangleRec(top, p.wall);
    DrawRectangleRec({top.x, top.y, top.width, std::fmax(1.0f, std::floor(top.height * 0.14f))}, p.wallLight);

    if (rect.width >= 12.0f)
    {
        // Mortar seams, shifted on every other block
        const bool even = (row + col) % 2 == 0;
        const float middleY = top.y + top.height * 0.55f;
        const float seamTop = top.x + top.width * (even ? 0.3f : 0.7f);
        const float seamBottom = top.x + top.width * (even ? 0.7f : 0.3f);
        DrawLineEx({top.x, middleY}, {top.x + top.width, middleY}, 1.0f, p.wallDark);
        DrawLineEx({seamTop, top.y + top.height * 0.14f}, {seamTop, middleY}, 1.0f, p.wallDark);
        DrawLineEx({seamBottom, middleY}, {seamBottom, top.y + top.height}, 1.0f, p.wallDark);
        drawWallDetails(location, top, row, col);
    }

    // Front face of the block (only visible when the tile below is open)
    if (showFront)
    {
        DrawRectangleGradientV(static_cast<int>(rect.x), static_cast<int>(top.y + top.height), static_cast<int>(rect.width),
                               static_cast<int>(frontHeight) + 1, p.wallDark, mixColor(p.wallDark, BLACK, 0.35f));
    }
}

void drawFloorTile(Location location, Rectangle rect, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    DrawRectangleRec(rect, (row + col) % 2 == 0 ? p.floorA : p.floorB);
    DrawLineEx({rect.x, rect.y + rect.height - 0.5f}, {rect.x + rect.width, rect.y + rect.height - 0.5f}, 1.0f, p.floorLine);
    DrawLineEx({rect.x + rect.width - 0.5f, rect.y}, {rect.x + rect.width - 0.5f, rect.y + rect.height}, 1.0f, p.floorLine);
    if (rect.width >= 12.0f)
    {
        drawFloorDetails(location, rect, row, col);
    }
}

void drawMazeScene(const GameState& state, const MazeView& view, float time, int skinId)
{
    const Maze& maze = state.maze;
    const int size = maze.getSize();
    const float total = view.tileSize * static_cast<float>(size);
    const Rectangle area = {view.originX, view.originY, total, total};
    const Location location = state.location;
    const LocationPalette& p = getPalette(location);
    const DifficultySettings& settings = getDifficultySettings(state.difficulty);
    const bool flashlightOnly = settings.light == LightMode::Flashlight;
    const GridPos exit = maze.getExit();

    // Frame around the pyramid
    DrawRectangleRec({area.x - 10.0f, area.y - 10.0f, total + 20.0f, total + 20.0f}, p.wallDark);
    DrawRectangleLinesEx({area.x - 6.0f, area.y - 6.0f, total + 12.0f, total + 12.0f}, 2.0f, p.accent);

    // 1. Stone tiles
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const Rectangle rect = getCellRect(view, row, col);
            const bool frontVisible = !isWallLike(maze, row + 1, col);
            switch (maze.at(row, col).tile)
            {
            case TileType::Wall:
                drawWallTile(location, rect, row, col, frontVisible);
                break;
            case TileType::SecretDoor:
                drawWallTile(location, rect, row, col, frontVisible);
                drawDoorSprite(location, rect, time);
                break;
            case TileType::OpenedDoor:
                drawFloorTile(location, rect, row, col);
                DrawRectangleLinesEx(rect, std::fmax(1.0f, view.tileSize / 10.0f), Fade(p.accent, 0.6f));
                break;
            case TileType::Floor:
            case TileType::Exit:
                drawFloorTile(location, rect, row, col);
                drawFloorShadows(maze, rect, row, col);
                break;
            }
        }
    }

    // 2. Rewards, traps and the guardian
    drawItems(state, view, time);
    if (state.guardian.active && isCellLit(state, state.guardian.pos.row, state.guardian.pos.col))
    {
        drawGuardianSprite(location, getRectCenter(getCellRect(view, state.guardian.pos.row, state.guardian.pos.col)),
                           view.tileSize, time);
    }

    // 3. Weather, timer effect and light
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(total), static_cast<int>(total));
    drawParticles(location, area, time);
    switch (settings.light)
    {
    case LightMode::FullLight:
        drawTimerPressure(state, area, time);
        break;
    case LightMode::Torch:
        drawTorchLight(state, view, time, settings.lightRange);
        drawTimerPressure(state, area, time);
        break;
    case LightMode::Flashlight:
        drawTimerPressure(state, area, time);  // drawn first, so it is hidden in the dark too
        drawFlashlightDarkness(state, view);
        break;
    }
    EndScissorMode();

    // 4. The exit gate always shines through the dark (requirement 6)
    drawExitSprite(location, getCellRect(view, exit.row, exit.col), time);

    // 5. The explorer (blinks while protected after losing a life)
    const bool blinking = state.invulnerableSeconds > 0.0f && std::fmod(time, 0.2f) < 0.1f;
    if (!blinking)
    {
        drawExplorerSprite(getRectCenter(getCellRect(view, state.player.row, state.player.col)), view.tileSize, time,
                           skinId, state.facing, flashlightOnly);
    }
    if (state.invulnerableSeconds > 0.0f)
    {
        DrawRectangleRec(area, Fade(theme::HEART, 0.18f * state.invulnerableSeconds));
    }
}

namespace
{
// The character shown for one tile in the text view.
char textSymbol(const GameState& state, int row, int col, bool flashlightOnly)
{
    const GridPos pos{row, col};
    if (pos == state.player)
    {
        return 'P';
    }
    if (state.guardian.active && pos == state.guardian.pos && isCellLit(state, row, col))
    {
        return 'G';
    }

    const Cell& cell = state.maze.at(row, col);
    const bool lit = isCellLit(state, row, col);
    const bool remembered = lit || (cell.explored && !flashlightOnly);
    switch (cell.item)
    {
    case ItemType::Key:
        return remembered ? 'k' : '.';
    case ItemType::Treasure:
        return remembered ? '$' : '.';
    case ItemType::Gem:
        return remembered ? '*' : '.';
    case ItemType::Relic:
        return remembered ? 'R' : '.';
    case ItemType::TimePickup:
        return remembered ? '+' : '.';
    case ItemType::Trap:
        return lit ? 'x' : '.';
    case ItemType::SprungTrap:
        return remembered ? 'o' : '.';
    case ItemType::None:
        break;
    }

    switch (cell.tile)
    {
    case TileType::Wall:
        return '#';
    case TileType::SecretDoor:
        return 'D';
    case TileType::OpenedDoor:
        return '/';
    case TileType::Exit:
        return 'E';
    case TileType::Floor:
        break;
    }
    return '.';
}

Color textSymbolColor(char symbol, const LocationPalette& p)
{
    switch (symbol)
    {
    case '#':
        return p.wallLight;
    case '.':
        return Fade(p.wallLight, 0.3f);
    case 'P':
        return WHITE;
    case 'E':
    case 'R':
        return theme::GOLD_BRIGHT;
    case 'G':
    case 'x':
        return theme::DANGER;
    case '+':
        return theme::SUCCESS;
    default:
        return p.accent;
    }
}
}  // namespace

void drawTextMaze(const GameState& state, const MazeView& view)
{
    const Maze& maze = state.maze;
    const int size = maze.getSize();
    const float total = view.tileSize * static_cast<float>(size);
    const LocationPalette& p = getPalette(state.location);
    const bool flashlightOnly = getDifficultySettings(state.difficulty).light == LightMode::Flashlight;

    DrawRectangleRec({view.originX - 10.0f, view.originY - 10.0f, total + 20.0f, total + 20.0f}, {12, 10, 9, 255});
    DrawRectangleLinesEx({view.originX - 6.0f, view.originY - 6.0f, total + 12.0f, total + 12.0f}, 2.0f, p.accent);

    const int fontSize = std::max(8, static_cast<int>(view.tileSize * 0.9f));
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const char symbol = textSymbol(state, row, col, flashlightOnly);
            const char text[2] = {symbol, '\0'};
            const Rectangle rect = getCellRect(view, row, col);
            const int width = MeasureText(text, fontSize);
            DrawText(text, static_cast<int>(rect.x + (rect.width - static_cast<float>(width)) / 2.0f),
                     static_cast<int>(rect.y + (rect.height - static_cast<float>(fontSize)) / 2.0f), fontSize,
                     textSymbolColor(symbol, p));
        }
    }
}
