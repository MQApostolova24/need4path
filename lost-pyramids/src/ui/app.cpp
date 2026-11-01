#include "ui/app.h"

#include "ui/screens.h"
#include "ui/theme.h"

#include <chrono>

namespace
{
void setupField(TextField& field, const char* label, std::size_t maxLength, bool masked)
{
    field.label = label;
    field.maxLength = maxLength;
    field.masked = masked;
    field.text.clear();
    field.active = false;
}

void deactivateAllFields(App& app)
{
    TextField* fields[] = {&app.loginUsername, &app.loginPassword, &app.signUpUsername,
                           &app.signUpPassword, &app.signUpConfirm, &app.sizeField};
    for (TextField* field : fields)
    {
        field->active = false;
    }
}
}  // namespace

void initApp(App& app)
{
    // A different seed every run, so every expedition gets a new pyramid.
    std::random_device device;
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    app.rng.seed(device() ^ static_cast<unsigned int>(now));

    const std::size_t passwordLength = static_cast<std::size_t>(MAX_PASSWORD_LENGTH);
    const std::size_t usernameLength = static_cast<std::size_t>(MAX_USERNAME_LENGTH);

    setupField(app.loginUsername, "Username", usernameLength, false);
    setupField(app.loginPassword, "Password", passwordLength, true);
    setupField(app.signUpUsername, "Choose a username", usernameLength, false);
    setupField(app.signUpPassword, "Choose a password", passwordLength, true);
    setupField(app.signUpConfirm, "Repeat the password", passwordLength, true);

    setupField(app.sizeField, "Size (10 - 50)", 2, false);
    app.sizeField.digitsOnly = true;
    app.sizeField.text = std::to_string(DEFAULT_MAZE_SIZE);

    changeScreen(app, Screen::Login);
}

void changeScreen(App& app, Screen next)
{
    app.screen = next;
    app.menuSelection = 0;
    deactivateAllFields(app);

    switch (next)
    {
    case Screen::Login:
        app.loginUsername.active = app.loginUsername.text.empty();
        app.loginPassword.active = !app.loginUsername.active;
        break;
    case Screen::SignUp:
        app.signUpUsername.active = true;
        break;
    case Screen::MainMenu:
        loadProfile(PROFILES_FILE_PATH, app.currentUser, app.profile);
        break;
    case Screen::ChooseLocation:
        app.menuSelection = static_cast<int>(app.selectedLocation);
        loadLeaderboard(LEADERBOARD_FILE_PATH, app.leaderboard);
        break;
    case Screen::Setup:
        app.sizeField.active = true;
        app.setupError.clear();
        break;
    case Screen::TradingPost:
        app.tradingSelection = 0;
        app.tradingMessage.clear();
        break;
    case Screen::Leaderboard:
        loadLeaderboard(LEADERBOARD_FILE_PATH, app.leaderboard);
        break;
    default:
        break;
    }
}

void saveCurrentProfile(App& app)
{
    if (!app.currentUser.empty())
    {
        app.profile.username = app.currentUser;
        saveProfile(PROFILES_FILE_PATH, app.profile);
    }
}

bool beginExpedition(App& app, int size)
{
    // The relic roll is saved right away, so restarting cannot be used to re-roll it.
    const bool withRelic = rollRelicSpawn(app.profile, app.rng);
    saveCurrentProfile(app);

    if (!startNewGame(app.game, size, app.selectedLocation, app.selectedDifficulty, withRelic,
                      static_cast<unsigned int>(app.rng())))
    {
        return false;
    }

    const LocationInfo& info = getLocationInfo(app.selectedLocation);
    app.lastMazeSize = size;
    app.paused = false;
    app.heldDirection = Direction::None;
    app.moveCooldown = 0.0f;
    app.eventMessage = withRelic ? std::string("A relic is hidden in this pyramid!")
                                 : std::string("Reach the ") + info.exitName + " in the bottom-right corner!";
    app.eventColor = withRelic ? theme::GOLD_BRIGHT : theme::PARCHMENT;
    app.eventTimer = 5.0f;
    app.result = ExpeditionResult{};
    changeScreen(app, Screen::Playing);
    return true;
}

void runCurrentScreen(App& app)
{
    switch (app.screen)
    {
    case Screen::Login:
        runLoginScreen(app);
        break;
    case Screen::SignUp:
        runSignUpScreen(app);
        break;
    case Screen::MainMenu:
        runMainMenuScreen(app);
        break;
    case Screen::ChooseLocation:
        runChooseLocationScreen(app);
        break;
    case Screen::Setup:
        runSetupScreen(app);
        break;
    case Screen::Playing:
        runPlayingScreen(app);
        break;
    case Screen::Results:
        runResultsScreen(app);
        break;
    case Screen::TradingPost:
        runTradingPostScreen(app);
        break;
    case Screen::Rules:
        runRulesScreen(app);
        break;
    case Screen::Leaderboard:
        runLeaderboardScreen(app);
        break;
    case Screen::Exit:
        break;
    }
}
