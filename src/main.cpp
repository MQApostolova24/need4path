// Tomb of the Forgotten Pharaoh - a maze game written in C++ with raylib.
//
// Project structure:
//   src/core - game logic (maze, rules, accounts, Hall of Fame). No graphics, no cin/cout.
//   src/ui   - everything that is drawn on the screen or read from the keyboard/mouse.

#include "raylib.h"
#include "ui/app.h"
#include "ui/theme.h"

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of the Forgotten Pharaoh");
    SetExitKey(KEY_NULL);  // Esc is used for "back" and "pause", not to close the window
    SetTargetFPS(60);

    App app;
    initApp(app);

    while (app.screen != Screen::Exit && !WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(theme::NIGHT_SKY);
        runCurrentScreen(app);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
