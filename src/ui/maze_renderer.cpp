#include "ui/maze_renderer.h"

#include "ui/theme.h"
#include "ui/widgets.h"

#include <cmath>

namespace
{
Rectangle getCellRect(const MazeView& view, int row, int col)
{
    return {view.originX + static_cast<float>(col) * view.tileSize,
            view.originY + static_cast<float>(row) * view.tileSize, view.tileSize, view.tileSize};
}

Vector2 getRectCenter(Rectangle rect)
{
    return {rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f};
}

void drawItems(const GameState& state, const MazeView& view)
{
    const Maze& maze = state.maze;
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            const Cell& cell = maze.at(row, col);
            if (cell.item == ItemType::None)
            {
                continue;
            }

            // Keys and treasure are remembered once seen; live traps only show inside the torch light.
            const bool lit = isCellLit(state, row, col);
            const bool remembered = lit || cell.explored;
            const Vector2 center = getRectCenter(getCellRect(view, row, col));
            const float size = view.tileSize;

            switch (cell.item)
            {
            case ItemType::Ankh:
                if (remembered)
                {
                    drawAnkhIcon(center, size * 0.75f, theme::TURQUOISE);
                }
                break;
            case ItemType::Treasure:
                if (remembered)
                {
                    drawTreasureSprite(center, size);
                }
                break;
            case ItemType::Trap:
                if (lit)
                {
                    drawTrapSprite(center, size, false);
                }
                break;
            case ItemType::SprungTrap:
                if (remembered)
                {
                    drawTrapSprite(center, size, true);
                }
                break;
            case ItemType::None:
                break;
            }
        }
    }
}

// Darkness outside the torch. The layout stays faintly visible so the
// player always sees walls, openings and the exit.
void drawTorchFog(const GameState& state, const MazeView& view, float time, float baseRadius)
{
    const float radius = baseRadius + 0.2f * std::sin(time * 9.0f) + 0.1f * std::sin(time * 23.0f);
    const GridPos exit = state.maze.getExit();

    for (int row = 0; row < state.maze.getSize(); ++row)
    {
        for (int col = 0; col < state.maze.getSize(); ++col)
        {
            const float rowOffset = static_cast<float>(row - state.player.row);
            const float colOffset = static_cast<float>(col - state.player.col);
            const float distance = std::sqrt(rowOffset * rowOffset + colOffset * colOffset);

            float darkness = 0.75f;
            if (distance <= radius)
            {
                const float ratio = distance / radius;
                darkness = 0.5f * ratio * ratio;
            }
            if (row == exit.row && col == exit.col)
            {
                darkness = std::fmin(darkness, 0.25f);  // the Sun Gate always glows
            }
            if (darkness > 0.01f)
            {
                DrawRectangleRec(getCellRect(view, row, col), Fade(BLACK, darkness));
            }
        }
    }

    // Warm torch glow around the explorer
    const Vector2 center = getRectCenter(getCellRect(view, state.player.row, state.player.col));
    DrawCircleGradient(static_cast<int>(center.x), static_cast<int>(center.y), radius * view.tileSize,
                       Fade(theme::BRIGHT_GOLD, 0.10f), Fade(theme::BRIGHT_GOLD, 0.0f));
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

void drawWallTile(Rectangle rect, int row, int col)
{
    DrawRectangleRec(rect, theme::STONE);
    const float edge = std::fmax(1.0f, std::floor(rect.height / 8.0f));
    DrawRectangleRec({rect.x, rect.y, rect.width, edge}, theme::SAND_DARK);
    DrawRectangleRec({rect.x, rect.y + rect.height - edge, rect.width, edge}, theme::STONE_DARK);

    if (rect.width >= 12.0f)
    {
        // Brick seams, shifted on every other block
        const float middleY = rect.y + rect.height / 2.0f;
        const bool even = (row + col) % 2 == 0;
        const float topSeam = rect.x + rect.width * (even ? 0.3f : 0.7f);
        const float bottomSeam = rect.x + rect.width * (even ? 0.7f : 0.3f);
        DrawLineEx({rect.x, middleY}, {rect.x + rect.width, middleY}, 1.0f, theme::STONE_DARK);
        DrawLineEx({topSeam, rect.y + edge}, {topSeam, middleY}, 1.0f, theme::STONE_DARK);
        DrawLineEx({bottomSeam, middleY}, {bottomSeam, rect.y + rect.height - edge}, 1.0f, theme::STONE_DARK);
    }
}

void drawFloorTile(Rectangle rect, int row, int col)
{
    DrawRectangleRec(rect, (row + col) % 2 == 0 ? theme::FLOOR : theme::FLOOR_LIGHT);
}

void drawHiddenDoorSprite(Rectangle rect, float time)
{
    const float pulse = 0.65f + 0.35f * std::sin(time * 3.0f);
    DrawRectangleLinesEx(rect, std::fmax(1.0f, rect.width / 12.0f), Fade(theme::BRIGHT_GOLD, pulse));
    drawEyeIcon(getRectCenter(rect), rect.width * 0.85f, Fade(theme::BRIGHT_GOLD, pulse));
}

void drawExitSprite(Rectangle rect, float time)
{
    const float glow = 0.5f + 0.5f * std::sin(time * 2.5f);
    const Vector2 center = getRectCenter(rect);
    DrawCircleGradient(static_cast<int>(center.x), static_cast<int>(center.y), rect.width * (1.2f + 0.4f * glow),
                       Fade(theme::BRIGHT_GOLD, 0.45f), Fade(theme::BRIGHT_GOLD, 0.0f));
    DrawRectangleGradientV(static_cast<int>(rect.x), static_cast<int>(rect.y), static_cast<int>(rect.width),
                           static_cast<int>(rect.height), theme::BRIGHT_GOLD, theme::DARK_GOLD);
    DrawRectangleLinesEx(rect, std::fmax(1.0f, rect.width / 10.0f), theme::HAT_BROWN);
    drawAnkhIcon(center, rect.height * 0.75f, theme::INK);
}

void drawExplorerSprite(Vector2 center, float size)
{
    const Vector2 body = {center.x, center.y + size * 0.08f};
    DrawCircleV(body, size * 0.34f, theme::KHAKI);
    DrawCircleLinesV(body, size * 0.34f, theme::INK);

    // Eyes
    DrawCircleV({center.x - size * 0.1f, center.y + size * 0.06f}, std::fmax(1.0f, size * 0.045f), theme::INK);
    DrawCircleV({center.x + size * 0.1f, center.y + size * 0.06f}, std::fmax(1.0f, size * 0.045f), theme::INK);

    // Explorer's hat
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y - size * 0.10f), size * 0.46f, size * 0.12f,
                theme::HAT_BROWN);
    DrawRectangleRounded({center.x - size * 0.2f, center.y - size * 0.38f, size * 0.4f, size * 0.28f}, 0.4f, 4,
                         theme::HAT_BROWN);
    DrawRectangleRec({center.x - size * 0.2f, center.y - size * 0.17f, size * 0.4f, std::fmax(1.0f, size * 0.05f)},
                     theme::BLOOD);
}

void drawMummySprite(Vector2 center, float size)
{
    const Rectangle body = {center.x - size * 0.3f, center.y - size * 0.4f, size * 0.6f, size * 0.8f};
    DrawRectangleRounded(body, 0.5f, 6, theme::BANDAGE);

    const float thickness = std::fmax(1.0f, size * 0.05f);
    for (int i = 1; i <= 3; ++i)
    {
        const float y = body.y + body.height * (0.22f * static_cast<float>(i) + 0.1f);
        DrawLineEx({body.x + 1.0f, y}, {body.x + body.width - 1.0f, y + size * 0.08f}, thickness,
                   theme::BANDAGE_DARK);
    }

    // Glowing red eyes
    DrawCircleV({center.x - size * 0.11f, center.y - size * 0.18f}, std::fmax(1.0f, size * 0.07f), theme::BLOOD);
    DrawCircleV({center.x + size * 0.11f, center.y - size * 0.18f}, std::fmax(1.0f, size * 0.07f), theme::BLOOD);
}

void drawTreasureSprite(Vector2 center, float size)
{
    DrawCircleV(center, size * 0.27f, theme::BRIGHT_GOLD);
    DrawCircleLinesV(center, size * 0.27f, theme::DARK_GOLD);
    DrawCircleV(center, size * 0.1f, theme::DARK_GOLD);
    DrawCircleV({center.x + size * 0.12f, center.y - size * 0.12f}, std::fmax(1.0f, size * 0.05f), theme::PAPYRUS);
}

void drawTrapSprite(Vector2 center, float size, bool sprung)
{
    DrawCircleV(center, size * 0.34f, theme::PIT);
    if (sprung)
    {
        // An empty, cracked pit
        DrawLineEx({center.x - size * 0.22f, center.y - size * 0.1f}, {center.x + size * 0.18f, center.y + size * 0.14f},
                   1.0f, theme::STONE);
        DrawLineEx({center.x - size * 0.05f, center.y + size * 0.2f}, {center.x + size * 0.1f, center.y - size * 0.2f},
                   1.0f, theme::STONE);
        return;
    }
    drawScarabIcon(center, size * 0.5f, theme::TURQUOISE);
}

void drawMazeScene(const GameState& state, const MazeView& view, float time)
{
    const Maze& maze = state.maze;
    const int size = maze.getSize();
    const float total = view.tileSize * static_cast<float>(size);
    const DifficultySettings& settings = getDifficultySettings(state.difficulty);

    // Golden frame around the tomb
    DrawRectangleRec({view.originX - 6.0f, view.originY - 6.0f, total + 12.0f, total + 12.0f}, theme::DARK_GOLD);

    // 1. Tiles
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            const Rectangle rect = getCellRect(view, row, col);
            switch (maze.at(row, col).tile)
            {
            case TileType::Wall:
                drawWallTile(rect, row, col);
                break;
            case TileType::SecretDoor:
                drawWallTile(rect, row, col);
                drawHiddenDoorSprite(rect, time);
                break;
            case TileType::OpenedDoor:
                drawFloorTile(rect, row, col);
                DrawRectangleLinesEx(rect, std::fmax(1.0f, view.tileSize / 10.0f), theme::DARK_GOLD);
                break;
            case TileType::Floor:
            case TileType::Exit:
                drawFloorTile(rect, row, col);
                break;
            }
        }
    }

    // 2. Items, the exit and the mummy
    drawItems(state, view);
    const GridPos exit = maze.getExit();
    drawExitSprite(getCellRect(view, exit.row, exit.col), time);

    if (state.mummy.active && isCellLit(state, state.mummy.pos.row, state.mummy.pos.col))
    {
        drawMummySprite(getRectCenter(getCellRect(view, state.mummy.pos.row, state.mummy.pos.col)), view.tileSize);
    }

    // 3. Torch light
    if (settings.torchRadius > 0.0f)
    {
        drawTorchFog(state, view, time, settings.torchRadius);
    }

    // 4. The explorer (blinks while protected after losing a life)
    const bool blinking = state.invulnerableSeconds > 0.0f && std::fmod(time, 0.2f) < 0.1f;
    if (!blinking)
    {
        drawExplorerSprite(getRectCenter(getCellRect(view, state.player.row, state.player.col)), view.tileSize);
    }
}
