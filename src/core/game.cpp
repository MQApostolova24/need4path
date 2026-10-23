#include "core/game.h"

#include "core/maze_generator.h"

#include <algorithm>
#include <cmath>

namespace
{
const DifficultySettings DIFFICULTIES[3] = {
    // name     rank             torch lives doors treasure trap   timer  base   step  mummy  speed  chase  bonus
    {"Easy",   "Explorer",       0.0f, 3,    2,    0.020f,  0.000f, false, 0.0f,  0.0f, false, 0.00f, 0,     1.0f},
    {"Medium", "Archaeologist",  6.5f, 3,    3,    0.025f,  0.015f, true,  40.0f, 1.5f, false, 0.00f, 0,     1.5f},
    {"Hard",   "Tomb Raider",    4.0f, 2,    4,    0.030f,  0.030f, true,  15.0f, 0.8f, true,  0.40f, 12,    2.2f},
};

const float INVULNERABILITY_SECONDS = 1.5f;
const int MUMMY_SPAWN_DISTANCE_FROM_EXIT = 6;
const int MUMMY_MIN_DISTANCE_FROM_START = 8;

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

// Marks every tile inside the torch light as explored.
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

// The mummy waits near the Sun Gate, but never too close to the entrance.
void setupMummy(GameState& state, const DifficultySettings& settings)
{
    state.mummy = Mummy{};
    if (!settings.hasMummy)
    {
        return;
    }

    const Maze& maze = state.maze;
    const std::vector<int> fromExit = computeDistances(maze, maze.getExit(), PathMode::MummyOnly);
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
                fromStart[index] < MUMMY_MIN_DISTANCE_FROM_START)
            {
                continue;
            }

            const int score = std::abs(fromExit[index] - MUMMY_SPAWN_DISTANCE_FROM_EXIT);
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
        state.mummy.active = true;
        state.mummy.spawn = best;
        state.mummy.pos = best;
        state.mummy.previous = best;
        state.mummy.stepTimer = settings.mummyStepSeconds;
    }
}

// The mummy hunts the player when close enough, otherwise it wanders.
void stepMummy(GameState& state, const DifficultySettings& settings)
{
    Mummy& mummy = state.mummy;
    const Maze& maze = state.maze;
    const std::vector<int> toPlayer = computeDistances(maze, state.player, PathMode::MummyOnly);
    const int currentDistance = toPlayer[maze.toIndex(mummy.pos)];

    GridPos next = mummy.pos;

    if (currentDistance != -1 && currentDistance <= settings.mummyChaseRange)
    {
        // Chase: step to the neighbour that is closer to the player.
        for (Direction direction : ALL_DIRECTIONS)
        {
            const GridPos candidate = stepInDirection(mummy.pos, direction);
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
            const GridPos candidate = stepInDirection(mummy.pos, direction);
            if (maze.isInside(candidate) && isPassable(maze.at(candidate).tile, PathMode::MummyOnly) &&
                candidate != mummy.previous)
            {
                options.push_back(candidate);
            }
        }

        if (options.empty())
        {
            next = mummy.previous;  // dead end: turn back
        }
        else
        {
            std::uniform_int_distribution<int> pick(0, static_cast<int>(options.size()) - 1);
            next = options[static_cast<std::size_t>(pick(state.rng))];
        }
    }

    mummy.previous = mummy.pos;
    mummy.pos = next;
}

GameEvent checkMummyCollision(GameState& state)
{
    Mummy& mummy = state.mummy;
    if (!mummy.active || mummy.pos != state.player || state.invulnerableSeconds > 0.0f)
    {
        return GameEvent::None;
    }

    loseLife(state);
    state.invulnerableSeconds = INVULNERABILITY_SECONDS;
    mummy.pos = mummy.spawn;
    mummy.previous = mummy.spawn;

    return state.status == GameStatus::Lost ? GameEvent::OutOfLives : GameEvent::CaughtByMummy;
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

bool startNewGame(GameState& state, int size, Difficulty difficulty, unsigned int seed)
{
    const DifficultySettings& settings = getDifficultySettings(difficulty);
    state.rng.seed(seed);

    // About half of the tiles in a carved maze are corridors.
    const float corridorEstimate = static_cast<float>(size * size) / 2.0f;

    GenerationSettings generation;
    generation.secretDoors = settings.baseSecretDoors + size / 15;
    generation.treasures = std::max(2, static_cast<int>(std::lround(corridorEstimate * settings.treasureDensity)));
    generation.traps = settings.trapDensity > 0.0f
                           ? std::max(1, static_cast<int>(std::lround(corridorEstimate * settings.trapDensity)))
                           : 0;

    GenerationResult result;
    if (!generateMaze(state.maze, size, generation, state.rng, result))
    {
        state.status = GameStatus::NotStarted;
        return false;
    }

    state.difficulty = difficulty;
    state.status = GameStatus::Playing;
    state.lossReason = LossReason::None;
    state.player = state.maze.getStart();
    state.lives = settings.lives;
    state.keys = 0;
    state.treasuresCollected = 0;
    state.totalTreasures = result.treasuresPlaced;
    state.secretDoors = result.secretDoorsPlaced;
    state.doorsOpened = 0;
    state.moves = 0;
    state.elapsedSeconds = 0.0f;
    state.invulnerableSeconds = 0.0f;
    state.optimalPathLength = shortestPathLength(state.maze, state.maze.getStart(), state.maze.getExit(),
                                                 PathMode::WalkableOnly);
    state.timeLimitSeconds = settings.hasTimeLimit
                                 ? settings.timeLimitBase +
                                       settings.timeLimitPerStep * static_cast<float>(state.optimalPathLength)
                                 : 0.0f;

    setupMummy(state, settings);
    revealAroundPlayer(state);
    return true;
}

GameEvent movePlayer(GameState& state, Direction direction)
{
    if (state.status != GameStatus::Playing || direction == Direction::None)
    {
        return GameEvent::None;
    }

    const GridPos target = stepInDirection(state.player, direction);
    if (!state.maze.isInside(target))
    {
        return GameEvent::BlockedByWall;
    }

    Cell& cell = state.maze.at(target);
    GameEvent event = GameEvent::Moved;

    if (cell.tile == TileType::Wall)
    {
        return GameEvent::BlockedByWall;
    }

    if (cell.tile == TileType::SecretDoor)
    {
        if (state.keys <= 0)
        {
            return GameEvent::DoorLocked;
        }
        state.keys--;
        state.doorsOpened++;
        cell.tile = TileType::OpenedDoor;
        event = GameEvent::DoorOpened;
    }

    state.player = target;
    state.moves++;

    switch (cell.item)
    {
    case ItemType::Ankh:
        state.keys++;
        cell.item = ItemType::None;
        event = GameEvent::AnkhCollected;
        break;
    case ItemType::Treasure:
        state.treasuresCollected++;
        cell.item = ItemType::None;
        event = GameEvent::TreasureCollected;
        break;
    case ItemType::Trap:
        cell.item = ItemType::SprungTrap;
        loseLife(state);
        event = state.status == GameStatus::Lost ? GameEvent::OutOfLives : GameEvent::TrapTriggered;
        break;
    default:
        break;
    }

    if (state.status == GameStatus::Playing && cell.tile == TileType::Exit)
    {
        state.status = GameStatus::Won;
        event = GameEvent::ReachedExit;
    }

    if (state.status == GameStatus::Playing)
    {
        const GameEvent collision = checkMummyCollision(state);
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

    const DifficultySettings& settings = getDifficultySettings(state.difficulty);
    state.elapsedSeconds += deltaSeconds;
    state.invulnerableSeconds = std::max(0.0f, state.invulnerableSeconds - deltaSeconds);

    if (settings.hasTimeLimit && state.elapsedSeconds >= state.timeLimitSeconds)
    {
        state.status = GameStatus::Lost;
        state.lossReason = LossReason::OutOfTime;
        return GameEvent::TimeUp;
    }

    if (state.mummy.active)
    {
        state.mummy.stepTimer -= deltaSeconds;
        if (state.mummy.stepTimer <= 0.0f)
        {
            state.mummy.stepTimer += settings.mummyStepSeconds;
            stepMummy(state, settings);
            return checkMummyCollision(state);
        }
    }

    return GameEvent::None;
}

bool isCellLit(const GameState& state, int row, int col)
{
    const float radius = getDifficultySettings(state.difficulty).torchRadius;
    if (radius <= 0.0f)
    {
        return true;
    }

    const float rowOffset = static_cast<float>(row - state.player.row);
    const float colOffset = static_cast<float>(col - state.player.col);
    return rowOffset * rowOffset + colOffset * colOffset <= radius * radius;
}

float getRemainingTime(const GameState& state)
{
    if (!getDifficultySettings(state.difficulty).hasTimeLimit)
    {
        return 0.0f;
    }
    return std::max(0.0f, state.timeLimitSeconds - state.elapsedSeconds);
}

int calculateScore(const GameState& state)
{
    if (state.status != GameStatus::Won)
    {
        return 0;
    }

    const DifficultySettings& settings = getDifficultySettings(state.difficulty);
    const float pathLength = static_cast<float>(state.optimalPathLength);

    // Finishing faster than the target time (e.g. by using secret doors) gives a bonus.
    const float targetSeconds = pathLength * 1.2f + 10.0f;
    const float timeBonus = std::max(0.0f, targetSeconds - state.elapsedSeconds) * 8.0f;

    const float baseScore = pathLength * 10.0f +
                            static_cast<float>(state.treasuresCollected) * 75.0f +
                            static_cast<float>(state.lives) * 100.0f +
                            timeBonus;

    return static_cast<int>(std::lround(baseScore * settings.scoreMultiplier));
}
