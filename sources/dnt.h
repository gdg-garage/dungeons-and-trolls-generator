#include <cage-core/math.h>

#include <vector>

using namespace cage;

enum class TileEnum
{
	Outside,
	Wall,
	Waypoint,
	Stairs,
	Door,
	Curtain,
	Chest,
	Item,
	Key,
	Monster,
	Empty,
	Rug,
};

enum class OccupancyEnum
{
	Block, // walls, unwalkable decorations
	Prop, // doors, chests, monsters
	Free, // empty tiles, walkable decorations
};

OccupancyEnum occupancy(TileEnum tile);

struct Floor
{
	uint32 width = 0, height = 0;
	std::vector<TileEnum> tiles;

	bool inside(Vec2i pos) const
	{
		return pos[0] >= 0 && pos[1] >= 0 && pos[0] < width && pos[1] < height;
	}

	TileEnum &tile(Vec2i pos)
	{
		CAGE_ASSERT(inside(pos));
		return tiles[pos[1] * width + pos[0]];
	}

	TileEnum tile(Vec2i pos) const
	{
		CAGE_ASSERT(inside(pos));
		return tiles[pos[1] * width + pos[0]];
	}

	TileEnum &tile(uint32 x, uint32 y)
	{
		return tile(Vec2i(x, y));
	}

	TileEnum tile(uint32 x, uint32 y) const
	{
		return tile(Vec2i(x, y));
	}
};

struct Dungeon
{
	std::vector<Floor> floors;
};

extern Dungeon dungeon;
