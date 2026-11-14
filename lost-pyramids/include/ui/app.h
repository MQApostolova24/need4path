#pragma once

#include "core/config.h"
#include "core/game.h"
#include "core/leaderboard.h"
#include "core/profile.h"
#include "ui/widgets.h"

#include <random>
#include <string>
#include <vector>

enum class Screen
{
    Login,
    SignUp,
    MainMenu,
    ChooseLocation,
    Setup,
    Playing,
    Results,
    TradingPost,
    Rules,
    Leaderboard,
    Exit
};

// What the results screen shows after an expedition.
struct ExpeditionResult
{
    ScoreBreakdown score;
    int stars = 0;
    int coinsEarned = 0;
    bool relicKept = false;  // the relic was added to the player's stash
};

// Everything the user interface needs to remember between frames.
struct App
{
    Screen screen = Screen::Login;
    std::string currentUser;
    PlayerProfile profile;  // coins, skins and relics of the logged-in player
    std::mt19937 rng;

    // Log in / Sign up
    TextField loginUsername;
    TextField loginPassword;
    TextField signUpUsername;
    TextField signUpPassword;
    TextField signUpConfirm;
    std::string statusMessage;
    bool statusIsError = false;

    // Keyboard-selected button on menus
    int menuSelection = 0;

    // Expedition setup
    Location selectedLocation = Location::Egypt;
    Difficulty selectedDifficulty = Difficulty::Easy;
    TextField sizeField;
    std::string setupError;
    int lastMazeSize = DEFAULT_MAZE_SIZE;

    // The running expedition
    GameState game;
    bool paused = false;
    bool textView = false;  // the classic text maze instead of the picture (T key)
    Direction heldDirection = Direction::None;
    float moveCooldown = 0.0f;
    std::string eventMessage;
    Color eventColor = WHITE;
    float eventTimer = 0.0f;
    ExpeditionResult result;

    // Trading Post
    int tradingTab = 0;       // 0 = sell relics, 1 = skins
    int tradingSelection = 0;
    std::string tradingMessage;
    bool tradingMessageIsError = false;

    // Hall of Fame and help
    std::vector<ScoreEntry> leaderboard;
    int leaderboardLocation = FILTER_ALL;
    int leaderboardDifficulty = FILTER_ALL;
    int rulesLocation = 0;
};

void initApp(App& app);
void changeScreen(App& app, Screen next);

// Rolls the relic chance, generates a new pyramid and switches to the game screen.
bool beginExpedition(App& app, int size);

// Saves the logged-in player's profile (coins, skins, relics).
void saveCurrentProfile(App& app);

// Runs (updates and draws) the screen that is currently open.
void runCurrentScreen(App& app);
