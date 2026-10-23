#include "core/maze.h"

#include "core/config.h"

#include <queue>

namespace
{
const int NEIGHBOUR_ROW[4] = {-1, 1, 0, 0};
const int NEIGHBOUR_COL[4] = {0, 0, -1, 1};
}

bool operator==(const GridPos& left, const GridPos& right)
{
    return left.row == right.row && left.col == right.col;
}

bool operator!=(const GridPos& left, const GridPos& right)
{
    return !(left == right);
}

Maze::~Maze()
{
    release();
}

bool Maze::allocate(int newSize)
{
    if (newSize < MIN_MAZE_SIZE || newSize > MAX_MAZE_SIZE)
    {
        return false;
    }

    release();

    size = newSize;
    cells = new Cell*[size];
    for (int row = 0; row < size; ++row)
    {
        cells[row] = new Cell[size];  // every Cell starts as a wall
    }
    return true;
}

void Maze::release()
{
    if (cells != nullptr)
    {
        for (int row = 0; row < size; ++row)
        {
            delete[] cells[row];
        }
        delete[] cells;
        cells = nullptr;
    }
    size = 0;
}

bool Maze::isAllocated() const
{
    return cells != nullptr;
}

int Maze::getSize() const
{
    return size;
}

bool Maze::isInside(int row, int col) const
{
    return row >= 0 && row < size && col >= 0 && col < size;
}

bool Maze::isInside(GridPos pos) const
{
    return isInside(pos.row, pos.col);
}

Cell& Maze::at(int row, int col)
{
    return cells[row][col];
}

const Cell& Maze::at(int row, int col) const
{
    return cells[row][col];
}

Cell& Maze::at(GridPos pos)
{
    return cells[pos.row][pos.col];
}

const Cell& Maze::at(GridPos pos) const
{
    return cells[pos.row][pos.col];
}

GridPos Maze::getStart() const
{
    return GridPos{0, 0};
}

GridPos Maze::getExit() const
{
    return GridPos{size - 1, size - 1};
}

std::size_t Maze::toIndex(GridPos pos) const
{
    return static_cast<std::size_t>(pos.row * size + pos.col);
}

bool isPassable(TileType tile, PathMode mode)
{
    switch (tile)
    {
    case TileType::Floor:
    case TileType::OpenedDoor:
        return true;
    case TileType::Exit:
        return mode != PathMode::MummyOnly;
    case TileType::SecretDoor:
        return mode == PathMode::IncludeSecretDoors;
    case TileType::Wall:
        return false;
    }
    return false;
}

std::vector<int> computeDistances(const Maze& maze, GridPos from, PathMode mode)
{
    const int size = maze.getSize();
    std::vector<int> distances(static_cast<std::size_t>(size * size), -1);
    if (!maze.isInside(from))
    {
        return distances;
    }

    std::queue<GridPos> frontier;
    distances[maze.toIndex(from)] = 0;
    frontier.push(from);

    while (!frontier.empty())
    {
        const GridPos current = frontier.front();
        frontier.pop();
        const int currentDistance = distances[maze.toIndex(current)];

        for (int direction = 0; direction < 4; ++direction)
        {
            const GridPos next{current.row + NEIGHBOUR_ROW[direction], current.col + NEIGHBOUR_COL[direction]};
            if (!maze.isInside(next))
            {
                continue;
            }

            const std::size_t nextIndex = maze.toIndex(next);
            if (distances[nextIndex] != -1 || !isPassable(maze.at(next).tile, mode))
            {
                continue;
            }

            distances[nextIndex] = currentDistance + 1;
            frontier.push(next);
        }
    }

    return distances;
}

int shortestPathLength(const Maze& maze, GridPos from, GridPos to, PathMode mode)
{
    if (!maze.isInside(from) || !maze.isInside(to))
    {
        return -1;
    }
    const std::vector<int> distances = computeDistances(maze, from, mode);
    return distances[maze.toIndex(to)];
}
