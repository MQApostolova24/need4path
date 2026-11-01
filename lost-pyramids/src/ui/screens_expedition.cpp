// Choosing a location and preparing the expedition (size and difficulty).

#include "ui/screens.h"

#include "core/utils.h"
#include "core/validation.h"
#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/sprites.h"
#include "ui/theme.h"

#include <algorithm>

namespace
{
const float CENTER_X = static_cast<float>(SCREEN_WIDTH) / 2.0f;
const Rectangle FULL_SCREEN = {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)};

const int SIZE_PRESETS[3] = {15, 25, 40};
const char* const SIZE_PRESET_NAMES[3] = {"Small 15", "Medium 25", "Large 40"};

// ---------------------------------------------------------------- location cards

void drawLocationCard(const App& app, Rectangle card, Location location, bool selected, float time)
{
    const LocationInfo& info = getLocationInfo(location);
    const LocationPalette& p = getPalette(location);

    if (selected)
    {
        card.y -= 10.0f;  // the chosen card lifts up
        DrawRectangleRounded({card.x - 6.0f, card.y - 6.0f, card.width + 12.0f, card.height + 12.0f}, 0.06f, 8,
                             Fade(p.accent, 0.35f));
    }

    DrawRectangleRounded(card, 0.05f, 8, theme::PANEL);
    const Rectangle picture = {card.x + 10.0f, card.y + 10.0f, card.width - 20.0f, 250.0f};
    drawLocationScene(location, picture, time);
    DrawRectangleLinesEx(picture, 2.0f, Fade(p.accent, 0.8f));
    DrawRectangleRoundedLinesEx(card, 0.05f, 8, selected ? 4.0f : 2.0f, selected ? p.accent : theme::BRONZE);

    const float centerX = card.x + card.width / 2.0f;
    drawStringShadowCentered(info.country, centerX, card.y + 272.0f, 50.0f, p.accent, locationFont(location));
    drawStringCentered(info.temple, centerX, card.y + 332.0f, 22.0f, theme::PARCHMENT, FontStyle::Bold);

    // The location's timer
    drawTimerEmblem(location, {card.x + 60.0f, card.y + 404.0f}, 46.0f, 0.6f, time);
    drawString(info.timerName, card.x + 100.0f, card.y + 384.0f, 22.0f, theme::TEXT_LIGHT, FontStyle::Bold);
    drawString(info.timeMultiplier < 1.0f ? "less time" : (info.timeMultiplier > 1.0f ? "more time" : "normal time"),
               card.x + 100.0f, card.y + 410.0f, 19.0f, theme::TEXT_MUTED, FontStyle::Body);

    // Personal best
    float bestTime = 0.0f;
    int bestStars = 0;
    if (getPersonalBest(app.leaderboard, app.currentUser, location, bestTime, bestStars))
    {
        drawString(TextFormat("Best escape  %s", formatTime(bestTime).c_str()), card.x + 34.0f, card.y + 466.0f, 21.0f,
                   theme::TEXT_LIGHT, FontStyle::Body);
        drawStarRow({card.x + card.width - 92.0f, card.y + 478.0f}, 11.0f, bestStars, 3, theme::GOLD_BRIGHT);
    }
    else
    {
        drawStringCentered("Not escaped yet", centerX, card.y + 466.0f, 21.0f, theme::TEXT_MUTED, FontStyle::Body);
    }

    if (!selected)
    {
        DrawRectangleRounded(card, 0.05f, 8, Fade(BLACK, 0.35f));
    }
}

// ---------------------------------------------------------------- setup helpers

void setMazeSize(App& app, int value)
{
    app.sizeField.text = std::to_string(std::clamp(value, MIN_MAZE_SIZE, MAX_MAZE_SIZE));
    app.setupError.clear();
}

int currentSizeValue(const App& app)
{
    return app.sizeField.text.empty() ? DEFAULT_MAZE_SIZE : std::stoi(app.sizeField.text);  // max two digits
}

const char* lightDescription(const DifficultySettings& settings)
{
    switch (settings.light)
    {
    case LightMode::FullLight:
        return "Fully lit pyramid";
    case LightMode::Torch:
        return "Torch light";
    case LightMode::Flashlight:
        return "Flashlight only";
    }
    return "";
}

void drawDifficultyCard(const App& app, Rectangle card, Difficulty difficulty, bool selected)
{
    const DifficultySettings& settings = getDifficultySettings(difficulty);
    const LocationInfo& info = getLocationInfo(app.selectedLocation);
    const LocationPalette& p = getPalette(app.selectedLocation);

    DrawRectangleRounded(card, 0.08f, 8, selected ? theme::PANEL_LIGHT : Fade(theme::PANEL, 0.85f));
    DrawRectangleRoundedLinesEx(card, 0.08f, 8, selected ? 4.0f : 2.0f, selected ? p.accent : theme::BRONZE);

    const float x = card.x + 26.0f;
    drawString(settings.name, x, card.y + 18.0f, 34.0f, selected ? p.accent : theme::PARCHMENT, FontStyle::Title);
    drawString(settings.rank, x, card.y + 60.0f, 21.0f, theme::TEXT_MUTED, FontStyle::Bold);

    const char* danger = "No traps, no guardian";
    if (settings.hasGuardian)
    {
        danger = TextFormat("Traps + the %s", info.guardianName);
    }
    else if (settings.trapDensity > 0.0f)
    {
        danger = "Hidden traps";
    }

    const char* lines[3] = {lightDescription(settings), danger,
                            TextFormat("%d lives,  score x%.1f", settings.lives, static_cast<double>(settings.scoreMultiplier))};
    for (int i = 0; i < 3; ++i)
    {
        const float y = card.y + 104.0f + 30.0f * static_cast<float>(i);
        drawDiamond({x + 5.0f, y + 11.0f}, 5.0f, p.accent);
        drawString(lines[i], x + 20.0f, y, 21.0f, theme::TEXT_LIGHT, FontStyle::Body);
    }
}

void tryBeginExpedition(App& app)
{
    int size = 0;
    if (!parseMazeSize(app.sizeField.text, size, app.setupError))
    {
        return;
    }
    if (!beginExpedition(app, size))
    {
        app.setupError = "The pyramid could not be generated. Try again.";
    }
}
}  // namespace

void runChooseLocationScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawLocationScene(static_cast<Location>(app.menuSelection), FULL_SCREEN, time);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.55f));
    drawHeading("CHOOSE YOUR DESTINATION", 22.0f, 44.0f, theme::GOLD_BRIGHT, FontStyle::Title);
    drawCoinBadge(app.profile.coins, static_cast<float>(SCREEN_WIDTH) - 24.0f, 22.0f);

    if (IsKeyPressed(KEY_LEFT))
    {
        app.menuSelection = (app.menuSelection + 2) % 3;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.menuSelection = (app.menuSelection + 1) % 3;
    }

    bool proceed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    for (int i = 0; i < 3; ++i)
    {
        const Rectangle card = {50.0f + 400.0f * static_cast<float>(i), 128.0f, 380.0f, 520.0f};
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), card))
        {
            proceed = proceed || app.menuSelection == i;  // a second click on the chosen card continues
            app.menuSelection = i;
        }
        drawLocationCard(app, card, static_cast<Location>(i), app.menuSelection == i, time);
    }

    if (drawButton({CENTER_X - 330.0f, 690.0f, 300.0f, 56.0f}, "Back", false) || IsKeyPressed(KEY_ESCAPE))
    {
        changeScreen(app, Screen::MainMenu);
        return;
    }
    if (drawButton({CENTER_X + 30.0f, 690.0f, 300.0f, 56.0f}, "Continue", true))
    {
        proceed = true;
    }
    drawStringCentered("Left / Right - choose      Enter - continue", CENTER_X, 762.0f, 20.0f, theme::PARCHMENT,
                       FontStyle::Body);

    if (proceed)
    {
        app.selectedLocation = static_cast<Location>(app.menuSelection);
        changeScreen(app, Screen::Setup);
    }
}

void runSetupScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    const Location location = app.selectedLocation;
    const LocationInfo& info = getLocationInfo(location);
    const LocationPalette& p = getPalette(location);

    drawLocationScene(location, FULL_SCREEN, time);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
    drawStringShadowCentered(info.temple, CENTER_X, 22.0f, 50.0f, p.accent, locationFont(location));
    drawStringShadowCentered("Prepare your expedition", CENTER_X, 82.0f, 24.0f, theme::PARCHMENT, FontStyle::Body);

    const Rectangle panel = {110.0f, 130.0f, 1060.0f, 520.0f};
    drawPanel(panel, p.accent);

    // --- 1. Pyramid size ---
    drawString("1.  Pyramid size", 160.0f, 158.0f, 26.0f, p.accent, FontStyle::Bold);

    const Rectangle minusButton = {160.0f, 222.0f, 54.0f, 52.0f};
    const Rectangle sizeBox = {224.0f, 222.0f, 92.0f, 52.0f};
    const Rectangle plusButton = {326.0f, 222.0f, 54.0f, 52.0f};

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
        setMazeSize(app, currentSizeValue(app) - 1);
    }
    if (drawButton(plusButton, "+", false) || IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))
    {
        setMazeSize(app, currentSizeValue(app) + 1);
    }
    for (int i = 0; i < 3; ++i)
    {
        const bool active = app.sizeField.text == std::to_string(SIZE_PRESETS[i]);
        if (drawTab({420.0f + 170.0f * static_cast<float>(i), 222.0f, 156.0f, 52.0f}, SIZE_PRESET_NAMES[i], active, p.accent))
        {
            setMazeSize(app, SIZE_PRESETS[i]);
        }
    }

    // --- 2. Difficulty ---
    drawString("2.  Difficulty", 160.0f, 300.0f, 26.0f, p.accent, FontStyle::Bold);
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
        const Rectangle card = {160.0f + 330.0f * static_cast<float>(i), 340.0f, 300.0f, 210.0f};
        const Difficulty difficulty = static_cast<Difficulty>(i);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), card))
        {
            app.selectedDifficulty = difficulty;
        }
        drawDifficultyCard(app, card, difficulty, difficulty == app.selectedDifficulty);
    }

    const char* note = app.setupError.empty()
                           ? "Relics are rare: about one every 3 expeditions. Escape with one to sell it for coins."
                           : app.setupError.c_str();
    drawStringCentered(note, CENTER_X, 574.0f, 21.0f, app.setupError.empty() ? theme::TEXT_MUTED : theme::DANGER,
                       FontStyle::Body);

    // --- Buttons ---
    bool begin = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    if (drawButton({CENTER_X - 330.0f, 676.0f, 300.0f, 56.0f}, "Back", false) || IsKeyPressed(KEY_ESCAPE))
    {
        changeScreen(app, Screen::ChooseLocation);
        return;
    }
    if (drawButton({CENTER_X + 30.0f, 676.0f, 300.0f, 56.0f}, "Begin Expedition", true))
    {
        begin = true;
    }
    drawStringCentered("Up / Down - size      Left / Right - difficulty      Enter - begin", CENTER_X, 754.0f, 20.0f,
                       theme::PARCHMENT, FontStyle::Body);

    if (begin)
    {
        tryBeginExpedition(app);
    }
}
