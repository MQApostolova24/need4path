#pragma once

#include "core/maze.h"

#include <random>
#include <string>

enum class Difficulty
{
    Easy = 0,
    Medium = 1,
    Hard = 2
};

enum class Direction
{
    None,
    Up,
    Down,
    Left,
    Right
};

enum class GameStatus
{
    NotStarted,
    Playing,
    Won,
    Lost
};

enum class LossReason
{
    None,
    OutOfLives,
    OutOfTime
};

// What happened as a result of a move or a game update.
// The UI turns these into messages - the logic never prints anything itself.
enum class GameEvent
{
    None,
    Moved,
    BlockedByWall,
    DoorLocked,
    DoorOpened,
    AnkhCollected,
    TreasureCollected,
    TrapTriggered,
    CaughtByMummy,
    ReachedExit,
    TimeUp,
    OutOfLives
};

// Everything that changes between Easy, Medium and Hard.
struct DifficultySettings
{
    const char* name;
    const char* rank;          // the explorer's title on this difficulty
    float torchRadius;         // 0 = the whole tomb is lit
    int lives;
    int baseSecretDoors;
    float treasureDensity;     // per corridor tile
    float trapDensity;         // per corridor tile
    bool hasTimeLimit;
    float timeLimitBase;       // seconds
    float timeLimitPerStep;    // extra seconds per tile of the shortest route
    bool hasMummy;
    float mummyStepSeconds;    // how often the mummy moves
    int mummyChaseRange;       // the mummy hunts the player within this many steps
    float scoreMultiplier;
};

struct Mummy
{
    bool active = false;
    GridPos pos;
    GridPos spawn;
    GridPos previous;
    float stepTimer = 0.0f;
};

// The complete state of one expedition.
struct GameState
{
    Maze maze;
    Difficulty difficulty = Difficulty::Easy;
    GameStatus status = GameStatus::NotStarted;
    LossReason lossReason = LossReason::None;

    GridPos player;
    Mummy mummy;

    int lives = 0;
    int keys = 0;
    int treasuresCollected = 0;
    int totalTreasures = 0;
    int secretDoors = 0;
    int doorsOpened = 0;
    int moves = 0;
    int optimalPathLength = 0;

    float elapsedSeconds = 0.0f;
    float timeLimitSeconds = 0.0f;
    float invulnerableSeconds = 0.0f;  // short protection after losing a life

    std::mt19937 rng;
};

const DifficultySettings& getDifficultySettings(Difficulty difficulty);
const char* difficultyName(Difficulty difficulty);
bool difficultyFromName(const std::string& name, Difficulty& difficulty);

// Generates a new tomb and resets all counters. Returns false for invalid sizes.
bool startNewGame(GameState& state, int size, Difficulty difficulty, unsigned int seed);

// Tries to move the player one tile. Handles walls, secret doors, items, traps and the exit.
GameEvent movePlayer(GameState& state, Direction direction);

// Advances time: the curse timer, the mummy and the short invulnerability.
GameEvent updateGame(GameState& state, float deltaSeconds);

// True if the torch currently lights this tile (always true on Easy).
bool isCellLit(const GameState& state, int row, int col);

// Seconds left on the curse timer (0 if there is no timer).
float getRemainingTime(const GameState& state);

// Final score for a won game (0 for a lost or unfinished game).
int calculateScore(const GameState& state);

// The neighbouring tile in the given direction.
GridPos stepInDirection(GridPos pos, Direction direction);
