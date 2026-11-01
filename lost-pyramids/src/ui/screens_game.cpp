// The game screen (pyramid + side panel), the pause menu and the results screen.

#include "ui/screens.h"

#include "core/utils.h"
#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/maze_renderer.h"
#include "ui/sprites.h"
#include "ui/theme.h"

#include <cmath>

namespace
{
const float CENTER_X = static_cast<float>(SCREEN_WIDTH) / 2.0f;
const float FIRST_REPEAT_DELAY = 0.18f;  // hold an arrow key to keep walking
const float REPEAT_INTERVAL = 0.085f;
const float EVENT_MESSAGE_SECONDS = 3.0f;

const Rectangle MAZE_AREA = {20.0f, 20.0f, 760.0f, 760.0f};
const Rectangle HUD_AREA = {800.0f, 20.0f, 460.0f, 760.0f};

// ---------------------------------------------------------------- input

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

// ---------------------------------------------------------------- messages

void showEvent(App& app, GameEvent event)
{
    const LocationInfo& info = getLocationInfo(app.game.location);
    const LocationPalette& p = getPalette(app.game.location);
    std::string text;
    Color color = theme::PARCHMENT;

    switch (event)
    {
    case GameEvent::DoorLocked:
        text = std::string("Sealed! You need a ") + info.keyName + ".";
        color = theme::PARCHMENT_DARK;
        break;
    case GameEvent::DoorOpened:
        text = "A secret door opens - a shortcut!";
        color = p.accent;
        break;
    case GameEvent::KeyCollected:
        text = std::string("You found a ") + info.keyName + "!";
        color = p.accent;
        break;
    case GameEvent::TreasureCollected:
        text = TextFormat("%s  +%d", info.coinName, POINTS_PER_TREASURE);
        color = theme::GOLD_BRIGHT;
        break;
    case GameEvent::GemCollected:
        text = TextFormat("%s!  +%d", info.gemName, POINTS_PER_GEM);
        color = theme::GOLD_BRIGHT;
        break;
    case GameEvent::RelicCollected:
        text = TextFormat("RELIC: %s!", getRelicName(app.game.location, app.game.difficulty));
        color = theme::GOLD_BRIGHT;
        break;
    case GameEvent::TimeCollected:
        text = TextFormat("%s  +%d seconds", info.timePickupName, static_cast<int>(TIME_PICKUP_SECONDS));
        color = theme::SUCCESS;
        break;
    case GameEvent::TrapTriggered:
        text = std::string(info.trapName) + "!  -1 life";
        color = theme::DANGER;
        break;
    case GameEvent::CaughtByGuardian:
        text = std::string("The ") + info.guardianName + " got you!  -1 life";
        color = theme::DANGER;
        break;
    default:
        break;
    }

    if (!text.empty())
    {
        app.eventMessage = text;
        app.eventColor = color;
        app.eventTimer = EVENT_MESSAGE_SECONDS;
    }
}

void finishExpedition(App& app)
{
    ExpeditionResult& result = app.result;
    result = ExpeditionResult{};
    result.score = calculateScoreBreakdown(app.game);
    result.stars = calculateStars(app.game);

    if (app.game.status == GameStatus::Won)
    {
        result.scoreSaved = saveScore(LEADERBOARD_FILE_PATH, createScoreEntry(app.game, app.currentUser));
        result.coinsEarned = collectRewards(app.profile, app.game);
        result.relicKept = app.game.relicFound;
        saveCurrentProfile(app);
    }
    changeScreen(app, Screen::Results);
}

// ---------------------------------------------------------------- side panel (HUD)

void drawTimerCard(const GameState& game, float x, float y, float width, float time)
{
    const LocationInfo& info = getLocationInfo(game.location);
    const LocationPalette& p = getPalette(game.location);
    const float fraction = getTimeFraction(game);
    const bool danger = fraction < 0.2f;

    const Rectangle box = {x, y, width, 104.0f};
    DrawRectangleRounded(box, 0.2f, 6, Fade(BLACK, 0.35f));
    drawTimerEmblem(game.location, {x + 52.0f, y + 52.0f}, 72.0f, fraction, time);

    drawString(info.timerName, x + 104.0f, y + 10.0f, 22.0f, p.accent, FontStyle::Bold);
    Color clock = theme::PARCHMENT;
    if (danger && std::fmod(time, 0.5f) < 0.25f)
    {
        clock = theme::DANGER;
    }
    drawString(formatTime(getRemainingTime(game)).c_str(), x + 104.0f, y + 34.0f, 42.0f, clock, FontStyle::Bold);

    const Rectangle bar = {x + 104.0f, y + 84.0f, width - 124.0f, 10.0f};
    DrawRectangleRounded(bar, 0.5f, 4, theme::PANEL_LIGHT);
    DrawRectangleRounded({bar.x, bar.y, bar.width * fraction, bar.height}, 0.5f, 4, danger ? theme::DANGER : p.accent);
}

// One of the four inventory slots: picture, value and a small label.
void drawSlotText(float centerX, float y, const char* value, const char* label, Color valueColor)
{
    drawStringCentered(value, centerX, y + 40.0f, 24.0f, valueColor, FontStyle::Bold);
    drawStringCentered(label, centerX, y + 68.0f, 17.0f, theme::TEXT_MUTED, FontStyle::Body);
}

void drawInventory(const GameState& game, float x, float y, float width, float time)
{
    const Location location = game.location;
    const LocationInfo& info = getLocationInfo(location);
    const float slot = width / 4.0f;
    const float iconY = y + 22.0f;

    DrawRectangleRounded({x, y, width, 100.0f}, 0.2f, 6, Fade(BLACK, 0.35f));

    float cx = x + slot * 0.5f;
    drawKeySprite(location, {cx, iconY}, 30.0f);
    drawSlotText(cx, y, TextFormat("x %d", game.keys), info.keyName, theme::PARCHMENT);

    cx += slot;
    drawTreasureSprite(location, {cx, iconY}, 36.0f, time);
    drawSlotText(cx, y, TextFormat("%d / %d", game.treasuresCollected, game.totalTreasures), info.coinName,
                 theme::PARCHMENT);

    cx += slot;
    drawGemSprite(location, {cx, iconY}, 36.0f, time);
    drawSlotText(cx, y, TextFormat("%d / %d", game.gemsCollected, game.totalGems), info.gemName, theme::PARCHMENT);

    cx += slot;
    if (game.relicFound)
    {
        drawRelicSprite(location, {cx, iconY}, 30.0f, time);
        drawSlotText(cx, y, "Found!", "Relic", theme::GOLD_BRIGHT);
    }
    else
    {
        DrawCircleV({cx, iconY}, 14.0f, Fade(theme::LOCKED, 0.8f));
        drawStringCentered(game.relicPlaced ? "?" : "-", cx, iconY - 12.0f, 24.0f, theme::PARCHMENT, FontStyle::Bold);
        drawSlotText(cx, y, game.relicPlaced ? "Hidden" : "None", "Relic",
                     game.relicPlaced ? theme::GOLD_BRIGHT : theme::TEXT_MUTED);
    }
}

void drawTextLegend(float x, float y)
{
    const char* const entries[] = {"#  wall",     ".  corridor", "P  you",       "E  exit",  "D  secret door",
                                   "/  open door", "k  key",     "$  treasure",  "*  gem",   "R  relic",
                                   "+  time",      "x  trap",    "G  guardian"};
    drawString("TEXT VIEW", x, y, 20.0f, theme::GOLD_BRIGHT, FontStyle::Bold);
    for (int i = 0; i < 13; ++i)
    {
        const float column = static_cast<float>(i % 3) * 140.0f;
        const float row = static_cast<float>(i / 3) * 24.0f;
        drawString(entries[i], x + column, y + 30.0f + row, 19.0f, theme::TEXT_LIGHT, FontStyle::Body);
    }
}

void drawHud(const App& app, float time)
{
    const GameState& game = app.game;
    const LocationInfo& info = getLocationInfo(game.location);
    const LocationPalette& p = getPalette(game.location);
    const DifficultySettings& settings = getDifficultySettings(game.difficulty);

    drawPanel(HUD_AREA, p.accent);
    const float x = HUD_AREA.x + 24.0f;
    const float width = HUD_AREA.width - 48.0f;

    // Where am I?
    drawLocationEmblem(game.location, {x + 22.0f, 64.0f}, 38.0f, p.accent);
    drawString(info.country, x + 54.0f, 40.0f, 40.0f, p.accent, locationFont(game.location));
    drawTab({x + width - 112.0f, 46.0f, 112.0f, 36.0f}, settings.name, true, p.accent);
    drawString(info.temple, x + 54.0f, 86.0f, 20.0f, theme::TEXT_MUTED, FontStyle::Body);
    drawOrnamentLine(x + width / 2.0f, 124.0f, width / 2.0f - 6.0f, Fade(p.accent, 0.8f));

    drawTimerCard(game, x, 142.0f, width, time);

    // Lives and the current position
    for (int i = 0; i < game.maxLives; ++i)
    {
        drawHeart({x + 16.0f + 36.0f * static_cast<float>(i), 278.0f}, 28.0f,
                  i < game.lives ? theme::HEART : Fade(theme::LOCKED, 0.7f));
    }
    drawStringRight(TextFormat("Row %d,  Column %d", game.player.row + 1, game.player.col + 1), x + width, 266.0f, 22.0f,
                    theme::TEXT_LIGHT, FontStyle::Body);

    drawInventory(game, x, 310.0f, width, time);

    // Latest event
    const Rectangle messageBox = {x, 428.0f, width, 50.0f};
    DrawRectangleRounded(messageBox, 0.3f, 6, Fade(BLACK, 0.4f));
    if (app.eventTimer > 0.0f)
    {
        drawStringCentered(app.eventMessage.c_str(), x + width / 2.0f, messageBox.y + 13.0f, 22.0f,
                           Fade(app.eventColor, std::fmin(1.0f, app.eventTimer)), FontStyle::Bold);
    }
    else
    {
        drawStringCentered(TextFormat("Moves %d      Time %s", game.moves, formatTime(game.elapsedSeconds).c_str()),
                           x + width / 2.0f, messageBox.y + 13.0f, 22.0f, theme::TEXT_MUTED, FontStyle::Body);
    }

    if (app.textView)
    {
        drawTextLegend(x, 506.0f);
    }

    const float bottom = HUD_AREA.y + HUD_AREA.height;
    drawStringCentered("Arrows - move     P - pause", x + width / 2.0f, bottom - 62.0f, 20.0f, theme::TEXT_LIGHT,
                       FontStyle::Body);
    drawStringCentered(app.textView ? "T - picture view" : "T - text view", x + width / 2.0f, bottom - 36.0f, 20.0f,
                       theme::TEXT_MUTED, FontStyle::Body);
}

// ---------------------------------------------------------------- pause menu

void runPauseMenu(App& app)
{
    const LocationPalette& p = getPalette(app.game.location);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));

    const Rectangle panel = {CENTER_X - 240.0f, static_cast<float>(SCREEN_HEIGHT) / 2.0f - 190.0f, 480.0f, 380.0f};
    drawPanel(panel, p.accent);
    drawStringCentered("PAUSED", CENTER_X, panel.y + 26.0f, 46.0f, p.accent, locationFont(app.game.location));
    drawStringCentered("The timer is stopped.", CENTER_X, panel.y + 86.0f, 21.0f, theme::TEXT_LIGHT, FontStyle::Body);

    const char* labels[] = {"Resume", "New Pyramid", "Abandon Expedition"};
    int activated = handleMenuNavigation(app.menuSelection, 3);
    for (int i = 0; i < 3; ++i)
    {
        const Rectangle button = {panel.x + 70.0f, panel.y + 136.0f + 70.0f * static_cast<float>(i), panel.width - 140.0f,
                                  54.0f};
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
        beginExpedition(app, app.lastMazeSize);
        break;
    case 2:
        changeScreen(app, Screen::MainMenu);
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------- results helpers

void drawScoreRow(const char* label, int points, float y, float left, float right)
{
    drawString(label, left, y, 22.0f, theme::TEXT_LIGHT, FontStyle::Body);
    drawStringRight(TextFormat("%d", points), right, y, 22.0f, theme::PARCHMENT, FontStyle::Bold);
}

void drawVictoryDetails(const App& app, float top, float left, float right, float time)
{
    const GameState& game = app.game;
    const ScoreBreakdown& score = app.result.score;
    const LocationPalette& p = getPalette(game.location);

    drawStarRow({CENTER_X - 62.0f, top + 142.0f}, 26.0f, app.result.stars, 3, theme::GOLD_BRIGHT);
    drawStringCentered(TextFormat("%d", score.total), CENTER_X, top + 178.0f, 56.0f, theme::GOLD_BRIGHT, FontStyle::Title);
    drawStringCentered(TextFormat("points  (x%.1f difficulty bonus)", static_cast<double>(score.multiplier)), CENTER_X,
                       top + 240.0f, 19.0f, theme::TEXT_MUTED, FontStyle::Body);

    float y = top + 280.0f;
    drawScoreRow("Route", score.route, y, left, right);
    y += 28.0f;
    drawScoreRow("Treasure and gems", score.treasure + score.gems, y, left, right);
    y += 28.0f;
    drawScoreRow("Relic", score.relic, y, left, right);
    y += 28.0f;
    drawScoreRow("Lives left", score.lives, y, left, right);
    y += 28.0f;
    drawScoreRow(TextFormat("Time left (%s)", formatTime(getRemainingTime(game)).c_str()), score.timeBonus, y, left, right);
    y += 44.0f;

    // Rewards that go to the player's profile
    const Rectangle box = {left, y, right - left, 66.0f};
    DrawRectangleRounded(box, 0.3f, 6, Fade(p.accent, 0.18f));
    drawCoinIcon({left + 34.0f, y + 33.0f}, 16.0f);
    drawString(TextFormat("+%d coins", app.result.coinsEarned), left + 62.0f, y + 18.0f, 26.0f, theme::GOLD_BRIGHT,
               FontStyle::Bold);
    if (app.result.relicKept)
    {
        drawRelicSprite(game.location, {left + 270.0f, y + 33.0f}, 40.0f, time);
        drawString(getRelicName(game.location, game.difficulty), left + 300.0f, y + 10.0f, 21.0f, theme::GOLD_BRIGHT,
                   FontStyle::Bold);
        drawString("added to your relics", left + 300.0f, y + 36.0f, 18.0f, theme::SUCCESS, FontStyle::Body);
    }
    else
    {
        drawString(game.relicPlaced ? "The relic stayed in the pyramid." : "No relic in this pyramid.", left + 250.0f,
                   y + 22.0f, 19.0f, theme::TEXT_MUTED, FontStyle::Body);
    }
}

void drawDefeatDetails(const App& app, float top, float time)
{
    const GameState& game = app.game;
    const LocationInfo& info = getLocationInfo(game.location);
    const LocationPalette& p = getPalette(game.location);

    drawGuardianSprite(game.location, {CENTER_X, top + 190.0f}, 120.0f, time);
    drawStringCentered(TextFormat("Time %s      Moves %d", formatTime(game.elapsedSeconds).c_str(), game.moves), CENTER_X,
                       top + 290.0f, 23.0f, theme::TEXT_LIGHT, FontStyle::Body);
    drawStringCentered("Rewards, coins and relics only count when you escape.", CENTER_X, top + 332.0f, 21.0f,
                       theme::TEXT_MUTED, FontStyle::Body);

    const Rectangle tipBox = {CENTER_X - 280.0f, top + 396.0f, 560.0f, 80.0f};
    DrawRectangleRounded(tipBox, 0.3f, 6, Fade(p.accent, 0.15f));
    drawStringCentered("EXPLORER'S TIP", CENTER_X, tipBox.y + 12.0f, 19.0f, p.accent, FontStyle::Bold);
    const char* tip = game.lossReason == LossReason::OutOfTime
                          ? TextFormat("Grab every %s - each one adds %d seconds!", info.timePickupName,
                                       static_cast<int>(TIME_PICKUP_SECONDS))
                          : TextFormat("%ss hide only in side halls - the main route is safe.", info.trapName);
    drawStringCentered(tip, CENTER_X, tipBox.y + 42.0f, 20.0f, theme::TEXT_LIGHT, FontStyle::Body);
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
    if (IsKeyPressed(KEY_T) && !app.paused)
    {
        app.textView = !app.textView;
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

    const LocationPalette& p = getPalette(app.game.location);
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, p.skyTop, mixColor(p.skyTop, BLACK, 0.5f));
    const MazeView view = computeMazeView(app.game.maze.getSize(), MAZE_AREA);
    if (app.textView)
    {
        drawTextMaze(app.game, view);
    }
    else
    {
        drawMazeScene(app.game, view, time, app.profile.equippedSkin);
    }
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
    const GameState& game = app.game;
    const LocationInfo& info = getLocationInfo(game.location);
    const LocationPalette& p = getPalette(game.location);
    const bool won = game.status == GameStatus::Won;

    drawLocationScene(game.location, {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)}, time);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.45f));

    const Rectangle panel = {CENTER_X - 330.0f, 40.0f, 660.0f, 720.0f};
    drawPanel(panel, won ? p.accent : theme::DANGER);
    const float top = panel.y;

    if (won)
    {
        drawStringShadowCentered("YOU ESCAPED!", CENTER_X, top + 26.0f, 58.0f, p.accent, locationFont(game.location));
        drawStringCentered(TextFormat("You walked out through the %s.", info.exitName), CENTER_X, top + 92.0f, 21.0f,
                           theme::TEXT_LIGHT, FontStyle::Body);
        drawVictoryDetails(app, top, panel.x + 80.0f, panel.x + panel.width - 80.0f, time);
    }
    else
    {
        drawStringShadowCentered("LOST IN THE PYRAMID", CENTER_X, top + 26.0f, 52.0f, theme::DANGER,
                                 locationFont(game.location));
        const char* reason = game.lossReason == LossReason::OutOfTime ? info.timeUpMessage : "You have no lives left.";
        drawStringCentered(reason, CENTER_X, top + 92.0f, 22.0f, theme::TEXT_LIGHT, FontStyle::Body);
        drawDefeatDetails(app, top, time);
    }

    const char* labels[] = {"Play Again", "New Location", "Main Menu"};
    int activated = handleMenuNavigation(app.menuSelection, 3);
    for (int i = 0; i < 3; ++i)
    {
        const Rectangle button = {panel.x + 45.0f + 195.0f * static_cast<float>(i), panel.y + panel.height - 84.0f,
                                  180.0f, 54.0f};
        if (drawButton(button, labels[i], i == app.menuSelection))
        {
            activated = i;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        activated = 2;
    }

    switch (activated)
    {
    case 0:
        beginExpedition(app, app.lastMazeSize);
        break;
    case 1:
        changeScreen(app, Screen::ChooseLocation);
        break;
    case 2:
        changeScreen(app, Screen::MainMenu);
        break;
    default:
        break;
    }
}
