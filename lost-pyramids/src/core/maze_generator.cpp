#include "core/maze_generator.h"

#include <algorithm>
#include <cstdlib>

namespace
{
// Corridors are carved two tiles at a time so walls stay between them.
const int CARVE_ROW[4] = {-2, 2, 0, 0};
const int CARVE_COL[4] = {0, 0, -2, 2};
const int NEIGHBOUR_ROW[4] = {-1, 1, 0, 0};
const int NEIGHBOUR_COL[4] = {0, 0, -1, 1};

const float NEWEST_CELL_CHANCE = 0.75f; // growing tree: 75% long corridors, 25% branching
const int MIN_DOOR_SPACING = 5;         // secret doors are not placed next to each other
const int MIN_TRAP_DISTANCE = 4;        // no traps right at the entrance
const float RELIC_DETOUR_RATIO = 0.7f;

struct Room
{
    int top;
    int left;
    int height;
    int width;
};

bool chance(std::mt19937& rng, float probability)
{
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    return roll(rng) < probability;
}

int randomIndex(std::mt19937& rng, std::size_t count)
{
    std::uniform_int_distribution<int> pick(0, static_cast<int>(count) - 1);
    return pick(rng);
}

// ---------------------------------------------------------------- corridors

// "Growing tree": keep a list of cells that can still grow. Taking the newest cell
// makes long winding corridors; taking a random one makes many short branches.
void carveGrowingTree(Maze& maze, std::mt19937& rng)
{
    std::vector<GridPos> active;
    active.push_back(GridPos{0, 0});
    maze.at(0, 0).tile = TileType::Floor;

    int order[4] = {0, 1, 2, 3};

    while (!active.empty())
    {
        const int index = chance(rng, NEWEST_CELL_CHANCE) ? static_cast<int>(active.size()) - 1
                                                           : randomIndex(rng, active.size());
        const GridPos current = active[static_cast<std::size_t>(index)];
        std::shuffle(order, order + 4, rng);

        bool carved = false;
        for (int direction : order)
        {
            const GridPos next{current.row + CARVE_ROW[direction], current.col + CARVE_COL[direction]};
            if (!maze.isInside(next) || maze.at(next).tile != TileType::Wall)
            {
                continue;
            }

            maze.at(current.row + CARVE_ROW[direction] / 2, current.col + CARVE_COL[direction] / 2).tile = TileType::Floor;
            maze.at(next).tile = TileType::Floor;
            active.push_back(next);
            carved = true;
            break;
        }

        if (!carved)
        {
            active.erase(active.begin() + index);
        }
    }
}

// With even sizes the carving never reaches the last row and column.
// Open nooks and short passages there so that edge is part of the pyramid too.
void openEvenEdges(Maze& maze, std::mt19937& rng)
{
    if (maze.getSize() % 2 != 0)
    {
        return;
    }

    const int last = maze.getSize() - 1;
    for (int i = 2; i < last; i += 2)
    {
        if (chance(rng, 0.45f))
        {
            maze.at(i, last).tile = TileType::Floor;
        }
        if (chance(rng, 0.45f))
        {
            maze.at(last, i).tile = TileType::Floor;
        }
    }
    // Join neighbouring nooks into short passages along the edge.
    for (int i = 3; i < last - 1; i += 2)
    {
        if (maze.at(i - 1, last).tile == TileType::Floor && maze.at(i + 1, last).tile == TileType::Floor && chance(rng, 0.5f))
        {
            maze.at(i, last).tile = TileType::Floor;
        }
        if (maze.at(last, i - 1).tile == TileType::Floor && maze.at(last, i + 1).tile == TileType::Floor && chance(rng, 0.5f))
        {
            maze.at(last, i).tile = TileType::Floor;
        }
    }
}

// ---------------------------------------------------------------- chambers and loops

bool roomsOverlap(const Room& a, const Room& b)
{
    // One tile of margin so two chambers never merge into one.
    return a.left - 1 < b.left + b.width && b.left - 1 < a.left + a.width && a.top - 1 < b.top + b.height &&
           b.top - 1 < a.top + a.height;
}

// Opens a few rectangular burial chambers. Their corners sit on carved cells,
// so every chamber is already connected to the corridors around it.
int carveChambers(Maze& maze, int count, std::mt19937& rng)
{
    const int size = maze.getSize();
    const int lastCell = (size % 2 == 0) ? size - 2 : size - 1;
    std::vector<Room> rooms;

    for (int attempt = 0; attempt < count * 20 && static_cast<int>(rooms.size()) < count; ++attempt)
    {
        const int height = chance(rng, 0.5f) ? 3 : 5;
        const int width = chance(rng, 0.5f) ? 3 : 5;
        const int maxTop = (lastCell - (height - 1)) / 2;
        const int maxLeft = (lastCell - (width - 1)) / 2;
        if (maxTop < 1 || maxLeft < 1)
        {
            break;
        }

        std::uniform_int_distribution<int> pickTop(1, maxTop);
        std::uniform_int_distribution<int> pickLeft(1, maxLeft);
        const Room room{pickTop(rng) * 2, pickLeft(rng) * 2, height, width};

        if (room.top + room.left < 6)
        {
            continue;  // keep the entrance a normal corridor
        }
        bool free = true;
        for (const Room& other : rooms)
        {
            free = free && !roomsOverlap(room, other);
        }
        if (!free)
        {
            continue;
        }

        for (int row = room.top; row < room.top + room.height; ++row)
        {
            for (int col = room.left; col < room.left + room.width; ++col)
            {
                maze.at(row, col).tile = TileType::Floor;
            }
        }
        rooms.push_back(room);
    }
    return static_cast<int>(rooms.size());
}

// Gives some dead ends a second way out, so the pyramid has loops
// (more than one route, and space to escape from the guardian).
int addLoops(Maze& maze, float fraction, std::mt19937& rng)
{
    if (fraction <= 0.0f)
    {
        return 0;
    }

    std::vector<GridPos> deadEnds;
    for (int row = 0; row < maze.getSize(); row += 2)
    {
        for (int col = 0; col < maze.getSize(); col += 2)
        {
            const GridPos pos{row, col};
            if (pos != maze.getStart() && maze.at(pos).tile == TileType::Floor &&
                countOpenNeighbours(maze, pos, PathMode::WalkableOnly) == 1)
            {
                deadEnds.push_back(pos);
            }
        }
    }
    std::shuffle(deadEnds.begin(), deadEnds.end(), rng);

    const int wanted = static_cast<int>(static_cast<float>(deadEnds.size()) * fraction);
    int added = 0;
    int order[4] = {0, 1, 2, 3};

    for (const GridPos& end : deadEnds)
    {
        if (added >= wanted)
        {
            break;
        }
        if (countOpenNeighbours(maze, end, PathMode::WalkableOnly) != 1)
        {
            continue;  // already opened by an earlier loop
        }

        std::shuffle(order, order + 4, rng);
        for (int direction : order)
        {
            const GridPos wall{end.row + NEIGHBOUR_ROW[direction], end.col + NEIGHBOUR_COL[direction]};
            const GridPos beyond{end.row + CARVE_ROW[direction], end.col + CARVE_COL[direction]};
            if (maze.isInside(beyond) && maze.at(wall).tile == TileType::Wall && maze.at(beyond).tile == TileType::Floor)
            {
                maze.at(wall).tile = TileType::Floor;
                ++added;
                break;
            }
        }
    }
    return added;
}

// ---------------------------------------------------------------- the exit

// The exit gate is always in the bottom-right corner (assignment requirement 3).
// With even sizes the corner is outside the carved grid, so a short passage links it.
void placeCornerExit(Maze& maze, GenerationResult& result)
{
    const int last = maze.getSize() - 1;
    if (maze.getSize() % 2 == 0)
    {
        maze.at(last - 1, last).tile = TileType::Floor;
    }
    maze.at(last, last).tile = TileType::Exit;
    maze.setExit(GridPos{last, last});
    result.exit = maze.getExit();
}

// ---------------------------------------------------------------- secret doors

bool isFarFromAll(const std::vector<GridPos>& chosen, GridPos candidate, int minimumDistance)
{
    for (const GridPos& other : chosen)
    {
        const int distance = std::abs(other.row - candidate.row) + std::abs(other.col - candidate.col);
        if (distance < minimumDistance)
        {
            return false;
        }
    }
    return true;
}

// Turns walls that separate two corridors which are far apart along the maze into
// secret doors - every door is a real shortcut.
int placeSecretDoors(Maze& maze, int count, std::mt19937& rng)
{
    if (count <= 0)
    {
        return 0;
    }

    const int size = maze.getSize();
    const std::vector<int> distances = computeDistances(maze, maze.getStart(), PathMode::WalkableOnly);
    const int minimumGain = std::max(6, size / 2);

    std::vector<GridPos> candidates;
    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            if (maze.at(row, col).tile != TileType::Wall)
            {
                continue;
            }

            int gain = 0;
            const GridPos pairs[2][2] = {{{row - 1, col}, {row + 1, col}}, {{row, col - 1}, {row, col + 1}}};
            for (const auto& pair : pairs)
            {
                if (!maze.isInside(pair[0]) || !maze.isInside(pair[1]))
                {
                    continue;
                }
                const int first = distances[maze.toIndex(pair[0])];
                const int second = distances[maze.toIndex(pair[1])];
                if (first >= 0 && second >= 0)
                {
                    gain = std::max(gain, std::abs(first - second) - 2);
                }
            }

            if (gain >= minimumGain)
            {
                candidates.push_back(GridPos{row, col});
            }
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng);

    std::vector<GridPos> chosen;
    for (const GridPos& candidate : candidates)
    {
        if (static_cast<int>(chosen.size()) >= count)
        {
            break;
        }
        if (isFarFromAll(chosen, candidate, MIN_DOOR_SPACING))
        {
            maze.at(candidate).tile = TileType::SecretDoor;
            chosen.push_back(candidate);
        }
    }
    return static_cast<int>(chosen.size());
}

// ---------------------------------------------------------------- items

int placeOnFreeTiles(Maze& maze, const std::vector<GridPos>& tiles, int count, ItemType item)
{
    int placed = 0;
    for (const GridPos& pos : tiles)
    {
        if (placed >= count)
        {
            break;
        }
        Cell& cell = maze.at(pos);
        if (cell.item == ItemType::None)
        {
            cell.item = item;
            ++placed;
        }
    }
    return placed;
}

// The relic waits at the end of the longest detour from the true route
// (a dead end if there is one far enough, otherwise the farthest tile off the route).
bool placeRelic(Maze& maze, const std::vector<GridPos>& tiles, const std::vector<int>& fromStart,
                const std::vector<int>& fromExit, int routeLength, std::mt19937& rng)
{
    int longestDetour = 0;
    for (const GridPos& pos : tiles)
    {
        const std::size_t index = maze.toIndex(pos);
        longestDetour = std::max(longestDetour, fromStart[index] + fromExit[index] - routeLength);
    }
    if (longestDetour <= 0)
    {
        return false;
    }

    std::vector<GridPos> deadEnds;
    std::vector<GridPos> farTiles;
    const int minimumDetour = static_cast<int>(static_cast<float>(longestDetour) * RELIC_DETOUR_RATIO);
    for (const GridPos& pos : tiles)
    {
        const std::size_t index = maze.toIndex(pos);
        const int detour = fromStart[index] + fromExit[index] - routeLength;
        if (detour < std::max(1, minimumDetour))
        {
            continue;
        }
        farTiles.push_back(pos);
        if (countOpenNeighbours(maze, pos, PathMode::WalkableOnly) == 1)
        {
            deadEnds.push_back(pos);
        }
    }

    const std::vector<GridPos>& pool = deadEnds.empty() ? farTiles : deadEnds;
    if (pool.empty())
    {
        return false;
    }
    maze.at(pool[static_cast<std::size_t>(randomIndex(rng, pool.size()))]).item = ItemType::Relic;
    return true;
}

void placeItems(Maze& maze, const GenerationSettings& settings, int keyCount, std::mt19937& rng,
                GenerationResult& result)
{
    const GridPos start = maze.getStart();
    const GridPos exit = maze.getExit();
    const std::vector<int> fromStart = computeDistances(maze, start, PathMode::WalkableOnly);
    const std::vector<int> fromExit = computeDistances(maze, exit, PathMode::WalkableOnly);
    const int routeLength = fromStart[maze.toIndex(exit)];

    std::vector<GridPos> allTiles;    // any free corridor tile
    std::vector<GridPos> sideTiles;   // tiles that are NOT on a shortest route
    std::vector<GridPos> laterTiles;  // tiles not too close to the entrance

    for (int row = 0; row < maze.getSize(); ++row)
    {
        for (int col = 0; col < maze.getSize(); ++col)
        {
            const GridPos pos{row, col};
            if (maze.at(pos).tile != TileType::Floor || pos == start)
            {
                continue;
            }
            allTiles.push_back(pos);

            const std::size_t index = maze.toIndex(pos);
            const bool onTrueRoute = fromStart[index] + fromExit[index] == routeLength;
            if (!onTrueRoute && fromStart[index] >= MIN_TRAP_DISTANCE)
            {
                sideTiles.push_back(pos);
            }
            if (fromStart[index] >= routeLength / 4)
            {
                laterTiles.push_back(pos);
            }
        }
    }

    if (settings.relic)
    {
        result.relicPlaced = placeRelic(maze, allTiles, fromStart, fromExit, routeLength, rng);
    }

    std::shuffle(allTiles.begin(), allTiles.end(), rng);
    std::shuffle(sideTiles.begin(), sideTiles.end(), rng);
    std::shuffle(laterTiles.begin(), laterTiles.end(), rng);

    result.keysPlaced = placeOnFreeTiles(maze, allTiles, keyCount, ItemType::Key);
    result.timePickupsPlaced = placeOnFreeTiles(maze, laterTiles, settings.timePickups, ItemType::TimePickup);
    result.trapsPlaced = placeOnFreeTiles(maze, sideTiles, settings.traps, ItemType::Trap);
    result.gemsPlaced = placeOnFreeTiles(maze, sideTiles, settings.gems, ItemType::Gem);  // rare = off the route
    result.treasuresPlaced = placeOnFreeTiles(maze, allTiles, settings.treasures, ItemType::Treasure);
}
}  // namespace

bool generateMaze(Maze& maze, int size, const GenerationSettings& settings, std::mt19937& rng,
                  GenerationResult& result)
{
    result = GenerationResult{};
    if (!maze.allocate(size))
    {
        return false;
    }

    carveGrowingTree(maze, rng);
    openEvenEdges(maze, rng);
    result.chambersPlaced = carveChambers(maze, settings.chambers, rng);
    result.loopsAdded = addLoops(maze, settings.loopFraction, rng);
    placeCornerExit(maze, result);
    result.secretDoorsPlaced = placeSecretDoors(maze, settings.secretDoors, rng);
    placeItems(maze, settings, result.secretDoorsPlaced, rng, result);
    return true;
}
