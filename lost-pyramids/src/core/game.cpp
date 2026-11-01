#include "core/game.h"

#include "core/config.h"
#include "core/maze_generator.h"

#include <algorithm>
#include <cmath>

namespace
{
const DifficultySettings DIFFICULTIES[3] = {
    // name     rank            light                   range lives doors treas.  gems traps   time   step  pick loops guard  bonus
    {"Easy",   "Explorer",      LightMode::FullLight,  0.0f, 3,    2,    0.020f, 2,   0.000f, 90.0f, 2.5f, 2,   0.35f, false, 1.0f},
    {"Medium", "Archaeologist", LightMode::Torch,      6.5f, 3,    3,    0.025f, 3,   0.015f, 45.0f, 1.5f, 2,   0.20f, false, 1.5f},
    {"Hard",   "Tomb Raider",   LightMode::Flashlight, 7.5f, 2,    4,    0.030f, 4,   0.030f, 25.0f, 0.9f, 3,   0.12f, true,  2.2f},
};

const float INVULNERABILITY_SECONDS = 1.5f;
const int GUARDIAN_SPAWN_DISTANCE_FROM_EXIT = 6;
const int GUARDIAN_MIN_DISTANCE_FROM_START = 8;

// Flashlight shape (Hard)
const float FLASHLIGHT_HALF_ANGLE = 0.55f;  // radians, about 31 degrees each side
const float FLASHLIGHT_AURA_RADIUS = 1.6f;  // a little light around the explorer
const float FLASHLIGHT_LIT_LEVEL = 0.15f;

const Direction ALL_DIRECTIONS[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

void loseLife(GameState& state)
{
    state.lives--;
    if (state.lives <= 0)
    {
        state.lives = 0;
        state.status = GameStatus::Lost;
        state.lossReason = LossReason::OutOfLives;
    }
}

float distanceBetween(GridPos a, GridPos b)
{
    const float rows = static_cast<float>(a.row - b.row);
    const float cols = static_cast<float>(a.col - b.col);
    return std::sqrt(rows * rows + cols * cols);
}

// Marks every tile the player can see as explored.
void revealAroundPlayer(GameState& state)
{
    const int size = state.maze.getSize();
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            if (isCellLit(state, row, col))
            {
                state.maze.at(row, col).explored = true;
            }
        }
    }
}

// The guardian waits near the exit, but never too close to the entrance.
void setupGuardian(GameState& state, const DifficultySettings& settings)
{
    state.guardian = Guardian{};
    if (!settings.hasGuardian)
    {
        return;
    }

    const Maze& maze = state.maze;
    const std::vector<int> fromExit = computeDistances(maze, maze.getExit(), PathMode::GuardianOnly);
    const std::vector<int> fromStart = computeDistances(maze, maze.getStart(), PathMode::WalkableOnly);

    bool found = false;
    int bestScore = 0;
    GridPos best;

    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            const GridPos pos{row, col};
            const std::size_t index = maze.toIndex(pos);
            if (maze.at(pos).tile != TileType::Floor || fromExit[index] < 0 ||
                fromStart[index] < GUARDIAN_MIN_DISTANCE_FROM_START)
            {
                continue;
            }

            const int score = std::abs(fromExit[index] - GUARDIAN_SPAWN_DISTANCE_FROM_EXIT);
            if (!found || score < bestScore)
            {
                found = true;
                bestScore = score;
                best = pos;
            }
        }
    }

    if (found)
    {
        state.guardian.active = true;
        state.guardian.spawn = best;
        state.guardian.pos = best;
        state.guardian.previous = best;
        state.guardian.stepTimer = getLocationInfo(state.location).guardianStepSeconds;
    }
}

// The guardian hunts the player when close enough, otherwise it wanders.
void stepGuardian(GameState& state)
{
    const LocationInfo& info = getLocationInfo(state.location);
    Guardian& guardian = state.guardian;
    const Maze& maze = state.maze;
    const std::vector<int> toPlayer = computeDistances(maze, state.player, PathMode::GuardianOnly);
    const int currentDistance = toPlayer[maze.toIndex(guardian.pos)];

    GridPos next = guardian.pos;

    if (currentDistance != -1 && currentDistance <= info.guardianChaseRange)
    {
        // Chase: step to the neighbour that is closer to the player.
        for (Direction direction : ALL_DIRECTIONS)
        {
            const GridPos candidate = stepInDirection(guardian.pos, direction);
            if (!maze.isInside(candidate))
            {
                continue;
            }
            const int distance = toPlayer[maze.toIndex(candidate)];
            if (distance != -1 && distance < currentDistance)
            {
                next = candidate;
            }
        }
    }
    else
    {
        // Wander: pick a random corridor, avoiding turning back if possible.
        std::vector<GridPos> options;
        for (Direction direction : ALL_DIRECTIONS)
        {
            const GridPos candidate = stepInDirection(guardian.pos, direction);
            if (maze.isInside(candidate) && isPassable(maze.at(candidate).tile, PathMode::GuardianOnly) &&
                candidate != guardian.previous)
            {
                options.push_back(candidate);
            }
        }

        if (options.empty())
        {
            next = guardian.previous;  // dead end: turn back
        }
        else
        {
            std::uniform_int_distribution<int> pick(0, static_cast<int>(options.size()) - 1);
            next = options[static_cast<std::size_t>(pick(state.rng))];
        }
    }

    guardian.previous = guardian.pos;
    guardian.pos = next;
}

GameEvent checkGuardianCollision(GameState& state)
{
    Guardian& guardian = state.guardian;
    if (!guardian.active || guardian.pos != state.player || state.invulnerableSeconds > 0.0f)
    {
        return GameEvent::None;
    }

    loseLife(state);
    state.invulnerableSeconds = INVULNERABILITY_SECONDS;
    guardian.pos = guardian.spawn;
    guardian.previous = guardian.spawn;

    return state.status == GameStatus::Lost ? GameEvent::OutOfLives : GameEvent::CaughtByGuardian;
}

// Picks up whatever lies on the tile the player just stepped on.
GameEvent collectItem(GameState& state, Cell& cell)
{
    switch (cell.item)
    {
    case ItemType::Key:
        state.keys++;
        cell.item = ItemType::None;
        return GameEvent::KeyCollected;
    case ItemType::Treasure:
        state.treasuresCollected++;
        cell.item = ItemType::None;
        return GameEvent::TreasureCollected;
    case ItemType::Gem:
        state.gemsCollected++;
        cell.item = ItemType::None;
        return GameEvent::GemCollected;
    case ItemType::Relic:
        state.relicFound = true;
        cell.item = ItemType::None;
        return GameEvent::RelicCollected;
    case ItemType::TimePickup:
        state.timePickupsCollected++;
        state.timeLimitSeconds += TIME_PICKUP_SECONDS;
        cell.item = ItemType::None;
        return GameEvent::TimeCollected;
    case ItemType::Trap:
        cell.item = ItemType::SprungTrap;
        loseLife(state);
        return state.status == GameStatus::Lost ? GameEvent::OutOfLives : GameEvent::TrapTriggered;
    case ItemType::SprungTrap:
    case ItemType::None:
        break;
    }
    return GameEvent::Moved;
}

// The flashlight on a corridor tile: a cone in the facing direction plus a faint glow
// around the explorer. Walls block the light.
float beamLevel(const GameState& state, int row, int col, float range)
{
    const float dy = static_cast<float>(row - state.player.row);
    const float dx = static_cast<float>(col - state.player.col);
    const float distance = std::sqrt(dx * dx + dy * dy);
    if (distance < 0.01f)
    {
        return 1.0f;
    }

    float level = 0.0f;
    if (distance < FLASHLIGHT_AURA_RADIUS)
    {
        level = 0.75f * (1.0f - distance / FLASHLIGHT_AURA_RADIUS);
    }

    const GridPos ahead = stepInDirection(GridPos{0, 0}, state.facing);
    const float cosine = (dx * static_cast<float>(ahead.col) + dy * static_cast<float>(ahead.row)) / distance;
    const float angle = std::acos(std::clamp(cosine, -1.0f, 1.0f));
    if (angle <= FLASHLIGHT_HALF_ANGLE && distance <= range)
    {
        const float byDistance = 1.0f - (distance / range) * (distance / range);
        const float byAngle = 1.0f - 0.5f * (angle / FLASHLIGHT_HALF_ANGLE) * (angle / FLASHLIGHT_HALF_ANGLE);
        level = std::max(level, byDistance * byAngle);
    }

    if (level > 0.0f && !hasLineOfSight(state.maze, state.player, GridPos{row, col}))
    {
        return 0.0f;
    }
    return level;
}

// Corridors get the beam directly. A wall is lit when a corridor tile next to it is lit
// (you see the face of the wall along the beam), so the walls of a lit corridor show up.
float flashlightLevel(const GameState& state, int row, int col, float range)
{
    const Maze& maze = state.maze;
    if (isPassable(maze.at(row, col).tile, PathMode::WalkableOnly))
    {
        return beamLevel(state, row, col, range);
    }

    float level = 0.0f;
    for (Direction direction : ALL_DIRECTIONS)
    {
        const GridPos next = stepInDirection(GridPos{row, col}, direction);
        if (maze.isInside(next) && isPassable(maze.at(next).tile, PathMode::WalkableOnly))
        {
            level = std::max(level, 0.85f * beamLevel(state, next.row, next.col, range));
        }
    }
    return level;
}
}  // namespace

const DifficultySettings& getDifficultySettings(Difficulty difficulty)
{
    const int index = std::clamp(static_cast<int>(difficulty), 0, 2);
    return DIFFICULTIES[index];
}

const char* difficultyName(Difficulty difficulty)
{
    return getDifficultySettings(difficulty).name;
}

bool difficultyFromName(const std::string& name, Difficulty& difficulty)
{
    for (int i = 0; i < 3; ++i)
    {
        if (name == DIFFICULTIES[i].name)
        {
            difficulty = static_cast<Difficulty>(i);
            return true;
        }
    }
    return false;
}

const char* getRelicName(Location location, Difficulty difficulty)
{
    const int index = std::clamp(static_cast<int>(difficulty), 0, 2);
    return getLocationInfo(location).relics[index];
}

GridPos stepInDirection(GridPos pos, Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return GridPos{pos.row - 1, pos.col};
    case Direction::Down:
        return GridPos{pos.row + 1, pos.col};
    case Direction::Left:
        return GridPos{pos.row, pos.col - 1};
    case Direction::Right:
        return GridPos{pos.row, pos.col + 1};
    case Direction::None:
        break;
    }
    return pos;
}

bool startNewGame(GameState& state, int size, Location location, Difficulty difficulty, bool withRelic,
                  unsigned int seed)
{
    const DifficultySettings& settings = getDifficultySettings(difficulty);
    const LocationInfo& info = getLocationInfo(location);
    state.rng.seed(seed);

    // About half of the tiles in a carved maze are corridors.
    const float corridorEstimate = static_cast<float>(size * size) / 2.0f;

    GenerationSettings generation;
    generation.secretDoors = settings.baseSecretDoors + size / 15;
    generation.treasures = std::max(3, static_cast<int>(std::lround(corridorEstimate * settings.treasureDensity)));
    generation.gems = settings.baseGems + size / 15;
    generation.timePickups = settings.baseTimePickups + size / 20;
    generation.traps = settings.trapDensity > 0.0f
                           ? std::max(1, static_cast<int>(std::lround(corridorEstimate * settings.trapDensity)))
                           : 0;
    generation.chambers = std::max(0, (size - 8) / 10);
    generation.loopFraction = settings.loopFraction;
    generation.relic = withRelic;

    GenerationResult result;
    if (!generateMaze(state.maze, size, generation, state.rng, result))
    {
        state.status = GameStatus::NotStarted;
        return false;
    }

    state.location = location;
    state.difficulty = difficulty;
    state.status = GameStatus::Playing;
    state.lossReason = LossReason::None;
    state.player = state.maze.getStart();
    state.facing = state.maze.at(0, 1).tile == TileType::Floor ? Direction::Right : Direction::Down;
    state.lives = settings.lives;
    state.maxLives = settings.lives;
    state.keys = 0;
    state.treasuresCollected = 0;
    state.totalTreasures = result.treasuresPlaced;
    state.gemsCollected = 0;
    state.totalGems = result.gemsPlaced;
    state.relicPlaced = result.relicPlaced;
    state.relicFound = false;
    state.timePickupsCollected = 0;
    state.totalTimePickups = result.timePickupsPlaced;
    state.secretDoors = result.secretDoorsPlaced;
    state.doorsOpened = 0;
    state.moves = 0;
    state.elapsedSeconds = 0.0f;
    state.invulnerableSeconds = 0.0f;
    state.optimalPathLength = shortestPathLength(state.maze, state.maze.getStart(), state.maze.getExit(),
                                                 PathMode::WalkableOnly);

    // Every location has its own timer (sand, eclipse, flood) with its own pace.
    state.timeLimitSeconds = (settings.timeLimitBase +
                              settings.timeLimitPerStep * static_cast<float>(state.optimalPathLength)) *
                             info.timeMultiplier;
    state.initialTimeLimit = state.timeLimitSeconds;

    setupGuardian(state, settings);
    revealAroundPlayer(state);
    return true;
}

GameEvent movePlayer(GameState& state, Direction direction)
{
    if (state.status != GameStatus::Playing || direction == Direction::None)
    {
        return GameEvent::None;
    }

    state.facing = direction;  // the explorer turns even if the way is blocked
    const GridPos target = stepInDirection(state.player, direction);
    if (!state.maze.isInside(target) || state.maze.at(target).tile == TileType::Wall)
    {
        revealAroundPlayer(state);
        return GameEvent::BlockedByWall;
    }

    Cell& cell = state.maze.at(target);
    bool openedDoor = false;

    if (cell.tile == TileType::SecretDoor)
    {
        if (state.keys <= 0)
        {
            revealAroundPlayer(state);
            return GameEvent::DoorLocked;
        }
        state.keys--;
        state.doorsOpened++;
        cell.tile = TileType::OpenedDoor;
        openedDoor = true;
    }

    state.player = target;
    state.moves++;

    GameEvent event = collectItem(state, cell);
    if (openedDoor && event == GameEvent::Moved)
    {
        event = GameEvent::DoorOpened;
    }

    if (state.status == GameStatus::Playing && cell.tile == TileType::Exit)
    {
        state.status = GameStatus::Won;
        event = GameEvent::ReachedExit;
    }

    if (state.status == GameStatus::Playing)
    {
        const GameEvent collision = checkGuardianCollision(state);
        if (collision != GameEvent::None)
        {
            event = collision;
        }
    }

    revealAroundPlayer(state);
    return event;
}

GameEvent updateGame(GameState& state, float deltaSeconds)
{
    if (state.status != GameStatus::Playing || deltaSeconds <= 0.0f)
    {
        return GameEvent::None;
    }

    state.elapsedSeconds += deltaSeconds;
    state.invulnerableSeconds = std::max(0.0f, state.invulnerableSeconds - deltaSeconds);

    if (state.elapsedSeconds >= state.timeLimitSeconds)
    {
        state.status = GameStatus::Lost;
        state.lossReason = LossReason::OutOfTime;
        return GameEvent::TimeUp;
    }

    if (state.guardian.active)
    {
        state.guardian.stepTimer -= deltaSeconds;
        if (state.guardian.stepTimer <= 0.0f)
        {
            state.guardian.stepTimer += getLocationInfo(state.location).guardianStepSeconds;
            stepGuardian(state);
            return checkGuardianCollision(state);
        }
    }

    return GameEvent::None;
}

float getLightLevel(const GameState& state, int row, int col)
{
    const DifficultySettings& settings = getDifficultySettings(state.difficulty);
    switch (settings.light)
    {
    case LightMode::FullLight:
        return 1.0f;
    case LightMode::Torch:
    {
        const float distance = distanceBetween(state.player, GridPos{row, col});
        const float inner = settings.lightRange * 0.45f;
        const float outer = settings.lightRange + 0.6f;
        const float t = std::clamp((distance - inner) / (outer - inner), 0.0f, 1.0f);
        return 1.0f - t * t * (3.0f - 2.0f * t);  // smooth edge
    }
    case LightMode::Flashlight:
        return flashlightLevel(state, row, col, settings.lightRange);
    }
    return 1.0f;
}

bool isCellLit(const GameState& state, int row, int col)
{
    const DifficultySettings& settings = getDifficultySettings(state.difficulty);
    switch (settings.light)
    {
    case LightMode::FullLight:
        return true;
    case LightMode::Torch:
        return distanceBetween(state.player, GridPos{row, col}) <= settings.lightRange;
    case LightMode::Flashlight:
        return flashlightLevel(state, row, col, settings.lightRange) >= FLASHLIGHT_LIT_LEVEL;
    }
    return true;
}

float getRemainingTime(const GameState& state)
{
    return std::max(0.0f, state.timeLimitSeconds - state.elapsedSeconds);
}

float getTimeFraction(const GameState& state)
{
    if (state.timeLimitSeconds <= 0.0f)
    {
        return 0.0f;
    }
    return std::clamp(getRemainingTime(state) / state.timeLimitSeconds, 0.0f, 1.0f);
}

ScoreBreakdown calculateScoreBreakdown(const GameState& state)
{
    ScoreBreakdown score;
    if (state.status != GameStatus::Won)
    {
        return score;
    }

    score.route = state.optimalPathLength * POINTS_PER_ROUTE_TILE;
    score.treasure = state.treasuresCollected * POINTS_PER_TREASURE;
    score.gems = state.gemsCollected * POINTS_PER_GEM;
    score.relic = state.relicFound ? POINTS_FOR_RELIC : 0;
    score.lives = state.lives * POINTS_PER_LIFE;
    score.timeBonus = static_cast<int>(getRemainingTime(state)) * POINTS_PER_SECOND_LEFT;
    score.subtotal = score.route + score.treasure + score.gems + score.relic + score.lives + score.timeBonus;
    score.multiplier = getDifficultySettings(state.difficulty).scoreMultiplier;
    score.total = static_cast<int>(std::lround(static_cast<float>(score.subtotal) * score.multiplier));
    return score;
}

int calculateScore(const GameState& state)
{
    return calculateScoreBreakdown(state).total;
}

int calculateStars(const GameState& state)
{
    if (state.status != GameStatus::Won)
    {
        return 0;
    }

    int stars = 1;
    const int collected = state.treasuresCollected + state.gemsCollected;
    const int total = state.totalTreasures + state.totalGems;
    if (state.relicFound || (total > 0 && collected * 2 >= total))
    {
        ++stars;
    }
    if (state.lives == state.maxLives && getRemainingTime(state) >= state.initialTimeLimit / 3.0f)
    {
        ++stars;
    }
    return stars;
}
