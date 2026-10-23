#include "core/maze_generator.h"

#include <algorithm>
#include <cstdlib>

namespace
{
// Corridors are carved two tiles at a time so walls stay between them.
const int CARVE_ROW[4] = {-2, 2, 0, 0};
const int CARVE_COL[4] = {0, 0, -2, 2};

const int MIN_DOOR_SPACING = 5;       // secret doors are not placed next to each other
const int MIN_TRAP_DISTANCE = 4;      // no traps right at the entrance

// Randomized depth-first search ("recursive backtracker") written with an explicit stack.
void carvePassages(Maze& maze, std::mt19937& rng)
{
    std::vector<GridPos> stack;
    stack.push_back(GridPos{0, 0});
    maze.at(0, 0).tile = TileType::Floor;

    int order[4] = {0, 1, 2, 3};

    while (!stack.empty())
    {
        const GridPos current = stack.back();
        std::shuffle(order, order + 4, rng);

        bool carved = false;
        for (int direction : order)
        {
            const GridPos next{current.row + CARVE_ROW[direction], current.col + CARVE_COL[direction]};
            if (!maze.isInside(next) || maze.at(next).tile != TileType::Wall)
            {
                continue;
            }

            // Open the wall between the two cells and the new cell itself.
            maze.at(current.row + CARVE_ROW[direction] / 2, current.col + CARVE_COL[direction] / 2).tile = TileType::Floor;
            maze.at(next).tile = TileType::Floor;
            stack.push_back(next);
            carved = true;
            break;
        }

        if (!carved)
        {
            stack.pop_back();
        }
    }
}

// Makes sure the Sun Gate in the bottom-right corner is reachable.
void connectExit(Maze& maze, std::mt19937& rng)
{
    const int last = maze.getSize() - 1;

    if (maze.getSize() % 2 == 0)
    {
        // With even sizes the carving never touches the last row and column,
        // so we link the exit and open a few small alcoves to keep it natural.
        maze.at(last - 1, last).tile = TileType::Floor;

        std::bernoulli_distribution openAlcove(0.3);
        for (int i = 0; i < last - 1; i += 2)
        {
            if (openAlcove(rng))
            {
                maze.at(i, last).tile = TileType::Floor;
            }
            if (openAlcove(rng))
            {
                maze.at(last, i).tile = TileType::Floor;
            }
        }
    }

    maze.at(last, last).tile = TileType::Exit;
}

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

// Finds walls that separate two corridors which are far apart along the maze
// and turns some of them into secret doors (shortcuts).
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
            // A vertical pair (above / below) or a horizontal pair (left / right).
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

// Puts an item on the first "count" free tiles from the list.
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

void placeItems(Maze& maze, const GenerationSettings& settings, int ankhCount,
                std::mt19937& rng, GenerationResult& result)
{
    const GridPos start = maze.getStart();
    const GridPos exit = maze.getExit();
    const std::vector<int> fromStart = computeDistances(maze, start, PathMode::WalkableOnly);
    const std::vector<int> fromExit = computeDistances(maze, exit, PathMode::WalkableOnly);
    const int routeLength = fromStart[maze.toIndex(exit)];

    std::vector<GridPos> allTiles;   // any free corridor tile
    std::vector<GridPos> sideTiles;  // tiles that are NOT on the true route

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
        }
    }

    std::shuffle(allTiles.begin(), allTiles.end(), rng);
    std::shuffle(sideTiles.begin(), sideTiles.end(), rng);

    result.ankhsPlaced = placeOnFreeTiles(maze, allTiles, ankhCount, ItemType::Ankh);
    result.trapsPlaced = placeOnFreeTiles(maze, sideTiles, settings.traps, ItemType::Trap);
    result.treasuresPlaced = placeOnFreeTiles(maze, allTiles, settings.treasures, ItemType::Treasure);
}
}  // namespace

bool generateMaze(Maze& maze, int size, const GenerationSettings& settings,
                  std::mt19937& rng, GenerationResult& result)
{
    result = GenerationResult{};
    if (!maze.allocate(size))
    {
        return false;
    }

    carvePassages(maze, rng);
    connectExit(maze, rng);
    result.secretDoorsPlaced = placeSecretDoors(maze, settings.secretDoors, rng);
    placeItems(maze, settings, result.secretDoorsPlaced, rng, result);
    return true;
}
