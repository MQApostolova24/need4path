#pragma once

#include "core/config.h"
#include "core/game.h"
#include "core/leaderboard.h"
#include "ui/widgets.h"

#include <random>
#include <string>
#include <vector>

enum class Screen
{
    Login,
    SignUp,
    MainMenu,
    NewGame,
    Playing,
    Results,
    Rules,
    Leaderboard,
    Exit
};

// Everything the user interface needs to remember between frames.
struct App
{
    Screen screen = Screen::Login;
    std::string currentUser;
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

    // New expedition setup
    TextField sizeField;
    std::string setupError;
    Difficulty selectedDifficulty = Difficulty::Easy;
    int lastMazeSize = DEFAULT_MAZE_SIZE;

    // The running game
    GameState game;
    bool paused = false;
    Direction heldDirection = Direction::None;
    float moveCooldown = 0.0f;
    std::string eventMessage;
    Color eventColor = WHITE;
    float eventTimer = 0.0f;
    int finalScore = 0;
    bool scoreSaved = false;

    // Hall of Fame
    int leaderboardTab = 0;  // 0 = all, 1 = Easy, 2 = Medium, 3 = Hard
    std::vector<ScoreEntry> leaderboard;
};

void initApp(App& app);
void changeScreen(App& app, Screen next);

// Generates a new tomb and switches to the game screen.
bool beginExpedition(App& app, int size, Difficulty difficulty);

// Runs (updates and draws) the screen that is currently open.
void runCurrentScreen(App& app);
