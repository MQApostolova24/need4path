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
const float TORCH_MAX_DARKNESS = 0.8f;       // Medium: the layout stays visible in the dark
const float FLASHLIGHT_MAX_DARKNESS = 0.88f; // Hard: only a faint outline of the walls (requirement 6)
const float URGENT_TIME_FRACTION = 0.35f;    // the timer effect starts at 35% time left
const Color WATER = {70, 150, 220, 255};
const Color ECLIPSE_SHADOW = {20, 8, 30, 255};
const int PARTICLE_COUNT = 40;

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

// ---------------------------------------------------------------- tile decorations

// Small location details on some wall blocks. tilePattern gives every tile a fixed
// number 0..9, so the same blocks always get the same decoration (no flickering).
void drawWallDetails(Location location, Rectangle top, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    const int pattern = tilePattern(row, col);
    const float size = top.width;
    const Vector2 c = {top.x + size * 0.5f, top.y + top.height * 0.55f};

    switch (location)
    {
    case Location::Egypt:
        if (pattern == 0)  // a carved hieroglyph (a small ankh)
        {
            DrawRing({c.x, c.y - size * 0.1f}, size * 0.05f, size * 0.09f, 0.0f, 360.0f, 10, p.wallDark);
            DrawLineEx({c.x, c.y - size * 0.02f}, {c.x, c.y + size * 0.2f}, std::fmax(1.0f, size * 0.05f), p.wallDark);
        }
        break;
    case Location::Mexico:
        if (pattern < 3)  // moss growing on the limestone
        {
            DrawEllipse(static_cast<int>(c.x), static_cast<int>(top.y + top.height * 0.3f), size * 0.22f, size * 0.12f,
                        Fade(p.groundLight, 0.8f));
        }
        break;
    case Location::Cambodia:
        // laterite stone is full of small holes
        DrawCircleV({top.x + size * 0.25f, top.y + top.height * 0.35f}, std::fmax(1.0f, size * 0.04f), Fade(p.wallDark, 0.6f));
        DrawCircleV({top.x + size * (0.5f + 0.04f * static_cast<float>(pattern)), top.y + top.height * 0.75f},
                    std::fmax(1.0f, size * 0.04f), Fade(p.wallDark, 0.6f));
        break;
    }
}

// A crack, a grain of sand, a leaf or a puddle on some floor slabs.
void drawFloorDetails(Location location, Rectangle rect, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    const int pattern = tilePattern(row, col);
    const float size = rect.width;

    if (pattern == 9)  // a crack in the slab
    {
        DrawLineEx({rect.x + size * 0.2f, rect.y + size * 0.3f}, {rect.x + size * 0.55f, rect.y + size * 0.5f}, 1.0f, p.floorLine);
        DrawLineEx({rect.x + size * 0.55f, rect.y + size * 0.5f}, {rect.x + size * 0.7f, rect.y + size * 0.8f}, 1.0f, p.floorLine);
    }
    else if (pattern == 4)
    {
        const Vector2 c = {rect.x + size * 0.5f, rect.y + size * 0.6f};
        switch (location)
        {
        case Location::Egypt:
            DrawCircleV(c, std::fmax(1.0f, size * 0.05f), Fade(p.particle, 0.3f));
            break;
        case Location::Mexico:
            DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y), size * 0.12f, size * 0.06f, Fade(p.groundLight, 0.6f));
            break;
        case Location::Cambodia:
            DrawEllipse(static_cast<int>(c.x), static_cast<int>(c.y), size * 0.32f, size * 0.16f, Fade(WATER, 0.22f));
            break;
        }
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

// ---------------------------------------------------------------- light, weather and the timer

// Covers the pyramid with darkness: the less light a tile gets, the darker it is.
// To make the edge of the light soft, every tile is split into 4 small squares and each
// square mixes the tile's own light with the light of the 3 tiles next to that corner.
void drawDarkness(const GameState& state, const MazeView& view, float maxDarkness)
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
    // Light of a tile, or of the nearest tile inside the pyramid at the border.
    auto lightAt = [&](int row, int col) {
        row = std::clamp(row, 0, size - 1);
        col = std::clamp(col, 0, size - 1);
        return light[static_cast<std::size_t>(row * size + col)];
    };

    const float half = view.tileSize / 2.0f;
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const Rectangle rect = getCellRect(view, row, col);
            for (int dy = -1; dy <= 1; dy += 2)  // top half, bottom half
            {
                for (int dx = -1; dx <= 1; dx += 2)  // left half, right half
                {
                    const float corner = (lightAt(row, col) + lightAt(row + dy, col) + lightAt(row, col + dx) +
                                          lightAt(row + dy, col + dx)) / 4.0f;
                    const float mixed = (lightAt(row, col) + corner) / 2.0f;
                    const float darkness = maxDarkness * (1.0f - mixed);
                    if (darkness > 0.01f)
                    {
                        const float x = dx < 0 ? rect.x : rect.x + half;
                        const float y = dy < 0 ? rect.y : rect.y + half;
                        DrawRectangleRec({x, y, half + 0.5f, half + 0.5f}, Fade(BLACK, darkness));
                    }
                }
            }
        }
    }
}

// Sand blowing (Egypt), fireflies (Mexico) or rain (Cambodia) over the pyramid.
void drawParticles(Location location, Rectangle area, float time)
{
    const Color color = getPalette(location).particle;
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        const float x = spread(i);                                          // 0..1 across
        const float y = static_cast<float>(i) / static_cast<float>(PARTICLE_COUNT);  // 0..1 down
        switch (location)
        {
        case Location::Egypt:
        {
            const float moved = std::fmod(x + time * 0.02f, 1.0f);  // drifts to the right
            DrawCircleV({area.x + moved * area.width, area.y + y * area.height}, 1.5f, Fade(color, 0.35f));
            break;
        }
        case Location::Mexico:
        {
            const float blink = std::fmax(0.0f, std::sin(time * 1.5f + static_cast<float>(i) * 2.1f));
            const Vector2 pos = {area.x + x * area.width, area.y + y * area.height};
            DrawCircleV(pos, 5.0f, Fade(color, 0.12f * blink));
            DrawCircleV(pos, 1.8f, Fade(color, 0.7f * blink));
            break;
        }
        case Location::Cambodia:
        {
            const float fall = std::fmod(y + time * 0.7f, 1.0f);  // falls down and starts again at the top
            const Vector2 top = {area.x + x * area.width, area.y + fall * area.height};
            DrawLineEx(top, {top.x - 3.0f, top.y + 14.0f}, 1.0f, Fade(color, 0.3f));
            break;
        }
        }
    }
}

// When the time runs low, the location closes in on the player.
void drawTimerPressure(const GameState& state, Rectangle area, float time)
{
    const float fraction = getTimeFraction(state);
    if (fraction >= URGENT_TIME_FRACTION)
    {
        return;
    }
    const float urgency = (URGENT_TIME_FRACTION - fraction) / URGENT_TIME_FRACTION;  // 0 -> 1
    const LocationPalette& p = getPalette(state.location);

    switch (state.location)
    {
    case Location::Egypt:  // sand pours in from the edges
        DrawRectangleLinesEx(area, area.width * 0.12f * urgency, Fade(p.groundLight, 0.55f * urgency));
        break;
    case Location::Mexico:  // the eclipse darkens the whole temple
        DrawRectangleRec(area, Fade(ECLIPSE_SHADOW, 0.45f * urgency));
        DrawRectangleLinesEx(area, 6.0f, Fade(p.accentSoft, 0.4f * urgency));
        break;
    case Location::Cambodia:  // flood water rises from the bottom, with waves on top
    {
        const float level = area.height * 0.45f * urgency;
        const float top = area.y + area.height - level;
        DrawRectangleRec({area.x, top, area.width, level}, Fade(WATER, 0.3f));
        for (float x = area.x; x < area.x + area.width; x += 12.0f)
        {
            DrawCircleV({x, top + std::sin(time * 3.0f + x * 0.05f) * 3.0f}, 4.0f, Fade(WATER, 0.35f));
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

    // Top of the stone block, with a light edge
    DrawRectangleRec(top, p.wall);
    DrawRectangleRec({top.x, top.y, top.width, std::fmax(1.0f, top.height * 0.14f)}, p.wallLight);

    // Mortar lines: one across the middle, one seam that moves on every other block
    if (rect.width >= 12.0f)
    {
        const float middleY = top.y + top.height * 0.55f;
        const float seamX = top.x + top.width * ((row + col) % 2 == 0 ? 0.3f : 0.7f);
        DrawLineEx({top.x, middleY}, {top.x + top.width, middleY}, 1.0f, p.wallDark);
        DrawLineEx({seamX, middleY}, {seamX, top.y + top.height}, 1.0f, p.wallDark);
        drawWallDetails(location, top, row, col);
    }

    // Front face of the block (only when the tile below is open), so walls look raised
    if (showFront)
    {
        DrawRectangleGradientV(static_cast<int>(rect.x), static_cast<int>(top.y + top.height), static_cast<int>(rect.width),
                               static_cast<int>(frontHeight) + 1, p.wallDark, ColorLerp(p.wallDark, BLACK, 0.35f));
    }
}

void drawFloorTile(Location location, Rectangle rect, int row, int col)
{
    const LocationPalette& p = getPalette(location);
    DrawRectangleRec(rect, (row + col) % 2 == 0 ? p.floorA : p.floorB);  // checkered slabs
    DrawRectangleLinesEx(rect, 1.0f, p.floorLine);
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
                if (isWallLike(maze, row - 1, col))  // soft shadow of the wall above
                {
                    DrawRectangleGradientV(static_cast<int>(rect.x), static_cast<int>(rect.y), static_cast<int>(rect.width),
                                           static_cast<int>(rect.height * 0.4f), Fade(BLACK, 0.45f), Fade(BLACK, 0.0f));
                }
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

    // 3. Weather, timer effect and darkness (kept inside the maze area)
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(total), static_cast<int>(total));
    drawParticles(location, area, time);
    drawTimerPressure(state, area, time);
    if (settings.light == LightMode::Torch)
    {
        drawDarkness(state, view, TORCH_MAX_DARKNESS);
        const Vector2 player = getRectCenter(getCellRect(view, state.player.row, state.player.col));
        DrawCircleGradient(player, settings.lightRange * view.tileSize, Fade(p.glow, 0.16f), Fade(p.glow, 0.0f));
    }
    else if (settings.light == LightMode::Flashlight)
    {
        drawDarkness(state, view, FLASHLIGHT_MAX_DARKNESS);
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
