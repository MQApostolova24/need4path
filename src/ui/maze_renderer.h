#pragma once

#include "core/game.h"
#include "raylib.h"

// Drawing of the tomb (top-down view): tiles, items, the mummy,
// the torch light and the explorer.

struct MazeView
{
    float originX = 0.0f;
    float originY = 0.0f;
    float tileSize = 0.0f;
};

// Fits a size x size tomb inside the given screen area.
MazeView computeMazeView(int mazeSize, Rectangle area);

// Draws the whole scene for the current game state.
void drawMazeScene(const GameState& state, const MazeView& view, float time);

// Single tiles and sprites (also reused by the HUD legend and the rules screen).
void drawWallTile(Rectangle rect, int row, int col);
void drawFloorTile(Rectangle rect, int row, int col);
void drawHiddenDoorSprite(Rectangle rect, float time);
void drawExitSprite(Rectangle rect, float time);
void drawExplorerSprite(Vector2 center, float size);
void drawMummySprite(Vector2 center, float size);
void drawTreasureSprite(Vector2 center, float size);
void drawTrapSprite(Vector2 center, float size, bool sprung);
