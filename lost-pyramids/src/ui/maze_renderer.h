#pragma once

#include "core/game.h"
#include "raylib.h"

// Drawing of the pyramid (top-down view): stone tiles, rewards, the guardian,
// torch light, weather particles, the timer effect and the explorer.

struct MazeView
{
    float originX = 0.0f;
    float originY = 0.0f;
    float tileSize = 0.0f;
};

// Fits a size x size pyramid inside the given screen area.
MazeView computeMazeView(int mazeSize, Rectangle area);

// Draws the whole scene for the current game state, with the explorer in the given skin.
void drawMazeScene(const GameState& state, const MazeView& view, float time, int skinId);

// The classic TEXT view of the maze (assignment requirement 5), seen from above:
//   #  wall          .  corridor        P  player        E  exit
//   D  secret door   /  opened door     k  key           $  treasure
//   *  gem           R  relic           +  time pickup   x  trap   G  guardian
// Rewards, traps and the guardian follow the same visibility rules as the picture view.
void drawTextMaze(const GameState& state, const MazeView& view);

// Single tiles (also used by the rules screen).
// "showFront" draws the front face of the wall block (when nothing stands below it).
void drawWallTile(Location location, Rectangle rect, int row, int col, bool showFront);
void drawFloorTile(Location location, Rectangle rect, int row, int col);
