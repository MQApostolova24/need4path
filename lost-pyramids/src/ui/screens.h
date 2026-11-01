#pragma once

#include "ui/app.h"

// One function per screen. Each one handles the input for that screen and draws it.

// screens_auth.cpp
void runLoginScreen(App& app);
void runSignUpScreen(App& app);

// screens_menu.cpp
void runMainMenuScreen(App& app);
void runRulesScreen(App& app);
void runLeaderboardScreen(App& app);

// screens_trading.cpp
void runTradingPostScreen(App& app);

// screens_expedition.cpp
void runChooseLocationScreen(App& app);
void runSetupScreen(App& app);

// screens_game.cpp
void runPlayingScreen(App& app);
void runResultsScreen(App& app);
