// QA: automated test functions for the game logic.
// They test the "core" folder directly, so no window or graphics are needed.
// Run the "lost_pyramids_tests" program - every test prints PASSED or FAILED.

#include "core/auth.h"
#include "core/config.h"
#include "core/game.h"
#include "core/leaderboard.h"
#include "core/location.h"
#include "core/maze.h"
#include "core/profile.h"
#include "core/skins.h"
#include "core/validation.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace
{
int checksPassed = 0;
int checksFailed = 0;
int testsFailed = 0;
int testsRun = 0;

const char* const TEST_USERS_FILE = "qa_test_users.txt";
const char* const TEST_SCORES_FILE = "qa_test_scores.txt";
const char* const TEST_PROFILES_FILE = "qa_test_profiles.txt";

void check(bool condition, const std::string& description)
{
    if (condition)
    {
        ++checksPassed;
    }
    else
    {
        ++checksFailed;
        std::cout << "      FAILED CHECK: " << description << '\n';
    }
}

void runTest(const char* id, const char* name, void (*test)())
{
    const int failedBefore = checksFailed;
    test();
    ++testsRun;
    const bool passed = checksFailed == failedBefore;
    if (!passed)
    {
        ++testsFailed;
    }
    std::cout << (passed ? "[PASSED] " : "[FAILED] ") << id << "  " << name << '\n';
}

// A hand-made 10x10 pyramid: one corridor along the top row, walls everywhere else.
// Tests place doors, items and the exit on it themselves.
void buildCorridorGame(GameState& state, Difficulty difficulty, Location location = Location::Egypt)
{
    state.maze.allocate(MIN_MAZE_SIZE);
    for (int col = 0; col < 7; ++col)
    {
        state.maze.at(0, col).tile = TileType::Floor;
    }
    state.location = location;
    state.difficulty = difficulty;
    state.status = GameStatus::Playing;
    state.lossReason = LossReason::None;
    state.player = GridPos{0, 0};
    state.lives = getDifficultySettings(difficulty).lives;
    state.maxLives = state.lives;
    state.keys = 0;
    state.moves = 0;
    state.guardian = Guardian{};
    state.optimalPathLength = 6;
    state.elapsedSeconds = 0.0f;
    state.timeLimitSeconds = 100.0f;
    state.initialTimeLimit = 100.0f;
}

int countTiles(const Maze& maze, TileType tile)
{
    int count = 0;
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            count += maze.at(row, col).tile == tile ? 1 : 0;
        }
    }
    return count;
}

int countItems(const Maze& maze, ItemType item)
{
    int count = 0;
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            count += maze.at(row, col).item == item ? 1 : 0;
        }
    }
    return count;
}

bool findItem(const Maze& maze, ItemType item, GridPos& found)
{
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            if (maze.at(row, col).item == item)
            {
                found = GridPos{row, col};
                return true;
            }
        }
    }
    return false;
}

// ---------------- Pyramid generation ----------------

void testMazeSizes()
{
    const int sizes[] = {10, 11, 25, 49, 50};
    for (int size : sizes)
    {
        GameState state;
        const bool started = startNewGame(state, size, Location::Egypt, Difficulty::Easy, true, 1234u);
        const std::string tag = " (size " + std::to_string(size) + ")";
        const GridPos exit = state.maze.getExit();
        check(started, "game starts" + tag);
        check(state.maze.getSize() == size, "maze has the chosen size" + tag);
        check(state.maze.at(0, 0).tile == TileType::Floor, "entrance is a corridor" + tag);
        check(state.maze.at(exit).tile == TileType::Exit, "the exit tile is marked" + tag);
        check(state.player == GridPos{0, 0}, "player starts at (0, 0)" + tag);
        check(state.optimalPathLength > 0, "exit is reachable without keys" + tag);
    }
}

void testInvalidSizesRejected()
{
    Maze maze;
    check(!maze.allocate(MIN_MAZE_SIZE - 1), "size 9 is rejected");
    check(!maze.allocate(MAX_MAZE_SIZE + 1), "size 51 is rejected");
    check(!maze.isAllocated(), "no memory is allocated for invalid sizes");

    GameState state;
    check(!startNewGame(state, 0, Location::Egypt, Difficulty::Easy, true, 1u),
          "a game cannot start with size 0");
}

void testNewMazeEveryRun()
{
    GameState first;
    GameState second;
    startNewGame(first, 21, Location::Mexico, Difficulty::Easy, true, 1u);
    startNewGame(second, 21, Location::Mexico, Difficulty::Easy, true, 2u);

    bool different = false;
    for (int row = 0; row < 21 && !different; ++row)
    {
        for (int col = 0; col < 21 && !different; ++col)
        {
            different = first.maze.at(row, col).tile != second.maze.at(row, col).tile;
        }
    }
    check(different, "different seeds create different pyramids");
}

void testCornerExit()
{
    const int sizes[] = {10, 11, 15, 30, 49, 50};
    for (int size : sizes)
    {
        GameState state;
        startNewGame(state, size, Location::Cambodia, Difficulty::Medium, true, 7u);
        const std::string tag = " (size " + std::to_string(size) + ")";
        check(state.maze.getExit() == GridPos{size - 1, size - 1}, "the exit is in the bottom-right corner" + tag);
        check(state.maze.at(size - 1, size - 1).tile == TileType::Exit, "the corner tile is the exit gate" + tag);
        check(state.optimalPathLength > 0, "the exit can be reached" + tag);
    }
}

void testGenerationQuality()
{
    for (int difficulty = 0; difficulty < DIFFICULTY_COUNT; ++difficulty)
    {
        GameState state;
        startNewGame(state, 31, Location::Egypt, static_cast<Difficulty>(difficulty), true, 4242u);
        const Maze& maze = state.maze;
        const std::string tag = std::string(" (") + difficultyName(static_cast<Difficulty>(difficulty)) + ")";

        int tiles = 0;
        int links = 0;
        int unreachable = 0;
        int deadEnds = 0;
        const std::vector<int> distances = computeDistances(maze, maze.getStart(), PathMode::IncludeSecretDoors);
        for (int row = 0; row < 31; ++row)
        {
            for (int col = 0; col < 31; ++col)
            {
                const GridPos pos{row, col};
                if (!isPassable(maze.at(pos).tile, PathMode::IncludeSecretDoors))
                {
                    continue;
                }
                ++tiles;
                const int neighbours = countOpenNeighbours(maze, pos, PathMode::IncludeSecretDoors);
                links += neighbours;
                deadEnds += neighbours == 1 ? 1 : 0;
                unreachable += distances[maze.toIndex(pos)] < 0 ? 1 : 0;
            }
        }
        const int loops = links / 2 - (tiles - 1);  // a maze without loops has exactly tiles - 1 links
        check(unreachable == 0, "every corridor can be reached" + tag);
        check(loops > 0, "the pyramid has loops (more than one way around)" + tag);
        check(deadEnds > 0, "the pyramid still has dead ends to explore" + tag);
    }

    // Open burial chambers: a 3x3 block of floor somewhere in a big pyramid
    GameState big;
    startNewGame(big, 45, Location::Mexico, Difficulty::Easy, false, 99u);
    bool chamberFound = false;
    for (int row = 0; row + 2 < 45 && !chamberFound; ++row)
    {
        for (int col = 0; col + 2 < 45 && !chamberFound; ++col)
        {
            bool open = true;
            for (int r = row; r < row + 3; ++r)
            {
                for (int c = col; c < col + 3; ++c)
                {
                    open = open && big.maze.at(r, c).tile != TileType::Wall;
                }
            }
            chamberFound = open;
        }
    }
    check(chamberFound, "big pyramids contain open chambers");

    GameState easy;
    GameState hard;
    startNewGame(easy, 31, Location::Egypt, Difficulty::Easy, false, 5u);
    startNewGame(hard, 31, Location::Egypt, Difficulty::Hard, false, 5u);
    const int easyRoute = easy.optimalPathLength;
    const int hardRoute = hard.optimalPathLength;
    check(easyRoute > 0 && hardRoute > 0, "both difficulties produce a route");
}

void testSecretDoorsAreShortcuts()
{
    GameState state;
    startNewGame(state, 41, Location::Egypt, Difficulty::Hard, true, 77u);
    const Maze& maze = state.maze;

    const int doors = countTiles(maze, TileType::SecretDoor);
    check(doors > 0, "a 41x41 pyramid has secret doors");
    check(doors == state.secretDoors, "the game knows how many doors exist");
    check(countItems(maze, ItemType::Key) == doors, "there is one key per secret door");

    const int withoutDoors = shortestPathLength(maze, maze.getStart(), maze.getExit(), PathMode::WalkableOnly);
    const int withDoors = shortestPathLength(maze, maze.getStart(), maze.getExit(), PathMode::IncludeSecretDoors);
    check(withDoors <= withoutDoors, "secret doors never make the route longer");
}

void testTrapsAvoidTrueRoute()
{
    GameState state;
    startNewGame(state, 35, Location::Mexico, Difficulty::Hard, true, 99u);
    const Maze& maze = state.maze;
    const std::vector<int> fromStart = computeDistances(maze, maze.getStart(), PathMode::WalkableOnly);
    const std::vector<int> fromExit = computeDistances(maze, maze.getExit(), PathMode::WalkableOnly);
    const int routeLength = fromStart[maze.toIndex(maze.getExit())];

    int traps = 0;
    bool trapOnRoute = false;
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            if (maze.at(row, col).item != ItemType::Trap)
            {
                continue;
            }
            ++traps;
            const std::size_t index = maze.toIndex(GridPos{row, col});
            trapOnRoute = trapOnRoute || fromStart[index] + fromExit[index] == routeLength;
        }
    }
    check(traps > 0, "Hard pyramids contain traps");
    check(!trapOnRoute, "no trap lies on the true route to the exit");
}

void testRelicPlacement()
{
    for (int location = 0; location < LOCATION_COUNT; ++location)
    {
        GameState state;
        startNewGame(state, 25, static_cast<Location>(location), Difficulty::Medium, true, 2024u);
        const Maze& maze = state.maze;
        const std::string tag = std::string(" (") + locationId(static_cast<Location>(location)) + ")";

        GridPos relic;
        check(state.relicPlaced, "a relic is placed when the roll says so" + tag);
        check(countItems(maze, ItemType::Relic) == 1, "exactly one relic" + tag);

        const std::vector<int> fromStart = computeDistances(maze, maze.getStart(), PathMode::WalkableOnly);
        const std::vector<int> fromExit = computeDistances(maze, maze.getExit(), PathMode::WalkableOnly);
        check(findItem(maze, ItemType::Relic, relic) &&
                  fromStart[maze.toIndex(relic)] + fromExit[maze.toIndex(relic)] > state.optimalPathLength,
              "the relic is off the true route" + tag);
    }

    GameState noRelic;
    startNewGame(noRelic, 25, Location::Egypt, Difficulty::Medium, false, 2024u);
    check(!noRelic.relicPlaced && countItems(noRelic.maze, ItemType::Relic) == 0,
          "most pyramids have no relic at all");
}

// ---------------- Movement and rules ----------------

void testWallsBlockMovement()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    check(movePlayer(state, Direction::Down) == GameEvent::BlockedByWall, "moving into a wall is blocked");
    check(movePlayer(state, Direction::Up) == GameEvent::BlockedByWall, "moving outside the pyramid is blocked");
    check(state.player == GridPos{0, 0}, "the player did not move");
    check(state.moves == 0, "blocked moves are not counted");
}

void testMovement()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    check(movePlayer(state, Direction::Right) == GameEvent::Moved, "moving along a corridor works");
    check(state.player == GridPos{0, 1}, "the player is one tile to the right");
    check(state.moves == 1, "the move is counted");
    check(movePlayer(state, Direction::Left) == GameEvent::Moved, "moving back works");
    check(state.player == GridPos{0, 0}, "the player is back at the start");
}

void testSecretDoorNeedsKey()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    state.maze.at(0, 1).tile = TileType::SecretDoor;

    check(movePlayer(state, Direction::Right) == GameEvent::DoorLocked, "a secret door is locked without a key");
    check(state.player == GridPos{0, 0}, "the player stays in front of the door");

    state.keys = 1;
    check(movePlayer(state, Direction::Right) == GameEvent::DoorOpened, "the key opens the door");
    check(state.keys == 0, "the key is used up");
    check(state.doorsOpened == 1, "the opened door is counted");
    check(state.maze.at(0, 1).tile == TileType::OpenedDoor, "the door stays open");
}

void testCollectKeyCoinGem()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    state.maze.at(0, 1).item = ItemType::Key;
    state.maze.at(0, 2).item = ItemType::Treasure;
    state.maze.at(0, 3).item = ItemType::Gem;

    check(movePlayer(state, Direction::Right) == GameEvent::KeyCollected, "stepping on a key collects it");
    check(state.keys == 1, "the key is in the inventory");
    check(movePlayer(state, Direction::Right) == GameEvent::TreasureCollected, "a coin is collected");
    check(movePlayer(state, Direction::Right) == GameEvent::GemCollected, "a gem is collected");
    check(state.treasuresCollected == 1 && state.gemsCollected == 1, "coins and gems are counted");
    check(state.maze.at(0, 2).item == ItemType::None, "collected items disappear");
}

void testRelicPickup()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Medium, Location::Cambodia);
    state.maze.at(0, 1).item = ItemType::Relic;
    check(!state.relicFound, "the relic is not found at the start");
    check(movePlayer(state, Direction::Right) == GameEvent::RelicCollected, "stepping on the relic picks it up");
    check(state.relicFound, "the relic is marked as found");
    check(std::strcmp(getRelicName(Location::Cambodia, Difficulty::Medium), "Apsara Statuette") == 0,
          "every location and difficulty has its own relic");
}

void testTimePickup()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Hard, Location::Mexico);
    state.maze.at(0, 1).item = ItemType::TimePickup;
    const float before = state.timeLimitSeconds;

    check(movePlayer(state, Direction::Right) == GameEvent::TimeCollected, "the time pickup is collected");
    check(state.timeLimitSeconds == before + TIME_PICKUP_SECONDS, "the timer gets extra seconds");
    check(state.timePickupsCollected == 1, "the pickup is counted");
}

void testTrapCostsLife()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Medium);
    state.maze.at(0, 1).item = ItemType::Trap;
    const int livesBefore = state.lives;

    check(movePlayer(state, Direction::Right) == GameEvent::TrapTriggered, "the trap fires");
    check(state.lives == livesBefore - 1, "the trap costs one life");
    check(state.maze.at(0, 1).item == ItemType::SprungTrap, "the trap becomes harmless");

    movePlayer(state, Direction::Left);
    movePlayer(state, Direction::Right);
    check(state.lives == livesBefore - 1, "a sprung trap does not hurt again");
}

void testLosingAllLives()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Hard);
    state.lives = 1;
    state.maze.at(0, 1).item = ItemType::Trap;

    check(movePlayer(state, Direction::Right) == GameEvent::OutOfLives, "the last life is lost");
    check(state.status == GameStatus::Lost, "the game is lost");
    check(state.lossReason == LossReason::OutOfLives, "the reason is: no lives left");
    check(movePlayer(state, Direction::Right) == GameEvent::None, "no moves after the game is over");
}

void testReachingExitWins()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    state.maze.at(0, 3).tile = TileType::Exit;
    state.maze.setExit(GridPos{0, 3});

    movePlayer(state, Direction::Right);
    movePlayer(state, Direction::Right);
    check(movePlayer(state, Direction::Right) == GameEvent::ReachedExit, "the exit gate is reached");
    check(state.status == GameStatus::Won, "the game is won");
}

void testLocationTimers()
{
    // The same seed builds the same pyramid, so only the location changes the timer.
    float limits[LOCATION_COUNT];
    for (int location = 0; location < LOCATION_COUNT; ++location)
    {
        GameState state;
        startNewGame(state, 15, static_cast<Location>(location), Difficulty::Easy, true, 5u);
        limits[location] = state.timeLimitSeconds;
        check(state.timeLimitSeconds > 0.0f, std::string("every location has a timer: ") + locationId(static_cast<Location>(location)));
    }
    check(limits[1] < limits[0], "the eclipse in Mexico gives less time than Egypt");
    check(limits[2] > limits[0], "the monsoon in Cambodia gives more time than Egypt");

    GameState state;
    startNewGame(state, 15, Location::Egypt, Difficulty::Medium, true, 5u);
    updateGame(state, 1.0f);
    check(state.status == GameStatus::Playing, "the game continues before the time is up");
    check(getTimeFraction(state) < 1.0f && getTimeFraction(state) > 0.0f, "the time fraction goes down");
    check(updateGame(state, state.timeLimitSeconds) == GameEvent::TimeUp, "the timer runs out");
    check(state.lossReason == LossReason::OutOfTime, "the reason is: out of time");
}

void testGuardian()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Hard);
    state.guardian.active = true;
    state.guardian.pos = GridPos{0, 1};
    state.guardian.spawn = GridPos{0, 6};
    const int livesBefore = state.lives;

    check(movePlayer(state, Direction::Right) == GameEvent::CaughtByGuardian, "walking into the guardian is dangerous");
    check(state.lives == livesBefore - 1, "the guardian takes a life");
    check(state.guardian.pos == GridPos{0, 6}, "the guardian returns to its lair");
    check(state.invulnerableSeconds > 0.0f, "the player gets a short protection");

    GameState hard;
    startNewGame(hard, 25, Location::Egypt, Difficulty::Hard, true, 3u);
    check(hard.guardian.active, "Hard pyramids have a guardian");
    check(hard.guardian.pos != hard.player, "the guardian does not start on the player");

    GameState easy;
    startNewGame(easy, 25, Location::Egypt, Difficulty::Easy, true, 3u);
    check(!easy.guardian.active, "Easy pyramids have no guardian");

    check(getLocationInfo(Location::Mexico).guardianStepSeconds < getLocationInfo(Location::Egypt).guardianStepSeconds,
          "the jaguar spirit is faster than the mummy");
    check(getLocationInfo(Location::Cambodia).guardianChaseRange > getLocationInfo(Location::Egypt).guardianChaseRange,
          "the naga senses the player from further away");
}

void testLighting()
{
    GameState easy;
    startNewGame(easy, 30, Location::Egypt, Difficulty::Easy, false, 8u);
    check(isCellLit(easy, 29, 29), "on Easy the whole pyramid is visible");

    GameState medium;
    startNewGame(medium, 30, Location::Egypt, Difficulty::Medium, false, 8u);
    check(isCellLit(medium, 0, 1) && !isCellLit(medium, 29, 29), "on Medium the torch lights only nearby tiles");

    // Hard: a flashlight in the facing direction, blocked by walls
    GameState hard;
    buildCorridorGame(hard, Difficulty::Hard);
    hard.facing = Direction::Right;
    check(isCellLit(hard, 0, 5), "the flashlight lights the corridor ahead");
    check(getLightLevel(hard, 0, 1) > getLightLevel(hard, 0, 6), "the beam gets weaker with distance");
    check(isCellLit(hard, 1, 1), "a wall face next to the beam is visible");
    check(!isCellLit(hard, 3, 1), "walls block the light (nothing behind a wall is lit)");

    hard.facing = Direction::Down;
    check(!isCellLit(hard, 0, 5), "turning away leaves the corridor dark");
    movePlayer(hard, Direction::Right);
    check(hard.facing == Direction::Right, "moving turns the explorer and the flashlight");
}

void testScore()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    check(calculateScore(state) == 0, "an unfinished game has no score");

    state.status = GameStatus::Won;
    state.treasuresCollected = 2;
    state.gemsCollected = 1;
    state.relicFound = true;
    state.elapsedSeconds = 40.0f;  // 60 seconds left
    const ScoreBreakdown score = calculateScoreBreakdown(state);
    check(score.route == 6 * POINTS_PER_ROUTE_TILE, "route points");
    check(score.treasure == 2 * POINTS_PER_TREASURE && score.gems == POINTS_PER_GEM, "treasure points");
    check(score.relic == POINTS_FOR_RELIC, "relic points");
    check(score.timeBonus == 60 * POINTS_PER_SECOND_LEFT, "time bonus for the seconds left");
    check(score.total == score.subtotal, "Easy multiplies by 1");

    state.difficulty = Difficulty::Hard;
    check(calculateScore(state) > score.total, "the same result scores more on Hard");

    state.status = GameStatus::Lost;
    check(calculateScore(state) == 0, "a lost game scores 0");
}

void testStars()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Medium);
    check(calculateStars(state) == 0, "no stars before escaping");

    state.status = GameStatus::Won;
    state.elapsedSeconds = 90.0f;  // only 10 seconds left
    check(calculateStars(state) == 1, "1 star for escaping");

    state.relicFound = true;
    check(calculateStars(state) == 2, "+1 star for the relic");

    state.elapsedSeconds = 20.0f;  // 80 seconds left, all lives kept
    check(calculateStars(state) == 3, "+1 star for a fast escape without losing a life");

    state.lives--;
    check(calculateStars(state) == 2, "losing a life costs the third star");

    state.relicFound = false;
    state.totalTreasures = 6;
    state.treasuresCollected = 3;
    check(calculateStars(state) == 2, "half of the treasure also earns the second star (relics are rare)");
}

// ---------------- Validation ----------------

void testUsernameValidation()
{
    check(validateUsername("Indiana_J").empty(), "a normal username is accepted");
    check(!validateUsername("").empty(), "an empty username is rejected");
    check(!validateUsername("ab").empty(), "a too short username is rejected");
    check(!validateUsername("abcdefghijklmnopq").empty(), "a too long username is rejected");
    check(!validateUsername("9lives").empty(), "a username starting with a digit is rejected");
    check(!validateUsername("bad name").empty(), "spaces are rejected");
    check(!validateUsername("semi;colon").empty(), "special characters are rejected");
}

void testPasswordValidation()
{
    check(validatePassword("scarab42").empty(), "a good password is accepted");
    check(!validatePassword("").empty(), "an empty password is rejected");
    check(!validatePassword("a1b2").empty(), "a too short password is rejected");
    check(!validatePassword("onlyletters").empty(), "a password without digits is rejected");
    check(!validatePassword("12345678").empty(), "a password without letters is rejected");
    check(!validatePassword("with space1").empty(), "a password with spaces is rejected");
}

void testMazeSizeParsing()
{
    int size = 0;
    std::string error;
    check(parseMazeSize("10", size, error) && size == 10, "10 is accepted");
    check(parseMazeSize("50", size, error) && size == 50, "50 is accepted");
    check(!parseMazeSize("9", size, error) && !error.empty(), "9 is rejected with a message");
    check(!parseMazeSize("51", size, error), "51 is rejected");
    check(!parseMazeSize("", size, error), "empty input is rejected");
    check(!parseMazeSize("abc", size, error), "letters are rejected");
    check(!parseMazeSize("-5", size, error), "negative numbers are rejected");
    check(!parseMazeSize("99999999999", size, error), "huge numbers do not crash the game");
}

// ---------------- Accounts, Hall of Fame and relics ----------------

void testSignUpAndLogin()
{
    std::remove(TEST_USERS_FILE);
    std::mt19937 rng(42u);

    check(registerUser(TEST_USERS_FILE, "Howard", "carter1922", "carter1922", rng).success,
          "a new account is created");
    check(!registerUser(TEST_USERS_FILE, "howard", "another1", "another1", rng).success,
          "the same name (any letter case) cannot be registered twice");
    check(!registerUser(TEST_USERS_FILE, "Evelyn", "nefertiti1", "nefertiti2", rng).success,
          "different passwords are rejected");
    check(!registerUser(TEST_USERS_FILE, "x", "nefertiti1", "nefertiti1", rng).success,
          "an invalid username is rejected");

    check(loginUser(TEST_USERS_FILE, "Howard", "carter1922").success, "login with the right password works");
    check(loginUser(TEST_USERS_FILE, "HOWARD", "carter1922").username == "Howard",
          "usernames are not case-sensitive at login");
    check(!loginUser(TEST_USERS_FILE, "Howard", "wrong123").success, "a wrong password is rejected");
    check(!loginUser(TEST_USERS_FILE, "Nobody", "carter1922").success, "an unknown user is rejected");
    check(!loginUser(TEST_USERS_FILE, "", "").success, "empty fields are rejected");

    std::ifstream file(TEST_USERS_FILE);
    std::stringstream content;
    content << file.rdbuf();
    check(content.str().find("carter1922") == std::string::npos, "the password is not stored as plain text");

    file.close();
    std::remove(TEST_USERS_FILE);
}

void testLeaderboard()
{
    std::remove(TEST_SCORES_FILE);

    ScoreEntry slow{"Ramses", Location::Egypt, Difficulty::Easy, 10, 500, 90.0f, 40, 1};
    ScoreEntry best{"Cleo", Location::Cambodia, Difficulty::Hard, 30, 4200, 75.5f, 120, 3};
    ScoreEntry fast{"Tut", Location::Mexico, Difficulty::Easy, 10, 500, 45.0f, 30, 2};
    check(saveScore(TEST_SCORES_FILE, slow), "a score is saved");
    saveScore(TEST_SCORES_FILE, best);
    saveScore(TEST_SCORES_FILE, fast);

    {
        std::ofstream extra(TEST_SCORES_FILE, std::ios::app);
        extra << "this;line;is;broken\n";
        extra << "Oldie;Medium;12;300;100.00;50\n";  // a line saved by an older version of the game
    }

    std::vector<ScoreEntry> entries;
    check(loadLeaderboard(TEST_SCORES_FILE, entries), "the Hall of Fame loads");
    check(entries.size() == 4, "broken lines are skipped, old lines are kept");
    check(!entries.empty() && entries[0].username == "Cleo", "the highest score is first");
    check(entries.size() == 4 && entries[1].username == "Tut", "equal scores: the faster time wins");
    check(entries.size() == 4 && entries[0].location == Location::Cambodia && entries[0].stars == 3,
          "location and stars are saved and loaded");

    check(getTopScores(entries, FILTER_ALL, static_cast<int>(Difficulty::Easy), LEADERBOARD_SIZE).size() == 2,
          "filtering by difficulty works");
    check(getTopScores(entries, static_cast<int>(Location::Egypt), FILTER_ALL, LEADERBOARD_SIZE).size() == 2,
          "filtering by location works (old lines count as Egypt)");
    check(getTopScores(entries, FILTER_ALL, FILTER_ALL, 1).size() == 1, "the list is limited");

    float bestTime = 0.0f;
    int bestStars = 0;
    check(getPersonalBest(entries, "tut", Location::Mexico, bestTime, bestStars) && bestTime == 45.0f && bestStars == 2,
          "the personal best per location is found");
    check(!getPersonalBest(entries, "Tut", Location::Egypt, bestTime, bestStars), "no best where the player never escaped");

    std::remove(TEST_SCORES_FILE);
}

void testProfileSaveLoad()
{
    std::remove(TEST_PROFILES_FILE);

    PlayerProfile lara;
    loadProfile(TEST_PROFILES_FILE, "Lara", lara);
    check(lara.coins == 0 && lara.ownsSkin(0) && lara.equippedSkin == 0, "a new player starts with 0 coins and the free skin");

    lara.coins = 420;
    lara.relicCounts[relicIndex(Location::Mexico, Difficulty::Hard)] = 2;
    lara.discoveredRelics = 1u << relicIndex(Location::Mexico, Difficulty::Hard);
    lara.ownedSkins |= 1u << 3;
    lara.equippedSkin = 3;
    lara.expeditionsWithoutRelic = 2;
    check(saveProfile(TEST_PROFILES_FILE, lara), "the profile is saved");

    PlayerProfile nate;
    loadProfile(TEST_PROFILES_FILE, "Nate", nate);
    nate.coins = 5;
    saveProfile(TEST_PROFILES_FILE, nate);

    lara.coins = 430;  // saving again replaces the old line instead of adding a new one
    saveProfile(TEST_PROFILES_FILE, lara);

    PlayerProfile loaded;
    loadProfile(TEST_PROFILES_FILE, "LARA", loaded);
    check(loaded.coins == 430, "coins are loaded back (any letter case)");
    check(loaded.relicCount(Location::Mexico, Difficulty::Hard) == 2, "relics are loaded back");
    check(loaded.hasDiscovered(Location::Mexico, Difficulty::Hard), "discovered relics are remembered");
    check(loaded.ownsSkin(3) && loaded.equippedSkin == 3, "skins are loaded back");
    check(loaded.expeditionsWithoutRelic == 2, "the relic counter is loaded back");

    PlayerProfile other;
    loadProfile(TEST_PROFILES_FILE, "Nate", other);
    check(other.coins == 5 && other.totalRelics() == 0, "other players keep their own data");

    {
        std::ofstream broken(TEST_PROFILES_FILE, std::ios::app);
        broken << "Broken;abc;0;1;0;0,0,0;0\n";
    }
    PlayerProfile damaged;
    loadProfile(TEST_PROFILES_FILE, "Broken", damaged);
    check(damaged.coins == 0, "a damaged line gives a fresh profile instead of a crash");

    std::remove(TEST_PROFILES_FILE);
}

void testRelicRarity()
{
    PlayerProfile profile;
    std::mt19937 rng(7u);
    int relics = 0;
    int longestDryStreak = 0;
    int dryStreak = 0;
    const int expeditions = 3000;
    for (int i = 0; i < expeditions; ++i)
    {
        if (rollRelicSpawn(profile, rng))
        {
            ++relics;
            dryStreak = 0;
        }
        else
        {
            longestDryStreak = std::max(longestDryStreak, ++dryStreak);
        }
    }
    const float averageGap = static_cast<float>(expeditions) / static_cast<float>(relics);
    check(averageGap > 2.5f && averageGap < 3.5f, "about one relic every 3 expeditions");
    check(longestDryStreak <= RELIC_GUARANTEED_AFTER, "never more than 3 expeditions in a row without a relic");

    PlayerProfile unlucky;
    unlucky.expeditionsWithoutRelic = RELIC_GUARANTEED_AFTER;
    check(rollRelicSpawn(unlucky, rng) && unlucky.expeditionsWithoutRelic == 0,
          "after 3 empty expeditions the next one always has a relic");
}

void testRewardsAndSelling()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Hard, Location::Egypt);
    state.treasuresCollected = 3;
    state.gemsCollected = 2;
    state.relicFound = true;
    check(coinsEarned(state) == 0, "no coins without escaping");

    state.status = GameStatus::Won;
    check(coinsEarned(state) == 3 * COINS_PER_TREASURE + 2 * COINS_PER_GEM, "treasure turns into coins on escape");

    PlayerProfile profile;
    const int earned = collectRewards(profile, state);
    check(profile.coins == earned, "the coins go to the profile");
    check(profile.relicCount(Location::Egypt, Difficulty::Hard) == 1, "the relic goes to the profile");
    check(profile.hasDiscovered(Location::Egypt, Difficulty::Hard), "the relic is marked as discovered");

    check(sellRelic(profile, Location::Egypt, Difficulty::Hard), "a relic can be sold");
    check(profile.coins == earned + relicSellPrice(Difficulty::Hard), "selling gives coins");
    check(profile.relicCount(Location::Egypt, Difficulty::Hard) == 0, "the sold relic leaves the stash");
    check(profile.hasDiscovered(Location::Egypt, Difficulty::Hard), "a sold relic stays known");
    check(!sellRelic(profile, Location::Egypt, Difficulty::Hard), "you cannot sell a relic you do not have");
    check(relicSellPrice(Difficulty::Hard) > relicSellPrice(Difficulty::Easy), "harder relics are worth more");
}

void testSkins()
{
    PlayerProfile profile;
    check(buySkin(profile, 1) == PurchaseResult::NotEnoughCoins, "a skin cannot be bought without coins");
    check(!profile.ownsSkin(1), "the skin is not given away");

    profile.coins = getSkinInfo(1).price + 10;
    check(buySkin(profile, 1) == PurchaseResult::Bought, "a skin can be bought");
    check(profile.coins == 10, "the price is paid");
    check(profile.equippedSkin == 1, "a bought skin is worn right away");
    check(buySkin(profile, 1) == PurchaseResult::AlreadyOwned, "a skin cannot be bought twice");
    check(buySkin(profile, SKIN_COUNT) == PurchaseResult::InvalidSkin, "unknown skins are rejected");

    check(equipSkin(profile, 0), "owned skins can be equipped");
    check(!equipSkin(profile, 5), "skins you do not own cannot be equipped");
    check(profile.equippedSkin == 0, "the equipped skin did not change");
}

void testLocationData()
{
    std::set<std::string> guardians;
    std::set<std::string> timers;
    for (int i = 0; i < LOCATION_COUNT; ++i)
    {
        const Location location = static_cast<Location>(i);
        Location parsed = Location::Egypt;
        check(locationFromId(locationId(location), parsed) && parsed == location, "location ids can be read back");
        guardians.insert(getLocationInfo(location).guardianName);
        timers.insert(getLocationInfo(location).timerName);
    }
    Location unused;
    check(!locationFromId("Atlantis", unused), "unknown locations are rejected");
    check(guardians.size() == 3 && timers.size() == 3, "every location has its own guardian and timer");
}

// ---------------- Dynamic memory ----------------

void testRepeatedAllocation()
{
    Maze maze;
    for (int i = 0; i < 200; ++i)
    {
        const int size = MIN_MAZE_SIZE + i % (MAX_MAZE_SIZE - MIN_MAZE_SIZE + 1);
        maze.allocate(size);
        maze.at(size - 1, size - 1).tile = TileType::Floor;  // touch the last cell
    }
    check(maze.getSize() == MIN_MAZE_SIZE + 199 % 41, "the last size is kept");
    maze.release();
    check(!maze.isAllocated() && maze.getSize() == 0, "the memory is released");
}
}  // namespace

int main()
{
    std::cout << "=== Lost Pyramids - QA tests ===\n";

    runTest("T_001", "Pyramid sizes 10x10 to 50x50", testMazeSizes);
    runTest("T_002", "Invalid pyramid sizes are rejected", testInvalidSizesRejected);
    runTest("T_003", "A new pyramid is generated every run", testNewMazeEveryRun);
    runTest("T_004", "Start top-left, exit bottom-right", testCornerExit);
    runTest("T_005", "Generation: connected, loops, dead ends, chambers", testGenerationQuality);
    runTest("T_006", "Secret doors are shortcuts with matching keys", testSecretDoorsAreShortcuts);
    runTest("T_007", "Traps stay off the true route", testTrapsAvoidTrueRoute);
    runTest("T_008", "Relic placement", testRelicPlacement);
    runTest("T_009", "Walls block movement", testWallsBlockMovement);
    runTest("T_010", "Player movement", testMovement);
    runTest("T_011", "Secret door needs a key", testSecretDoorNeedsKey);
    runTest("T_012", "Collecting keys, treasure and gems", testCollectKeyCoinGem);
    runTest("T_013", "Picking up the relic", testRelicPickup);
    runTest("T_014", "Time pickup adds seconds", testTimePickup);
    runTest("T_015", "Trap costs a life", testTrapCostsLife);
    runTest("T_016", "Losing all lives ends the game", testLosingAllLives);
    runTest("T_017", "Reaching the exit gate wins", testReachingExitWins);
    runTest("T_018", "Location timers", testLocationTimers);
    runTest("T_019", "Guardians per location", testGuardian);
    runTest("T_020", "Light: full, torch and flashlight", testLighting);
    runTest("T_021", "Score breakdown", testScore);
    runTest("T_022", "Star rating", testStars);
    runTest("T_023", "Username validation", testUsernameValidation);
    runTest("T_024", "Password validation", testPasswordValidation);
    runTest("T_025", "Pyramid size input validation", testMazeSizeParsing);
    runTest("T_026", "Sign up and log in", testSignUpAndLogin);
    runTest("T_027", "Hall of Fame save, load, sort, filter", testLeaderboard);
    runTest("T_028", "Player profile save and load", testProfileSaveLoad);
    runTest("T_029", "Relic rarity (about 1 in 3 expeditions)", testRelicRarity);
    runTest("T_030", "Coins, relic rewards and selling", testRewardsAndSelling);
    runTest("T_031", "Buying and equipping skins", testSkins);
    runTest("T_032", "Location data", testLocationData);
    runTest("T_033", "Dynamic memory is reused and released", testRepeatedAllocation);

    std::cout << "\nTests: " << testsRun - testsFailed << " / " << testsRun << " passed"
              << "   (checks: " << checksPassed << " passed, " << checksFailed << " failed)\n";
    return testsFailed == 0 ? 0 : 1;
}
