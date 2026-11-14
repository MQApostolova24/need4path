#pragma once

#include "core/location.h"
#include "core/maze.h"

#include <random>
#include <string>

enum class Difficulty
{
    Easy = 0,
    Medium = 1,
    Hard = 2
};

// How the pyramid is lit.
enum class LightMode
{
    FullLight,  // Easy: everything is visible
    Torch,      // Medium: a circle of torch light, the layout stays faintly visible
    Flashlight  // Hard: a flashlight beam; outside it only a faint outline of the walls and the exit
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
    KeyCollected,
    TreasureCollected,
    GemCollected,
    RelicCollected,
    TimeCollected,
    TrapTriggered,
    CaughtByGuardian,
    ReachedExit,
    TimeUp,
    OutOfLives
};

// Everything that changes between Easy, Medium and Hard.
struct DifficultySettings
{
    const char* name;
    const char* rank;          // the explorer's title on this difficulty
    LightMode light;
    float lightRange;          // torch radius or flashlight range, in tiles
    int lives;
    int baseSecretDoors;
    float treasureDensity;     // treasures per corridor tile
    int baseGems;
    float trapDensity;         // traps per corridor tile
    float timeLimitBase;       // seconds
    float timeLimitPerStep;    // extra seconds per tile of the shortest route
    int baseTimePickups;
    float loopFraction;        // more loops = more ways around (easier)
    bool hasGuardian;
    float scoreMultiplier;
};

// The creature that hunts the player on Hard (mummy, jaguar spirit or naga).
struct Guardian
{
    bool active = false;
    GridPos pos;
    GridPos spawn;
    GridPos previous;
    float stepTimer = 0.0f;
};

// How the final score was put together (shown on the results screen).
struct ScoreBreakdown
{
    int route = 0;
    int treasure = 0;
    int gems = 0;
    int relic = 0;
    int lives = 0;
    int timeBonus = 0;
    int subtotal = 0;
    float multiplier = 1.0f;
    int total = 0;
};

// The complete state of one expedition.
struct GameState
{
    Maze maze;
    Location location = Location::Egypt;
    Difficulty difficulty = Difficulty::Easy;
    GameStatus status = GameStatus::NotStarted;
    LossReason lossReason = LossReason::None;

    GridPos player;
    Direction facing = Direction::Right;  // where the flashlight points
    Guardian guardian;

    int lives = 0;
    int maxLives = 0;
    int keys = 0;
    int treasuresCollected = 0;
    int totalTreasures = 0;
    int gemsCollected = 0;
    int totalGems = 0;
    bool relicPlaced = false;
    bool relicFound = false;
    int moves = 0;
    int optimalPathLength = 0;

    float elapsedSeconds = 0.0f;
    float timeLimitSeconds = 0.0f;    // grows when time pickups are collected
    float initialTimeLimit = 0.0f;
    float invulnerableSeconds = 0.0f; // short protection after losing a life

    std::mt19937 rng;
};

const DifficultySettings& getDifficultySettings(Difficulty difficulty);
const char* difficultyName(Difficulty difficulty);
bool difficultyFromName(const std::string& name, Difficulty& difficulty);

// The relic of a location and difficulty.
const char* getRelicName(Location location, Difficulty difficulty);

// Generates a new pyramid and resets all counters. "withRelic" decides if this
// pyramid hides a relic (see rollRelicSpawn in profile.h). Returns false for invalid sizes.
bool startNewGame(GameState& state, int size, Location location, Difficulty difficulty, bool withRelic,
                  unsigned int seed);

// Tries to move the player one tile. Handles walls, secret doors, rewards, traps and the exit.
// The player also turns to face the direction, even when the way is blocked.
GameEvent movePlayer(GameState& state, Direction direction);

// Advances time: the location timer, the guardian and the short invulnerability.
// One call never advances more than 0.1 s, so a frozen window cannot make the guardian sprint.
GameEvent updateGame(GameState& state, float deltaSeconds);

// How much light reaches a tile, from 0 (dark) to 1 (fully lit).
float getLightLevel(const GameState& state, int row, int col);

// True if the player can see this tile right now.
bool isCellLit(const GameState& state, int row, int col);

// Seconds left on the location timer.
float getRemainingTime(const GameState& state);

// Remaining time as a value from 0 (time is up) to 1 (full).
float getTimeFraction(const GameState& state);

// Final score with all its parts (all zero for a lost or unfinished game).
ScoreBreakdown calculateScoreBreakdown(const GameState& state);
int calculateScore(const GameState& state);

// 0 stars = not escaped. 1 = escaped. +1 = found the relic OR half of the treasure.
// +1 = no life lost and at least a third of the starting time left.
int calculateStars(const GameState& state);

