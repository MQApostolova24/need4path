#pragma once

#include <cstddef>
#include <vector>

// What a single tile of the pyramid is made of.
enum class TileType
{
    Wall,        // solid stone - cannot be crossed
    Floor,       // corridor
    SecretDoor,  // carved wall - opens with a key
    OpenedDoor,  // secret door that was already opened
    Exit         // the gate out of the pyramid
};

// What lies on top of a tile.
enum class ItemType
{
    None,
    Key,         // opens one secret door (Ankh / Jade glyph / Lotus key)
    Treasure,    // common reward (gold scarab, gold bead, silver coin)
    Gem,         // rare reward
    Relic,       // one legendary relic per pyramid
    TimePickup,  // adds seconds to the location timer
    Trap,        // hidden trap - costs a life
    SprungTrap   // trap that already fired (safe now)
};

// Which tiles count as passable when searching for paths.
enum class PathMode
{
    WalkableOnly,        // the player without keys
    IncludeSecretDoors,  // the player with unlimited keys
    GuardianOnly         // the guardian cannot use doors or the exit
};

struct Cell
{
    TileType tile = TileType::Wall;
    ItemType item = ItemType::None;
    bool explored = false;  // the player has seen this tile with the torch
};

struct GridPos
{
    int row = 0;
    int col = 0;
};

enum class Direction
{
    None,
    Up,
    Down,
    Left,
    Right
};

// The four directions, for loops that visit every neighbour of a tile.
const Direction ALL_DIRECTIONS[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

// The neighbouring tile in the given direction.
GridPos stepInDirection(GridPos pos, Direction direction);

bool operator==(const GridPos& left, const GridPos& right);
bool operator!=(const GridPos& left, const GridPos& right);

// The pyramid grid. Stored as a dynamic two-dimensional array (Cell**)
// whose size is chosen by the player at runtime (10x10 up to 50x50).
class Maze
{
public:
    Maze() = default;
    ~Maze();

    // The grid owns raw memory, so copying is forbidden.
    Maze(const Maze&) = delete;
    Maze& operator=(const Maze&) = delete;

    // Allocates a size x size grid filled with walls. Returns false for invalid sizes.
    bool allocate(int newSize);
    // Frees the grid memory.
    void release();

    bool isAllocated() const;
    int getSize() const;

    bool isInside(int row, int col) const;
    bool isInside(GridPos pos) const;

    Cell& at(int row, int col);
    const Cell& at(int row, int col) const;
    Cell& at(GridPos pos);
    const Cell& at(GridPos pos) const;

    GridPos getStart() const;  // always (0, 0) - the top-left corner
    GridPos getExit() const;   // always (size - 1, size - 1) - the bottom-right corner

    // Converts a position to an index in a flat size * size array.
    std::size_t toIndex(GridPos pos) const;

private:
    Cell** cells = nullptr;
    int size = 0;
};

// True if a tile can be stepped on in the given path mode.
bool isPassable(TileType tile, PathMode mode);

// Breadth-first search. Returns the distance from "from" to every tile
// (flat array, index = row * size + col). Unreachable tiles get -1.
std::vector<int> computeDistances(const Maze& maze, GridPos from, PathMode mode);

// Length of the shortest path between two tiles, or -1 if there is none.
int shortestPathLength(const Maze& maze, GridPos from, GridPos to, PathMode mode);

// Number of passable neighbours (1 means the tile is a dead end).
int countOpenNeighbours(const Maze& maze, GridPos pos, PathMode mode);

// True if a straight line from the centre of "from" to the centre of "to"
// does not pass through a wall. The target tile itself may be a wall (you can see a wall's face).
bool hasLineOfSight(const Maze& maze, GridPos from, GridPos to);
