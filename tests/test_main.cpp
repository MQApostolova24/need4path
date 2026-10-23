// QA: automated test functions for the game logic.
// They test the "core" folder directly, so no window or graphics are needed.
// Run the "tomb_tests" program - every test prints PASSED or FAILED.

#include "core/auth.h"
#include "core/config.h"
#include "core/game.h"
#include "core/leaderboard.h"
#include "core/maze.h"
#include "core/maze_generator.h"
#include "core/validation.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
int checksPassed = 0;
int checksFailed = 0;
int testsFailed = 0;
int testsRun = 0;

const char* const TEST_USERS_FILE = "qa_test_users.txt";
const char* const TEST_SCORES_FILE = "qa_test_scores.txt";

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

// A hand-made 10x10 tomb: one corridor along the top row, walls everywhere else.
// Tests place doors, items and the exit on it themselves.
void buildCorridorGame(GameState& state, Difficulty difficulty)
{
    state.maze.allocate(MIN_MAZE_SIZE);
    for (int col = 0; col < 7; ++col)
    {
        state.maze.at(0, col).tile = TileType::Floor;
    }
    state.difficulty = difficulty;
    state.status = GameStatus::Playing;
    state.lossReason = LossReason::None;
    state.player = GridPos{0, 0};
    state.lives = getDifficultySettings(difficulty).lives;
    state.keys = 0;
    state.moves = 0;
    state.mummy = Mummy{};
    state.optimalPathLength = 6;
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

// ---------------- Maze generation ----------------

void testMazeSizes()
{
    const int sizes[] = {10, 11, 25, 49, 50};
    for (int size : sizes)
    {
        GameState state;
        const bool started = startNewGame(state, size, Difficulty::Easy, 1234u);
        const std::string tag = " (size " + std::to_string(size) + ")";
        check(started, "game starts" + tag);
        check(state.maze.getSize() == size, "maze has the chosen size" + tag);
        check(state.maze.at(0, 0).tile == TileType::Floor, "entrance is a corridor" + tag);
        check(state.maze.at(size - 1, size - 1).tile == TileType::Exit, "exit is in the bottom-right corner" + tag);
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
    check(!startNewGame(state, 0, Difficulty::Easy, 1u), "a game cannot start with size 0");
}

void testNewMazeEveryRun()
{
    GameState first;
    GameState second;
    startNewGame(first, 21, Difficulty::Easy, 1u);
    startNewGame(second, 21, Difficulty::Easy, 2u);

    bool different = false;
    for (int row = 0; row < 21 && !different; ++row)
    {
        for (int col = 0; col < 21 && !different; ++col)
        {
            different = first.maze.at(row, col).tile != second.maze.at(row, col).tile;
        }
    }
    check(different, "different seeds create different tombs");
}

void testSecretDoorsAreShortcuts()
{
    GameState state;
    startNewGame(state, 41, Difficulty::Hard, 77u);
    const Maze& maze = state.maze;

    const int doors = countTiles(maze, TileType::SecretDoor);
    check(doors > 0, "a 41x41 tomb has secret doors");
    check(doors == state.secretDoors, "the game knows how many doors exist");

    int ankhs = 0;
    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            ankhs += maze.at(row, col).item == ItemType::Ankh ? 1 : 0;
        }
    }
    check(ankhs == doors, "there is one Ankh key per secret door");

    const int withoutDoors = shortestPathLength(maze, maze.getStart(), maze.getExit(), PathMode::WalkableOnly);
    const int withDoors = shortestPathLength(maze, maze.getStart(), maze.getExit(), PathMode::IncludeSecretDoors);
    check(withDoors <= withoutDoors, "secret doors never make the route longer");
}

void testTrapsAvoidTrueRoute()
{
    GameState state;
    startNewGame(state, 35, Difficulty::Hard, 99u);
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
    check(traps > 0, "Hard tombs contain traps");
    check(!trapOnRoute, "no trap lies on the true route to the exit");
}

// ---------------- Movement and rules ----------------

void testWallsBlockMovement()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    check(movePlayer(state, Direction::Down) == GameEvent::BlockedByWall, "moving into a wall is blocked");
    check(movePlayer(state, Direction::Up) == GameEvent::BlockedByWall, "moving outside the tomb is blocked");
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

void testSecretDoorNeedsAnkh()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    state.maze.at(0, 1).tile = TileType::SecretDoor;

    check(movePlayer(state, Direction::Right) == GameEvent::DoorLocked, "a secret door is locked without a key");
    check(state.player == GridPos{0, 0}, "the player stays in front of the door");

    state.keys = 1;
    check(movePlayer(state, Direction::Right) == GameEvent::DoorOpened, "the Ankh opens the door");
    check(state.keys == 0, "the key is used up");
    check(state.doorsOpened == 1, "the opened door is counted");
    check(state.maze.at(0, 1).tile == TileType::OpenedDoor, "the door stays open");
}

void testCollectAnkhAndTreasure()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    state.maze.at(0, 1).item = ItemType::Ankh;
    state.maze.at(0, 2).item = ItemType::Treasure;

    check(movePlayer(state, Direction::Right) == GameEvent::AnkhCollected, "stepping on an Ankh collects it");
    check(state.keys == 1, "the key is in the inventory");
    check(movePlayer(state, Direction::Right) == GameEvent::TreasureCollected, "treasure is collected");
    check(state.treasuresCollected == 1, "the treasure is counted");
    check(state.maze.at(0, 2).item == ItemType::None, "collected items disappear");
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

    movePlayer(state, Direction::Right);
    movePlayer(state, Direction::Right);
    check(movePlayer(state, Direction::Right) == GameEvent::ReachedExit, "the Sun Gate is reached");
    check(state.status == GameStatus::Won, "the game is won");
}

void testCurseTimer()
{
    GameState state;
    startNewGame(state, 15, Difficulty::Medium, 5u);
    check(state.timeLimitSeconds > 0.0f, "Medium has a time limit");

    updateGame(state, 1.0f);
    check(state.status == GameStatus::Playing, "the game continues before the time is up");
    check(updateGame(state, state.timeLimitSeconds) == GameEvent::TimeUp, "the timer runs out");
    check(state.lossReason == LossReason::OutOfTime, "the reason is: out of time");

    GameState easy;
    startNewGame(easy, 15, Difficulty::Easy, 5u);
    updateGame(easy, 10000.0f);
    check(easy.status == GameStatus::Playing, "Easy has no time limit");
}

void testMummyCatchesPlayer()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Hard);
    state.mummy.active = true;
    state.mummy.pos = GridPos{0, 1};
    state.mummy.spawn = GridPos{0, 6};
    const int livesBefore = state.lives;

    check(movePlayer(state, Direction::Right) == GameEvent::CaughtByMummy, "walking into the mummy is dangerous");
    check(state.lives == livesBefore - 1, "the mummy takes a life");
    check(state.mummy.pos == GridPos{0, 6}, "the mummy returns to its lair");
    check(state.invulnerableSeconds > 0.0f, "the player gets a short protection");

    GameState hard;
    startNewGame(hard, 25, Difficulty::Hard, 3u);
    check(hard.mummy.active, "Hard tombs have a mummy");
    check(hard.mummy.pos != hard.player, "the mummy does not start on the player");

    GameState easy;
    startNewGame(easy, 25, Difficulty::Easy, 3u);
    check(!easy.mummy.active, "Easy tombs have no mummy");
}

void testTorchVisibility()
{
    GameState easy;
    startNewGame(easy, 30, Difficulty::Easy, 8u);
    check(isCellLit(easy, 29, 29), "on Easy the whole tomb is visible");

    GameState hard;
    startNewGame(hard, 30, Difficulty::Hard, 8u);
    check(isCellLit(hard, 0, 1), "tiles next to the explorer are lit");
    check(!isCellLit(hard, 29, 29), "far tiles are dark on Hard");
}

void testScore()
{
    GameState state;
    buildCorridorGame(state, Difficulty::Easy);
    check(calculateScore(state) == 0, "an unfinished game has no score");

    state.status = GameStatus::Won;
    const int easyScore = calculateScore(state);
    check(easyScore > 0, "a won game has a positive score");

    state.difficulty = Difficulty::Hard;
    check(calculateScore(state) > easyScore, "the same result scores more on Hard");

    state.status = GameStatus::Lost;
    check(calculateScore(state) == 0, "a lost game scores 0");
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

// ---------------- Accounts and Hall of Fame ----------------

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

    ScoreEntry slow{"Ramses", Difficulty::Easy, 10, 500, 90.0f, 40};
    ScoreEntry best{"Cleo", Difficulty::Hard, 30, 4200, 75.5f, 120};
    ScoreEntry fast{"Tut", Difficulty::Easy, 10, 500, 45.0f, 30};
    check(saveScore(TEST_SCORES_FILE, slow), "a score is saved");
    saveScore(TEST_SCORES_FILE, best);
    saveScore(TEST_SCORES_FILE, fast);

    {
        std::ofstream broken(TEST_SCORES_FILE, std::ios::app);
        broken << "this;line;is;broken\n";
    }

    std::vector<ScoreEntry> entries;
    check(loadLeaderboard(TEST_SCORES_FILE, entries), "the Hall of Fame loads");
    check(entries.size() == 3, "broken lines are skipped");
    check(!entries.empty() && entries[0].username == "Cleo", "the highest score is first");
    check(entries.size() == 3 && entries[1].username == "Tut", "equal scores: the faster time wins");

    const std::vector<ScoreEntry> easyOnly = getTopScores(entries, true, Difficulty::Easy, LEADERBOARD_SIZE);
    check(easyOnly.size() == 2, "filtering by difficulty works");
    check(getTopScores(entries, false, Difficulty::Easy, 1).size() == 1, "the list is limited");

    std::remove(TEST_SCORES_FILE);
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
    std::cout << "=== Tomb of the Forgotten Pharaoh - QA tests ===\n";

    runTest("T_001", "Tomb sizes 10x10 to 50x50", testMazeSizes);
    runTest("T_002", "Invalid tomb sizes are rejected", testInvalidSizesRejected);
    runTest("T_003", "A new tomb is generated every run", testNewMazeEveryRun);
    runTest("T_004", "Secret doors are shortcuts with matching keys", testSecretDoorsAreShortcuts);
    runTest("T_005", "Traps stay off the true route", testTrapsAvoidTrueRoute);
    runTest("T_006", "Walls block movement", testWallsBlockMovement);
    runTest("T_007", "Player movement", testMovement);
    runTest("T_008", "Secret door needs an Ankh key", testSecretDoorNeedsAnkh);
    runTest("T_009", "Collecting Ankh keys and treasure", testCollectAnkhAndTreasure);
    runTest("T_010", "Scarab trap costs a life", testTrapCostsLife);
    runTest("T_011", "Losing all lives ends the game", testLosingAllLives);
    runTest("T_012", "Reaching the Sun Gate wins", testReachingExitWins);
    runTest("T_013", "Curse timer", testCurseTimer);
    runTest("T_014", "Mummy catches the player", testMummyCatchesPlayer);
    runTest("T_015", "Torch visibility per difficulty", testTorchVisibility);
    runTest("T_016", "Score calculation", testScore);
    runTest("T_017", "Username validation", testUsernameValidation);
    runTest("T_018", "Password validation", testPasswordValidation);
    runTest("T_019", "Tomb size input validation", testMazeSizeParsing);
    runTest("T_020", "Sign up and log in", testSignUpAndLogin);
    runTest("T_021", "Hall of Fame save, load, sort, filter", testLeaderboard);
    runTest("T_022", "Dynamic memory is reused and released", testRepeatedAllocation);

    std::cout << "\nTests: " << testsRun - testsFailed << " / " << testsRun << " passed"
              << "   (checks: " << checksPassed << " passed, " << checksFailed << " failed)\n";
    return testsFailed == 0 ? 0 : 1;
}
