#pragma once

#include <cstddef>
#include <vector>

// What a single tile of the tomb is made of.
enum class TileType
{
    Wall,        // solid sandstone - cannot be crossed
    Floor,       // corridor
    SecretDoor,  // wall marked with the Eye of Horus - opens with an Ankh key
    OpenedDoor,  // secret door that was already opened
    Exit         // the Sun Gate (bottom-right corner)
};

// What lies on top of a tile.
enum class ItemType
{
    None,
    Ankh,       // key that opens one secret door
    Treasure,   // bonus points
    Trap,       // hidden scarab trap - costs a life
    SprungTrap  // trap that already fired (safe now)
};

// Which tiles count as passable when searching for paths.
enum class PathMode
{
    WalkableOnly,        // the player without keys
    IncludeSecretDoors,  // the player with unlimited keys
    MummyOnly            // the mummy cannot use doors or the exit
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

bool operator==(const GridPos& left, const GridPos& right);
bool operator!=(const GridPos& left, const GridPos& right);

// The tomb grid. Stored as a dynamic two-dimensional array (Cell**)
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

    GridPos getStart() const;  // always (0, 0)
    GridPos getExit() const;   // always (size - 1, size - 1)

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
