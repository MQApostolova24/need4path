// Lost Pyramids - a maze game written in C++ with raylib.
// Explore pyramids in Egypt, Mexico and Cambodia before their timers run out.
//
// Project structure:
//   src/core - game logic (maze, locations, rules, accounts, rewards). No graphics, no cin/cout.
//   src/ui   - everything that is drawn on the screen or read from the keyboard/mouse.
//   assets   - themed fonts (SIL Open Font License).

#include "raylib.h"
#include "ui/app.h"
#include "ui/fonts.h"
#include "ui/theme.h"

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lost Pyramids");
    SetExitKey(KEY_NULL);  // Esc is used for "back" and "pause", not to close the window
    SetTargetFPS(60);
    loadGameFonts();

    App app;
    initApp(app);

    while (app.screen != Screen::Exit && !WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(theme::INK);
        runCurrentScreen(app);
        EndDrawing();
    }

    unloadGameFonts();
    CloseWindow();
    return 0;
}
