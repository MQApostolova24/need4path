// The game screen (tomb + side panel), the pause menu and the results screen.

#include "ui/screens.h"

#include "core/utils.h"
#include "ui/maze_renderer.h"
#include "ui/theme.h"

#include <cmath>

namespace
{
const int CENTER_X = SCREEN_WIDTH / 2;
const float FIRST_REPEAT_DELAY = 0.18f;  // hold an arrow key to keep walking
const float REPEAT_INTERVAL = 0.085f;
const float EVENT_MESSAGE_SECONDS = 3.0f;

const Rectangle MAZE_AREA = {20.0f, 20.0f, 760.0f, 760.0f};
const Rectangle HUD_AREA = {800.0f, 20.0f, 460.0f, 760.0f};

Vector2 point(int x, int y)
{
    return {static_cast<float>(x), static_cast<float>(y)};
}

// ---------- Input ----------

bool isDirectionKeyDown(Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    case Direction::Down:
        return IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    case Direction::Left:
        return IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    case Direction::Right:
        return IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    case Direction::None:
        break;
    }
    return false;
}

Direction getPressedDirection()
{
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        return Direction::Up;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        return Direction::Down;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    {
        return Direction::Left;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        return Direction::Right;
    }
    return Direction::None;
}

// Turns key presses into a direction. Holding a key repeats the move.
Direction readMovementInput(App& app, float deltaSeconds)
{
    const Direction pressed = getPressedDirection();
    if (pressed != Direction::None)
    {
        app.heldDirection = pressed;
        app.moveCooldown = FIRST_REPEAT_DELAY;
        return pressed;
    }

    if (app.heldDirection == Direction::None || !isDirectionKeyDown(app.heldDirection))
    {
        app.heldDirection = Direction::None;
        return Direction::None;
    }

    app.moveCooldown -= deltaSeconds;
    if (app.moveCooldown > 0.0f)
    {
        return Direction::None;
    }
    app.moveCooldown = REPEAT_INTERVAL;
    return app.heldDirection;
}

// ---------- Messages ----------

void showEvent(App& app, GameEvent event)
{
    const char* text = nullptr;
    Color color = theme::PAPYRUS;

    switch (event)
    {
    case GameEvent::DoorLocked:
        text = "Sealed door! Find an Ankh key.";
        color = theme::SAND_LIGHT;
        break;
    case GameEvent::DoorOpened:
        text = "The Ankh glows... the door opens!";
        color = theme::BRIGHT_GOLD;
        break;
    case GameEvent::AnkhCollected:
        text = "You found an Ankh key!";
        color = theme::TURQUOISE;
        break;
    case GameEvent::TreasureCollected:
        text = "Golden treasure! +75 points";
        color = theme::BRIGHT_GOLD;
        break;
    case GameEvent::TrapTriggered:
        text = "Scarab trap! You lost a life.";
        color = theme::BLOOD_LIGHT;
        break;
    case GameEvent::CaughtByMummy:
        text = "The mummy got you! -1 life";
        color = theme::BLOOD_LIGHT;
        break;
    default:
        break;
    }

    if (text != nullptr)
    {
        app.eventMessage = text;
        app.eventColor = color;
        app.eventTimer = EVENT_MESSAGE_SECONDS;
    }
}

void finishExpedition(App& app)
{
    app.finalScore = calculateScore(app.game);
    app.scoreSaved = false;
    if (app.game.status == GameStatus::Won)
    {
        app.scoreSaved = saveScore(LEADERBOARD_FILE_PATH, createScoreEntry(app.game, app.currentUser));
    }
    changeScreen(app, Screen::Results);
}

// ---------- Side panel (HUD) ----------

void drawInfoLine(const char* label, const char* value, int x, int y)
{
    DrawText(label, x, y, 20, theme::SAND_LIGHT);
    DrawText(value, x + 140, y, 20, theme::PAPYRUS);
}

void drawCurseTimer(const GameState& game, int x, int y, int width, float time)
{
    const DifficultySettings& settings = getDifficultySettings(game.difficulty);
    if (!settings.hasTimeLimit)
    {
        DrawText("No curse timer - take your time.", x, y + 8, 18, theme::SAND);
        return;
    }

    const float remaining = getRemainingTime(game);
    const float fraction = game.timeLimitSeconds > 0.0f ? remaining / game.timeLimitSeconds : 0.0f;
    const bool danger = fraction < 0.2f;

    DrawText("Curse timer", x, y, 20, theme::SAND_LIGHT);
    DrawText(formatTime(remaining).c_str(), x + 140, y, 20, danger ? theme::BLOOD_LIGHT : theme::PAPYRUS);

    const Rectangle bar = {static_cast<float>(x), static_cast<float>(y + 26), static_cast<float>(width), 16.0f};
    DrawRectangleRec(bar, theme::STONE_DARK);
    Color fill = danger ? theme::BLOOD : theme::SAND;
    if (danger && std::fmod(time, 0.5f) < 0.25f)
    {
        fill = theme::BLOOD_LIGHT;
    }
    DrawRectangleRec({bar.x, bar.y, bar.width * fraction, bar.height}, fill);
    DrawRectangleLinesEx(bar, 1.0f, theme::DARK_GOLD);
}

void drawLegend(int x, int y, float time)
{
    DrawText("TOMB GUIDE", x, y, 20, theme::BRIGHT_GOLD);
    y += 30;
    const float icon = 24.0f;

    drawAnkhIcon(point(x + 14, y + 10), icon * 0.8f, theme::TURQUOISE);
    DrawText("Ankh key - opens a secret door", x + 40, y, 18, theme::PAPYRUS);
    y += 30;

    const Rectangle door = {static_cast<float>(x + 2), static_cast<float>(y - 2), icon, icon};
    drawWallTile(door, 0, 0);
    drawHiddenDoorSprite(door, time);
    DrawText("Secret door - a shortcut", x + 40, y, 18, theme::PAPYRUS);
    y += 30;

    drawTreasureSprite(point(x + 14, y + 10), icon);
    DrawText("Treasure - bonus points", x + 40, y, 18, theme::PAPYRUS);
    y += 30;

    drawTrapSprite(point(x + 14, y + 10), icon, false);
    DrawText("Scarab trap - costs a life", x + 40, y, 18, theme::PAPYRUS);
    y += 30;

    drawMummySprite(point(x + 14, y + 10), icon);
    DrawText("Mummy - run from it!", x + 40, y, 18, theme::PAPYRUS);
    y += 30;

    drawExitSprite({static_cast<float>(x + 2), static_cast<float>(y - 2), icon, icon}, time);
    DrawText("Sun Gate - the way out", x + 40, y, 18, theme::PAPYRUS);
}

void drawHud(const App& app, float time)
{
    const GameState& game = app.game;
    const DifficultySettings& settings = getDifficultySettings(game.difficulty);
    const int size = game.maze.getSize();

    drawPanel(HUD_AREA);
    const int x = static_cast<int>(HUD_AREA.x) + 24;
    const int width = static_cast<int>(HUD_AREA.width) - 48;
    int y = static_cast<int>(HUD_AREA.y) + 18;

    drawCenteredText("THE PHARAOH'S TOMB", static_cast<int>(HUD_AREA.x + HUD_AREA.width / 2.0f), y, 28,
                     theme::BRIGHT_GOLD);
    y += 40;
    DrawLineEx(point(x, y), point(x + width, y), 2.0f, theme::DARK_GOLD);
    y += 12;

    // Who and where
    drawInfoLine("Explorer", app.currentUser.c_str(), x, y);
    y += 26;
    drawInfoLine("Difficulty", TextFormat("%s (%s)", settings.name, settings.rank), x, y);
    y += 26;
    drawInfoLine("Tomb", TextFormat("%d x %d", size, size), x, y);
    y += 26;
    drawInfoLine("Position", TextFormat("row %d, column %d", game.player.row + 1, game.player.col + 1), x, y);
    y += 32;

    // Inventory and progress
    DrawText("Lives", x, y, 20, theme::SAND_LIGHT);
    for (int i = 0; i < settings.lives; ++i)
    {
        drawHeartIcon(point(x + 152 + 32 * i, y + 10), 24.0f,
                      i < game.lives ? theme::BLOOD : Fade(theme::STONE, 0.6f));
    }
    y += 34;

    DrawText("Ankh keys", x, y, 20, theme::SAND_LIGHT);
    drawAnkhIcon(point(x + 152, y + 10), 22.0f, theme::TURQUOISE);
    DrawText(TextFormat("x %d", game.keys), x + 172, y, 20, theme::PAPYRUS);
    y += 34;

    DrawText("Treasure", x, y, 20, theme::SAND_LIGHT);
    drawTreasureSprite(point(x + 152, y + 10), 30.0f);
    DrawText(TextFormat("%d / %d", game.treasuresCollected, game.totalTreasures), x + 172, y, 20, theme::PAPYRUS);
    y += 34;

    drawInfoLine("Secret doors", TextFormat("%d / %d opened", game.doorsOpened, game.secretDoors), x, y);
    y += 30;
    drawInfoLine("Moves", TextFormat("%d", game.moves), x, y);
    y += 30;
    drawInfoLine("Time", formatTime(game.elapsedSeconds).c_str(), x, y);
    y += 34;

    drawCurseTimer(game, x, y, width, time);
    y += 54;

    // Latest event
    const Rectangle messageBox = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), 44.0f};
    DrawRectangleRounded(messageBox, 0.3f, 6, Fade(BLACK, 0.35f));
    if (app.eventTimer > 0.0f)
    {
        const float alpha = std::fmin(1.0f, app.eventTimer);
        DrawText(app.eventMessage.c_str(), x + 12, y + 12, 20, Fade(app.eventColor, alpha));
    }
    else
    {
        DrawText("Find the Sun Gate (bottom-right).", x + 12, y + 12, 20, Fade(theme::SAND, 0.7f));
    }
    y += 60;

    drawLegend(x, y, time);

    DrawText("Arrows / WASD - move     P / Esc - pause", x, static_cast<int>(HUD_AREA.y + HUD_AREA.height) - 30,
             16, theme::SAND);
}

// ---------- Pause menu ----------

void runPauseMenu(App& app)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.65f));

    const Rectangle panel = {static_cast<float>(CENTER_X) - 230.0f, static_cast<float>(SCREEN_HEIGHT) / 2.0f - 180.0f,
                             460.0f, 360.0f};
    drawPanel(panel);
    drawCenteredText("PAUSED", CENTER_X, static_cast<int>(panel.y) + 26, 42, theme::BRIGHT_GOLD);
    drawCenteredText("The torch flickers in the silence...", CENTER_X, static_cast<int>(panel.y) + 78, 18,
                     theme::PAPYRUS);

    const char* labels[] = {"Resume", "New Tomb", "Abandon Expedition"};
    int activated = handleMenuNavigation(app.menuSelection, 3);
    for (int i = 0; i < 3; ++i)
    {
        const Rectangle button = {panel.x + 70.0f, panel.y + 125.0f + 70.0f * static_cast<float>(i),
                                  panel.width - 140.0f, 52.0f};
        if (drawButton(button, labels[i], i == app.menuSelection))
        {
            activated = i;
        }
    }

    switch (activated)
    {
    case 0:
        app.paused = false;
        break;
    case 1:
        beginExpedition(app, app.lastMazeSize, app.selectedDifficulty);
        break;
    case 2:
        changeScreen(app, Screen::MainMenu);
        break;
    default:
        break;
    }
}
}  // namespace

void runPlayingScreen(App& app)
{
    const float deltaSeconds = GetFrameTime();
    const float time = static_cast<float>(GetTime());

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P))
    {
        app.paused = !app.paused;
        app.menuSelection = 0;
    }

    if (!app.paused)
    {
        const Direction direction = readMovementInput(app, deltaSeconds);
        if (direction != Direction::None)
        {
            showEvent(app, movePlayer(app.game, direction));
        }
        showEvent(app, updateGame(app.game, deltaSeconds));
        app.eventTimer = std::fmax(0.0f, app.eventTimer - deltaSeconds);
    }

    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, theme::NIGHT_SKY, Color{40, 26, 20, 255});
    const MazeView view = computeMazeView(app.game.maze.getSize(), MAZE_AREA);
    drawMazeScene(app.game, view, time);
    drawHud(app, time);

    if (app.game.status == GameStatus::Won || app.game.status == GameStatus::Lost)
    {
        finishExpedition(app);
        return;
    }

    if (app.paused)
    {
        runPauseMenu(app);
    }
}

void runResultsScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);

    const GameState& game = app.game;
    const bool won = game.status == GameStatus::Won;
    const int size = game.maze.getSize();

    const Rectangle panel = {static_cast<float>(CENTER_X) - 320.0f, 100.0f, 640.0f, 590.0f};
    drawPanel(panel);
    const int top = static_cast<int>(panel.y);

    if (won)
    {
        drawCenteredText("YOU ESCAPED THE TOMB!", CENTER_X, top + 28, 40, theme::BRIGHT_GOLD);
        drawCenteredText("The Sun Gate opens and daylight floods the corridor.", CENTER_X, top + 80, 20,
                         theme::PAPYRUS);
        drawAnkhIcon(point(CENTER_X, top + 150), 80.0f, theme::BRIGHT_GOLD);
    }
    else
    {
        drawCenteredText("THE TOMB CLAIMS ANOTHER SOUL", CENTER_X, top + 28, 36, theme::BLOOD_LIGHT);
        const char* reason = game.lossReason == LossReason::OutOfTime ? "The sands of the curse timer ran out."
                                                                      : "You have no lives left.";
        drawCenteredText(reason, CENTER_X, top + 80, 20, theme::PAPYRUS);
        drawMummySprite(point(CENTER_X, top + 150), 80.0f);
    }

    int y = top + 210;
    if (won)
    {
        drawCenteredText(TextFormat("Final score: %d", app.finalScore), CENTER_X, y, 32, theme::BRIGHT_GOLD);
        y += 46;
    }

    const DifficultySettings& settings = getDifficultySettings(game.difficulty);
    drawCenteredText(TextFormat("Tomb: %d x %d  -  %s", size, size, settings.name), CENTER_X, y, 22, theme::PAPYRUS);
    y += 32;
    drawCenteredText(TextFormat("Time: %s     Moves: %d", formatTime(game.elapsedSeconds).c_str(), game.moves),
                     CENTER_X, y, 22, theme::PAPYRUS);
    y += 32;
    drawCenteredText(TextFormat("Treasure: %d / %d     Lives left: %d", game.treasuresCollected,
                                game.totalTreasures, game.lives),
                     CENTER_X, y, 22, theme::PAPYRUS);
    y += 32;
    drawCenteredText(TextFormat("Secret doors opened: %d / %d", game.doorsOpened, game.secretDoors), CENTER_X, y, 22,
                     theme::PAPYRUS);
    y += 44;

    const char* saved = !won              ? "Only successful escapes enter the Hall of Fame."
                        : app.scoreSaved ? "Your name is carved into the Hall of Fame."
                                         : "The score could not be saved.";
    drawCenteredText(saved, CENTER_X, y, 20, won ? theme::TURQUOISE : theme::SAND);

    const char* labels[] = {"Play Again", "Main Menu"};
    int activated = handleMenuNavigation(app.menuSelection, 2);
    for (int i = 0; i < 2; ++i)
    {
        const Rectangle button = {panel.x + 50.0f + 280.0f * static_cast<float>(i), panel.y + panel.height - 90.0f,
                                  260.0f, 54.0f};
        if (drawButton(button, labels[i], i == app.menuSelection))
        {
            activated = i;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        activated = 1;
    }

    if (activated == 0)
    {
        beginExpedition(app, app.lastMazeSize, app.selectedDifficulty);
    }
    else if (activated == 1)
    {
        changeScreen(app, Screen::MainMenu);
    }
}
