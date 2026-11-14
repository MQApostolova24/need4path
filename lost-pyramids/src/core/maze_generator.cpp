#include "core/maze_generator.h"

#include <algorithm>
#include <cstdlib>

namespace
{
const int MIN_DOOR_SPACING = 5;   // secret doors are not placed next to each other
const int MIN_TRAP_DISTANCE = 4;  // no traps right at the entrance
const int ROOM_SIZE = 3;          // burial chambers are 3x3 tiles

bool chance(std::mt19937& rng, float probability)
{
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    return roll(rng) < probability;
}

// ---------------------------------------------------------------- corridors

// "Recursive backtracker" with a stack: dig to a random unvisited neighbour two tiles
// away; when there is none, step back. This visits every cell, so every corridor is
// connected to the entrance.
void carveCorridors(Maze& maze, std::mt19937& rng)
{
    std::vector<GridPos> stack;
    stack.push_back(GridPos{0, 0});
    maze.at(0, 0).tile = TileType::Floor;

    Direction order[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

    while (!stack.empty())
    {
        const GridPos current = stack.back();
        std::shuffle(order, order + 4, rng);  // try the directions in a random order

        bool carved = false;
        for (Direction direction : order)
        {
            // Cells are two tiles apart, so a wall always stays between corridors.
            const GridPos wall = stepInDirection(current, direction);
            const GridPos next = stepInDirection(wall, direction);
            if (!maze.isInside(next) || maze.at(next).tile != TileType::Wall)
            {
                continue;
            }

            // Open the wall between the two cells, then the new cell itself.
            maze.at(wall).tile = TileType::Floor;
            maze.at(next).tile = TileType::Floor;
            stack.push_back(next);
            carved = true;
            break;
        }

        if (!carved)
        {
            stack.pop_back();  // dead end: go back one cell
        }
    }
}

// With even sizes the carving (2 tiles at a time) never reaches the last row and column.
// Open some small nooks there so that edge is part of the pyramid too.
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
}

// ---------------------------------------------------------------- chambers and loops

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

// Opens a few 3x3 burial chambers. Their corners sit on carved cells,
// so every chamber is already connected to the corridors around it.
void carveChambers(Maze& maze, int count, std::mt19937& rng)
{
    const int size = maze.getSize();
    const int lastCell = (size % 2 == 0) ? size - 2 : size - 1;
    const int maxCorner = (lastCell - (ROOM_SIZE - 1)) / 2;  // in steps of 2 tiles
    if (maxCorner < 2)
    {
        return;
    }
    std::uniform_int_distribution<int> pickCorner(1, maxCorner);

    std::vector<GridPos> rooms;  // top-left corner of every chamber
    for (int attempt = 0; attempt < count * 20 && static_cast<int>(rooms.size()) < count; ++attempt)
    {
        const GridPos corner{pickCorner(rng) * 2, pickCorner(rng) * 2};
        if (corner.row + corner.col < 6 || !isFarFromAll(rooms, corner, 2 * ROOM_SIZE + 2))
        {
            continue;  // too close to the entrance or to another chamber
        }

        for (int row = corner.row; row < corner.row + ROOM_SIZE; ++row)
        {
            for (int col = corner.col; col < corner.col + ROOM_SIZE; ++col)
            {
                maze.at(row, col).tile = TileType::Floor;
            }
        }
        rooms.push_back(corner);
    }
}

// Gives some dead ends a second way out, so the pyramid has loops
// (more than one route, and space to escape from the guardian).
void addLoops(Maze& maze, float fraction, std::mt19937& rng)
{
    if (fraction <= 0.0f)
    {
        return;
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
    Direction order[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

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
        for (Direction direction : order)
        {
            const GridPos wall = stepInDirection(end, direction);
            const GridPos beyond = stepInDirection(wall, direction);
            if (maze.isInside(beyond) && maze.at(wall).tile == TileType::Wall && maze.at(beyond).tile == TileType::Floor)
            {
                maze.at(wall).tile = TileType::Floor;
                ++added;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------- the exit

// The exit gate is always in the bottom-right corner (assignment requirement 3).
// With even sizes the corner is outside the carved grid, so a short passage links it.
void placeCornerExit(Maze& maze)
{
    const int last = maze.getSize() - 1;
    if (maze.getSize() % 2 == 0)
    {
        maze.at(last - 1, last).tile = TileType::Floor;
    }
    maze.at(last, last).tile = TileType::Exit;
}

// ---------------------------------------------------------------- secret doors

// How many steps a door between tiles a and b would save (0 if it saves nothing).
int stepsSaved(const Maze& maze, const std::vector<int>& distances, GridPos a, GridPos b)
{
    if (!maze.isInside(a) || !maze.isInside(b))
    {
        return 0;
    }
    const int first = distances[maze.toIndex(a)];
    const int second = distances[maze.toIndex(b)];
    if (first < 0 || second < 0)
    {
        return 0;
    }
    return std::abs(first - second) - 2;  // walking through the door takes 2 steps
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

            // Corridors above and below, or to the left and right of this wall.
            const int gain = std::max(stepsSaved(maze, distances, {row - 1, col}, {row + 1, col}),
                                      stepsSaved(maze, distances, {row, col - 1}, {row, col + 1}));
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

// The relic waits in the dead end that is the longest detour from the true route.
bool placeRelic(Maze& maze, const std::vector<GridPos>& tiles, const std::vector<int>& fromStart,
                const std::vector<int>& fromExit, int routeLength)
{
    GridPos best;
    int bestDetour = 0;
    for (const GridPos& pos : tiles)
    {
        const std::size_t index = maze.toIndex(pos);
        const int detour = fromStart[index] + fromExit[index] - routeLength;  // extra steps to visit this tile
        if (detour > bestDetour && countOpenNeighbours(maze, pos, PathMode::WalkableOnly) == 1)
        {
            best = pos;
            bestDetour = detour;
        }
    }
    if (bestDetour == 0)
    {
        return false;
    }
    maze.at(best).item = ItemType::Relic;
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
        result.relicPlaced = placeRelic(maze, allTiles, fromStart, fromExit, routeLength);
    }

    std::shuffle(allTiles.begin(), allTiles.end(), rng);
    std::shuffle(sideTiles.begin(), sideTiles.end(), rng);
    std::shuffle(laterTiles.begin(), laterTiles.end(), rng);

    placeOnFreeTiles(maze, allTiles, keyCount, ItemType::Key);
    placeOnFreeTiles(maze, laterTiles, settings.timePickups, ItemType::TimePickup);
    placeOnFreeTiles(maze, sideTiles, settings.traps, ItemType::Trap);
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

    carveCorridors(maze, rng);
    openEvenEdges(maze, rng);
    carveChambers(maze, settings.chambers, rng);
    addLoops(maze, settings.loopFraction, rng);
    placeCornerExit(maze);
    result.secretDoorsPlaced = placeSecretDoors(maze, settings.secretDoors, rng);
    placeItems(maze, settings, result.secretDoorsPlaced, rng, result);
    return true;
}
