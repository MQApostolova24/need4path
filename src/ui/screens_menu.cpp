// Main menu, expedition setup, rules and Hall of Fame screens.

#include "ui/screens.h"

#include "core/utils.h"
#include "core/validation.h"
#include "ui/maze_renderer.h"
#include "ui/theme.h"

#include <algorithm>

namespace
{
const int CENTER_X = SCREEN_WIDTH / 2;

Vector2 point(int x, int y)
{
    return {static_cast<float>(x), static_cast<float>(y)};
}

// ---------- New expedition helpers ----------

void adjustMazeSize(App& app, int delta)
{
    int value = DEFAULT_MAZE_SIZE;
    if (!app.sizeField.text.empty())
    {
        value = std::stoi(app.sizeField.text);  // the field only accepts digits
    }
    value = std::clamp(value + delta, MIN_MAZE_SIZE, MAX_MAZE_SIZE);
    app.sizeField.text = std::to_string(value);
    app.setupError.clear();
}

void drawCardLine(int x, int& y, const char* text)
{
    DrawRectangle(x, y + 7, 6, 6, theme::DARK_GOLD);
    DrawText(text, x + 14, y, 19, theme::PAPYRUS);
    y += 28;
}

void drawDifficultyCard(Rectangle card, Difficulty difficulty, bool selected)
{
    const DifficultySettings& settings = getDifficultySettings(difficulty);
    DrawRectangleRounded(card, 0.08f, 8, selected ? theme::PANEL_LIGHT : Fade(theme::PANEL, 0.8f));
    DrawRectangleRoundedLinesEx(card, 0.08f, 8, selected ? 4.0f : 2.0f, selected ? theme::BRIGHT_GOLD : theme::STONE);

    const int x = static_cast<int>(card.x) + 22;
    int y = static_cast<int>(card.y) + 20;
    DrawText(settings.name, x, y, 34, selected ? theme::BRIGHT_GOLD : theme::SAND_LIGHT);
    DrawText(settings.rank, x, y + 42, 20, theme::TURQUOISE);
    y += 86;

    if (settings.torchRadius > 0.0f)
    {
        drawCardLine(x, y, TextFormat("Torch light: %.1f tiles", static_cast<double>(settings.torchRadius)));
    }
    else
    {
        drawCardLine(x, y, "The whole tomb is lit");
    }
    drawCardLine(x, y, settings.trapDensity > 0.0f ? "Hidden scarab traps" : "No traps");
    drawCardLine(x, y, settings.hasTimeLimit ? "The curse timer runs" : "No time limit");
    drawCardLine(x, y, settings.hasMummy ? "A mummy hunts you" : "No tomb guardians");
    drawCardLine(x, y, TextFormat("%d lives", settings.lives));
    drawCardLine(x, y, TextFormat("Score bonus x%.1f", static_cast<double>(settings.scoreMultiplier)));
}

void tryBeginExpedition(App& app)
{
    int size = 0;
    if (!parseMazeSize(app.sizeField.text, size, app.setupError))
    {
        return;
    }
    if (!beginExpedition(app, size, app.selectedDifficulty))
    {
        app.setupError = "The tomb could not be generated. Try again.";
    }
}

// ---------- Rules helpers ----------

void drawParagraph(const char* const lines[], int count, int x, int& y)
{
    for (int i = 0; i < count; ++i)
    {
        DrawText(lines[i], x, y, 19, theme::PAPYRUS);
        y += 25;
    }
}

void drawHeading(const char* text, int x, int& y)
{
    DrawText(text, x, y, 24, theme::BRIGHT_GOLD);
    y += 34;
}

void drawElementRow(int x, int y, const char* name, const char* description)
{
    DrawText(name, x + 44, y, 20, theme::SAND_LIGHT);
    DrawText(description, x + 200, y, 19, theme::PAPYRUS);
}
}  // namespace

void runMainMenuScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("TOMB OF THE FORGOTTEN PHARAOH", nullptr, 50);
    drawCenteredText(TextFormat("Welcome, explorer %s", app.currentUser.c_str()), CENTER_X, 135, 24,
                     theme::PAPYRUS);

    const char* labels[] = {"New Expedition", "How to Play", "Hall of Fame", "Log Out", "Quit Game"};
    const int count = 5;
    int activated = handleMenuNavigation(app.menuSelection, count);

    for (int i = 0; i < count; ++i)
    {
        const Rectangle button = {static_cast<float>(CENTER_X) - 170.0f, 200.0f + 74.0f * static_cast<float>(i),
                                  340.0f, 56.0f};
        if (drawButton(button, labels[i], i == app.menuSelection))
        {
            activated = i;
        }
    }

    drawCenteredText("Arrow keys + Enter, or use the mouse.  Esc - quit", CENTER_X, SCREEN_HEIGHT - 40, 18,
                     theme::INK);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        activated = 4;
    }

    switch (activated)
    {
    case 0:
        changeScreen(app, Screen::NewGame);
        break;
    case 1:
        changeScreen(app, Screen::Rules);
        break;
    case 2:
        changeScreen(app, Screen::Leaderboard);
        break;
    case 3:
        app.currentUser.clear();
        app.loginUsername.text.clear();
        app.loginPassword.text.clear();
        app.statusMessage = "You left the tomb. See you soon!";
        app.statusIsError = false;
        changeScreen(app, Screen::Login);
        break;
    case 4:
        app.screen = Screen::Exit;
        break;
    default:
        break;
    }
}

void runNewGameScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("PREPARE YOUR EXPEDITION", nullptr, 30);

    const Rectangle panel = {80.0f, 130.0f, 1120.0f, 600.0f};
    drawPanel(panel);

    // --- Tomb size ---
    const Rectangle minusButton = {140.0f, 190.0f, 56.0f, 50.0f};
    const Rectangle sizeBox = {206.0f, 190.0f, 110.0f, 50.0f};
    const Rectangle plusButton = {326.0f, 190.0f, 56.0f, 50.0f};

    TextField* fields[] = {&app.sizeField};
    const Rectangle boxes[] = {sizeBox};
    app.sizeField.active = true;
    const std::string before = app.sizeField.text;
    updateTextFields(fields, boxes, 1);
    if (app.sizeField.text != before)
    {
        app.setupError.clear();
    }
    drawTextField(app.sizeField, sizeBox);

    if (drawButton(minusButton, "-", false) || IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))
    {
        adjustMazeSize(app, -1);
    }
    if (drawButton(plusButton, "+", false) || IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))
    {
        adjustMazeSize(app, 1);
    }

    DrawText("Type a number or use Up / Down.", 420, 196, 20, theme::PAPYRUS);
    DrawText("Bigger tombs hide more secret doors, treasure and traps.", 420, 222, 18, theme::SAND);

    // --- Difficulty cards ---
    DrawText("Choose your difficulty (Left / Right):", 140, 270, 22, theme::SAND_LIGHT);

    if (IsKeyPressed(KEY_LEFT))
    {
        app.selectedDifficulty = static_cast<Difficulty>((static_cast<int>(app.selectedDifficulty) + 2) % 3);
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.selectedDifficulty = static_cast<Difficulty>((static_cast<int>(app.selectedDifficulty) + 1) % 3);
    }

    for (int i = 0; i < 3; ++i)
    {
        const Rectangle card = {140.0f + 340.0f * static_cast<float>(i), 305.0f, 300.0f, 290.0f};
        const Difficulty difficulty = static_cast<Difficulty>(i);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), card))
        {
            app.selectedDifficulty = difficulty;
        }
        drawDifficultyCard(card, difficulty, difficulty == app.selectedDifficulty);
    }

    if (!app.setupError.empty())
    {
        drawCenteredText(app.setupError.c_str(), CENTER_X, 612, 22, theme::BLOOD_LIGHT);
    }

    // --- Buttons ---
    bool begin = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    if (drawButton({static_cast<float>(CENTER_X) - 320.0f, 650.0f, 300.0f, 54.0f}, "Back", false) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        changeScreen(app, Screen::MainMenu);
        return;
    }
    if (drawButton({static_cast<float>(CENTER_X) + 20.0f, 650.0f, 300.0f, 54.0f}, "Begin Expedition", false))
    {
        begin = true;
    }

    drawCenteredText("Enter - begin     Esc - back", CENTER_X, SCREEN_HEIGHT - 40, 18, theme::INK);

    if (begin)
    {
        tryBeginExpedition(app);
    }
}

void runRulesScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("HOW TO PLAY", nullptr, 20);

    const Rectangle panel = {60.0f, 105.0f, 1160.0f, 590.0f};
    drawPanel(panel);

    // ----- Left column -----
    const int leftX = 100;
    int y = 130;

    drawHeading("THE LEGEND", leftX, y);
    const char* legend[] = {
        "Pharaoh Akhen-Ra sealed his tomb with a curse.",
        "Every expedition finds a new labyrinth of",
        "sandstone corridors. You enter at the top-left",
        "corner - the Sun Gate waits at the bottom-right.",
    };
    drawParagraph(legend, 4, leftX, y);
    y += 14;

    drawHeading("HOW TO PLAY", leftX, y);
    const char* howTo[] = {
        "- Move with the Arrow keys (or W A S D).",
        "- Solid sandstone walls cannot be crossed.",
        "- Walls marked with the Eye of Horus are secret",
        "  doors. Each Ankh key opens one - a shortcut!",
        "- Collect golden treasure for extra points.",
        "- Reach the Sun Gate to escape and win.",
        "- Press P or Esc to pause the expedition.",
    };
    drawParagraph(howTo, 7, leftX, y);
    y += 14;

    drawHeading("SCORING", leftX, y);
    const char* scoring[] = {
        "Route length, treasure, lives left and speed,",
        "multiplied by the difficulty bonus.",
    };
    drawParagraph(scoring, 2, leftX, y);

    // ----- Right column -----
    const int rightX = 660;
    y = 130;
    drawHeading("TOMB ELEMENTS", rightX, y);

    const float iconSize = 28.0f;
    const int rowStep = 38;

    drawExplorerSprite(point(rightX + 16, y + 10), iconSize);
    drawElementRow(rightX, y, "Explorer", "that's you");
    y += rowStep;

    drawWallTile({static_cast<float>(rightX + 2), static_cast<float>(y - 4), iconSize, iconSize}, 0, 0);
    drawElementRow(rightX, y, "Wall", "solid sandstone");
    y += rowStep;

    const Rectangle doorRect = {static_cast<float>(rightX + 2), static_cast<float>(y - 4), iconSize, iconSize};
    drawWallTile(doorRect, 0, 0);
    drawHiddenDoorSprite(doorRect, time);
    drawElementRow(rightX, y, "Secret door", "needs an Ankh key");
    y += rowStep;

    drawAnkhIcon(point(rightX + 16, y + 10), iconSize * 0.8f, theme::TURQUOISE);
    drawElementRow(rightX, y, "Ankh key", "opens one secret door");
    y += rowStep;

    drawTreasureSprite(point(rightX + 16, y + 10), iconSize);
    drawElementRow(rightX, y, "Treasure", "+75 points (before bonus)");
    y += rowStep;

    drawTrapSprite(point(rightX + 16, y + 10), iconSize, false);
    drawElementRow(rightX, y, "Scarab trap", "hidden in side halls, -1 life");
    y += rowStep;

    drawMummySprite(point(rightX + 16, y + 10), iconSize);
    drawElementRow(rightX, y, "Mummy", "Hard only - hunts you, -1 life");
    y += rowStep;

    drawExitSprite({static_cast<float>(rightX + 2), static_cast<float>(y - 4), iconSize, iconSize}, time);
    drawElementRow(rightX, y, "Sun Gate", "the way out");
    y += rowStep + 10;

    drawHeading("DIFFICULTIES", rightX, y);
    const char* difficulties[] = {
        "Easy - fully lit, no traps, no timer, 3 lives.",
        "Medium - torch light, traps, curse timer.",
        "Hard - small torch, more traps, a hunting",
        "mummy, short timer and only 2 lives.",
    };
    drawParagraph(difficulties, 4, rightX, y);

    if (drawButton({static_cast<float>(CENTER_X) - 150.0f, 715.0f, 300.0f, 52.0f}, "Back", true) ||
        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        changeScreen(app, Screen::MainMenu);
    }
}

void runLeaderboardScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("HALL OF FAME", "The greatest explorers who escaped the tomb", 30);

    // --- Tabs ---
    const char* tabs[] = {"All", "Easy", "Medium", "Hard"};
    if (IsKeyPressed(KEY_LEFT))
    {
        app.leaderboardTab = (app.leaderboardTab + 3) % 4;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.leaderboardTab = (app.leaderboardTab + 1) % 4;
    }
    for (int i = 0; i < 4; ++i)
    {
        const Rectangle tab = {static_cast<float>(CENTER_X) - 340.0f + 175.0f * static_cast<float>(i), 145.0f,
                               160.0f, 44.0f};
        if (drawButton(tab, tabs[i], i == app.leaderboardTab))
        {
            app.leaderboardTab = i;
        }
    }

    // --- Table ---
    const Rectangle panel = {140.0f, 205.0f, 1000.0f, 470.0f};
    drawPanel(panel);

    const int columns[] = {170, 230, 470, 640, 760, 880, 1000};
    const char* headers[] = {"#", "Explorer", "Difficulty", "Size", "Time", "Moves", "Score"};
    for (int i = 0; i < 7; ++i)
    {
        DrawText(headers[i], columns[i], 228, 22, theme::BRIGHT_GOLD);
    }
    DrawLineEx({160.0f, 258.0f}, {1120.0f, 258.0f}, 2.0f, theme::DARK_GOLD);

    const bool filter = app.leaderboardTab > 0;
    const Difficulty difficulty = static_cast<Difficulty>(std::max(0, app.leaderboardTab - 1));
    const std::vector<ScoreEntry> top = getTopScores(app.leaderboard, filter, difficulty, LEADERBOARD_SIZE);

    if (top.empty())
    {
        drawCenteredText("No expeditions recorded yet. Be the first to escape!", CENTER_X, 420, 22,
                         theme::PAPYRUS);
    }

    for (std::size_t i = 0; i < top.size(); ++i)
    {
        const ScoreEntry& entry = top[i];
        const int y = 270 + 39 * static_cast<int>(i);
        const bool isMe = entry.username == app.currentUser;
        const Color color = isMe ? theme::BRIGHT_GOLD : theme::PAPYRUS;

        if (i % 2 == 0)
        {
            DrawRectangle(160, y - 6, 960, 36, Fade(theme::SAND_DARK, 0.15f));
        }

        DrawText(TextFormat("%d", static_cast<int>(i) + 1), columns[0], y, 22, color);
        DrawText(entry.username.c_str(), columns[1], y, 22, color);
        DrawText(difficultyName(entry.difficulty), columns[2], y, 22, color);
        DrawText(TextFormat("%dx%d", entry.mazeSize, entry.mazeSize), columns[3], y, 22, color);
        DrawText(formatTime(entry.timeSeconds).c_str(), columns[4], y, 22, color);
        DrawText(TextFormat("%d", entry.moves), columns[5], y, 22, color);
        DrawText(TextFormat("%d", entry.score), columns[6], y, 22, color);
    }

    if (drawButton({static_cast<float>(CENTER_X) - 150.0f, 700.0f, 300.0f, 52.0f}, "Back", false) ||
        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        changeScreen(app, Screen::MainMenu);
    }
}
