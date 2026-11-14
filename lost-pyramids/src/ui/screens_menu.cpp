// Main menu, How to Play and Hall of Fame screens.

#include "ui/screens.h"

#include "core/utils.h"
#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/maze_renderer.h"
#include "ui/sprites.h"
#include "ui/theme.h"

#include <algorithm>

namespace
{

// ---------------------------------------------------------------- How to Play helpers

void drawRulesHeading(const char* text, float x, float& y, Color color)
{
    drawString(text, x, y, 24.0f, color, FontStyle::Bold);
    y += 32.0f;
}

void drawRulesLines(const char* const lines[], int count, float x, float& y)
{
    for (int i = 0; i < count; ++i)
    {
        drawString(lines[i], x, y, 20.0f, theme::TEXT_LIGHT, FontStyle::Body);
        y += 24.0f;
    }
}

void drawElementRow(float x, float y, const char* name, const char* description, Color accent)
{
    drawString(name, x + 46.0f, y, 21.0f, accent, FontStyle::Bold);
    drawString(description, x + 214.0f, y, 20.0f, theme::TEXT_LIGHT, FontStyle::Body);
}

// The list of pyramid elements with their pictures, using the names of one location.
void drawElementList(Location location, float x, float y, float time)
{
    const LocationInfo& info = getLocationInfo(location);
    const LocationPalette& p = getPalette(location);
    const float icon = 30.0f;
    const float step = 36.0f;
    const Vector2 iconCenter = {x + 17.0f, y + 11.0f};

    auto at = [&](int index) { return Vector2{iconCenter.x, iconCenter.y + step * static_cast<float>(index)}; };
    auto rowY = [&](int index) { return y + step * static_cast<float>(index); };
    auto tile = [&](int index) { return Rectangle{x + 2.0f, rowY(index) - 4.0f, icon, icon}; };

    drawExplorerSprite(at(0), icon, time, 0, Direction::Right, false);
    drawElementRow(x, rowY(0), "Explorer", "that's you (buy new skins with coins)", p.accent);

    drawWallTile(location, tile(1), 0, 0, true);
    drawElementRow(x, rowY(1), "Wall", "solid stone, cannot be crossed", p.accent);

    drawWallTile(location, tile(2), 0, 0, false);
    drawDoorSprite(location, tile(2), time);
    drawElementRow(x, rowY(2), info.doorName, TextFormat("opens with a %s", info.keyName), p.accent);

    drawKeySprite(location, at(3), icon * 0.85f);
    drawElementRow(x, rowY(3), info.keyName, "opens one secret door (a shortcut)", p.accent);

    drawTreasureSprite(location, at(4), icon, time);
    drawElementRow(x, rowY(4), info.coinName, TextFormat("+%d points, %d coins", POINTS_PER_TREASURE, COINS_PER_TREASURE),
                   p.accent);

    drawGemSprite(location, at(5), icon, time);
    drawElementRow(x, rowY(5), info.gemName, TextFormat("+%d points, %d coins, off the main route", POINTS_PER_GEM, COINS_PER_GEM), p.accent);

    drawRelicSprite(location, at(6), icon, time);
    drawElementRow(x, rowY(6), "Relic", "rare! +500 points, sell it for coins", p.accent);

    drawTimePickupSprite(location, at(7), icon, time);
    drawElementRow(x, rowY(7), info.timePickupName, TextFormat("+%d seconds on the timer", static_cast<int>(TIME_PICKUP_SECONDS)),
                   p.accent);

    drawTrapSprite(location, at(8), icon, false);
    drawElementRow(x, rowY(8), info.trapName, "hidden in side halls, -1 life", p.accent);

    drawGuardianSprite(location, at(9), icon, time);
    drawElementRow(x, rowY(9), info.guardianName, TextFormat("Hard only - %s", info.guardianTrait), p.accent);

    drawExitSprite(location, tile(10), time);
    drawElementRow(x, rowY(10), info.exitName, "the way out - bottom-right corner", p.accent);
}

// ---------------------------------------------------------------- Hall of Fame helpers

int nextFilter(int current, int count, int delta)
{
    // FILTER_ALL (-1) is the first entry, then 0 .. count-1
    const int index = (current + 1 + delta + count + 1) % (count + 1);
    return index - 1;
}

}  // namespace

void runMainMenuScreen(App& app)
{
    drawMenuBackdrop(static_cast<float>(GetTime()));
    drawGameTitle(36.0f, TextFormat("Welcome back, explorer %s", app.currentUser.c_str()));
    drawCoinBadge(app.profile.coins, static_cast<float>(SCREEN_WIDTH) - 24.0f, 22.0f);

    const char* labels[] = {"Start Expedition", "Trading Post", "How to Play", "Hall of Fame", "Log Out", "Quit Game"};
    const int count = 6;
    int activated = handleMenuNavigation(app.menuSelection, count);

    for (int i = 0; i < count; ++i)
    {
        const Rectangle button = {SCREEN_CENTER_X - 180.0f, 212.0f + 76.0f * static_cast<float>(i), 360.0f, 58.0f};
        if (drawButton(button, labels[i], i == app.menuSelection))
        {
            activated = i;
        }
    }

    drawStringCentered("Arrow keys + Enter, or use the mouse.   Esc - quit", SCREEN_CENTER_X,
                       static_cast<float>(SCREEN_HEIGHT) - 40.0f, 20.0f, theme::PARCHMENT, FontStyle::Body);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        activated = 5;
    }

    switch (activated)
    {
    case 0:
        changeScreen(app, Screen::ChooseLocation);
        break;
    case 1:
        changeScreen(app, Screen::TradingPost);
        break;
    case 2:
        changeScreen(app, Screen::Rules);
        break;
    case 3:
        changeScreen(app, Screen::Leaderboard);
        break;
    case 4:
        saveCurrentProfile(app);
        app.currentUser.clear();
        app.loginUsername.text.clear();
        app.loginPassword.text.clear();
        app.statusMessage = "You left the expedition. See you soon!";
        app.statusIsError = false;
        changeScreen(app, Screen::Login);
        break;
    case 5:
        saveCurrentProfile(app);
        app.screen = Screen::Exit;
        break;
    default:
        break;
    }
}

void runRulesScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    const Location location = static_cast<Location>(app.rulesLocation);
    const LocationPalette& p = getPalette(location);

    drawLocationScene(location, SCREEN_AREA, time);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.55f));
    drawHeading("HOW TO PLAY", 14.0f, 44.0f, theme::GOLD_BRIGHT, FontStyle::Title);

    const Rectangle panel = {40.0f, 96.0f, 1200.0f, 616.0f};
    drawPanel(panel, p.accent);

    // ----- Left column: the rules -----
    const float leftX = 78.0f;
    float y = 120.0f;

    drawRulesHeading("THE GOAL", leftX, y, p.accent);
    const char* goal[] = {
        "Start in the top-left corner and reach the exit gate",
        "in the bottom-right corner before the timer runs out.",
        "Walls block you - secret doors open with a key.",
    };
    drawRulesLines(goal, 3, leftX, y);
    y += 14.0f;

    drawRulesHeading("CONTROLS", leftX, y, p.accent);
    const char* controls[] = {
        "Arrow keys - move (hold to keep walking)",
        "P / Esc - pause      T - text view of the maze",
    };
    drawRulesLines(controls, 2, leftX, y);
    y += 14.0f;

    drawRulesHeading("DIFFICULTY", leftX, y, p.accent);
    const char* difficulty[] = {
        "Easy - the whole pyramid is lit.",
        "Medium - torch light and hidden traps.",
        "Hard - only a flashlight, more traps and a guardian.",
    };
    drawRulesLines(difficulty, 3, leftX, y);
    y += 14.0f;

    drawRulesHeading("REWARDS", leftX, y, p.accent);
    const char* rewards[] = {
        "Escape to keep your treasure as coins.",
        "Relics are rare (about one every 3 expeditions).",
        "Sell relics and buy skins in the Trading Post.",
    };
    drawRulesLines(rewards, 3, leftX, y);

    // ----- Right column: elements of the selected location -----
    const float rightX = 640.0f;
    for (int i = 0; i < LOCATION_COUNT; ++i)
    {
        const Location tabLocation = static_cast<Location>(i);
        if (drawTab({rightX + 186.0f * static_cast<float>(i), 116.0f, 172.0f, 42.0f}, getLocationInfo(tabLocation).name,
                    i == app.rulesLocation, getPalette(tabLocation).accent))
        {
            app.rulesLocation = i;
        }
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        app.rulesLocation = (app.rulesLocation + 2) % 3;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.rulesLocation = (app.rulesLocation + 1) % 3;
    }
    drawElementList(location, rightX, 182.0f, time);

    drawStringCentered("Left / Right - switch location", rightX + 280.0f, 588.0f + 92.0f, 18.0f, theme::TEXT_MUTED,
                       FontStyle::Body);

    if (drawButton({SCREEN_CENTER_X - 150.0f, 728.0f, 300.0f, 52.0f}, "Back", true) || IsKeyPressed(KEY_ESCAPE) ||
        IsKeyPressed(KEY_ENTER))
    {
        changeScreen(app, Screen::MainMenu);
    }
}

void runLeaderboardScreen(App& app)
{
    drawMenuBackdrop(static_cast<float>(GetTime()));
    drawHeading("HALL OF FAME", 14.0f, 44.0f, theme::GOLD_BRIGHT, FontStyle::Title);
    drawStringShadowCentered("The greatest explorers who escaped alive", SCREEN_CENTER_X, 82.0f, 22.0f, theme::PARCHMENT,
                             FontStyle::Body);

    // --- Filters: location (Left / Right) and difficulty (Up / Down) ---
    if (IsKeyPressed(KEY_LEFT))
    {
        app.leaderboardLocation = nextFilter(app.leaderboardLocation, LOCATION_COUNT, -1);
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.leaderboardLocation = nextFilter(app.leaderboardLocation, LOCATION_COUNT, 1);
    }
    if (IsKeyPressed(KEY_UP))
    {
        app.leaderboardDifficulty = nextFilter(app.leaderboardDifficulty, DIFFICULTY_COUNT, -1);
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        app.leaderboardDifficulty = nextFilter(app.leaderboardDifficulty, DIFFICULTY_COUNT, 1);
    }

    const char* locationTabs[] = {"All locations", "Egypt", "Mexico", "Cambodia"};
    const char* difficultyTabs[] = {"All difficulties", "Easy", "Medium", "Hard"};
    for (int i = 0; i < 4; ++i)
    {
        const Color accent = i == 0 ? theme::GOLD_BRIGHT : getPalette(static_cast<Location>(i - 1)).accent;
        if (drawTab({SCREEN_CENTER_X - 400.0f + 200.0f * static_cast<float>(i), 120.0f, 190.0f, 40.0f}, locationTabs[i],
                    app.leaderboardLocation == i - 1, accent))
        {
            app.leaderboardLocation = i - 1;
        }
        if (drawTab({SCREEN_CENTER_X - 400.0f + 200.0f * static_cast<float>(i), 168.0f, 190.0f, 40.0f}, difficultyTabs[i],
                    app.leaderboardDifficulty == i - 1, theme::GOLD_BRIGHT))
        {
            app.leaderboardDifficulty = i - 1;
        }
    }

    // --- Table ---
    const Rectangle panel = {80.0f, 222.0f, 1120.0f, 470.0f};
    drawPanel(panel, theme::GOLD_BRIGHT);

    const float columns[] = {112.0f, 160.0f, 400.0f, 580.0f, 730.0f, 830.0f, 950.0f, 1170.0f};
    const char* headers[] = {"#", "Explorer", "Location", "Difficulty", "Size", "Time", "Stars", "Score"};
    for (int i = 0; i < 7; ++i)
    {
        drawString(headers[i], columns[i], 244.0f, 22.0f, theme::GOLD_BRIGHT, FontStyle::Bold);
    }
    drawStringRight(headers[7], columns[7], 244.0f, 22.0f, theme::GOLD_BRIGHT, FontStyle::Bold);
    DrawLineEx({100.0f, 274.0f}, {1180.0f, 274.0f}, 2.0f, theme::GOLD_DEEP);

    const std::vector<ScoreEntry> top =
        getTopScores(app.leaderboard, app.leaderboardLocation, app.leaderboardDifficulty, LEADERBOARD_SIZE);

    if (top.empty())
    {
        drawStringCentered("No expeditions recorded here yet. Be the first to escape!", SCREEN_CENTER_X, 440.0f, 24.0f,
                           theme::PARCHMENT, FontStyle::Body);
    }

    for (std::size_t i = 0; i < top.size(); ++i)
    {
        const ScoreEntry& entry = top[i];
        const float y = 286.0f + 39.0f * static_cast<float>(i);
        const bool isMe = toLowerCopy(entry.username) == toLowerCopy(app.currentUser);
        const Color color = isMe ? theme::GOLD_BRIGHT : theme::PARCHMENT;
        const LocationPalette& p = getPalette(entry.location);

        if (i % 2 == 0)
        {
            DrawRectangleRec({100.0f, y - 6.0f, 1080.0f, 36.0f}, Fade(theme::BRONZE, 0.25f));
        }

        drawString(TextFormat("%d", static_cast<int>(i) + 1), columns[0], y, 22.0f, color, FontStyle::Bold);
        drawString(entry.username.c_str(), columns[1], y, 22.0f, color, FontStyle::Bold);
        drawLocationEmblem(entry.location, {columns[2] + 12.0f, y + 12.0f}, 22.0f, p.accent);
        drawString(getLocationInfo(entry.location).name, columns[2] + 32.0f, y, 22.0f, color, FontStyle::Body);
        drawString(difficultyName(entry.difficulty), columns[3], y, 22.0f, color, FontStyle::Body);
        drawString(TextFormat("%dx%d", entry.mazeSize, entry.mazeSize), columns[4], y, 22.0f, color, FontStyle::Body);
        drawString(formatTime(entry.timeSeconds).c_str(), columns[5], y, 22.0f, color, FontStyle::Body);
        drawStarRow({columns[6] + 10.0f, y + 11.0f}, 10.0f, entry.stars, 3, theme::GOLD_BRIGHT);
        drawStringRight(TextFormat("%d", entry.score), columns[7], y, 22.0f, color, FontStyle::Bold);
    }

    drawStringCentered("Left / Right - location      Up / Down - difficulty", SCREEN_CENTER_X, 702.0f, 19.0f, theme::PARCHMENT,
                       FontStyle::Body);
    if (drawButton({SCREEN_CENTER_X - 150.0f, 732.0f, 300.0f, 52.0f}, "Back", false) || IsKeyPressed(KEY_ESCAPE) ||
        IsKeyPressed(KEY_ENTER))
    {
        changeScreen(app, Screen::MainMenu);
    }
}
