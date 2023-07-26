#include <cage-core/math.h>

#include <string>
#include <vector>

using namespace cage;

enum class TileEnum
{
	// free
	Empty,
	Decoration,
	// props
	Spawn, // the position where all players spawn when entering this level (in any way)
	Waypoint, // teleports players to any other floor with a waypoint
	Stairs, // teleports players to next floor
	Door,
	Chest,
	Monster,
	// blocking
	Wall,
	Outside,
};

enum class OccupancyEnum
{
	Free, // empty tiles, walkable decorations
	Prop, // doors, chests, monsters
	Block, // walls, non-walkable decorations
};

struct Floor
{
	std::vector<TileEnum> tiles;
	std::vector<std::string> extras; // json with extra information relevant for particular tile
	uint32 width = 0, height = 0;
	uint32 level = 0;

	bool inside(Vec2i pos) const { return pos[0] >= 0 && pos[1] >= 0 && pos[0] < width && pos[1] < height; }

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

	TileEnum &tile(uint32 x, uint32 y) { return tile(Vec2i(x, y)); }

	TileEnum tile(uint32 x, uint32 y) const { return tile(Vec2i(x, y)); }

	std::string &extra(Vec2i pos)
	{
		CAGE_ASSERT(inside(pos));
		return extras[pos[1] * width + pos[0]];
	}

	std::string extra(Vec2i pos) const
	{
		CAGE_ASSERT(inside(pos));
		return extras[pos[1] * width + pos[0]];
	}

	std::string &extra(uint32 x, uint32 y) { return extra(Vec2i(x, y)); }

	std::string extra(uint32 x, uint32 y) const { return extra(Vec2i(x, y)); }
};

struct Export
{
	std::string html;
	std::string json;
};

OccupancyEnum occupancy(TileEnum tile);
bool isLevelBoss(uint32 level);
uint32 levelToBossIndex(uint32 level);
uint32 bossIndexToLevel(uint32 index);
std::string generateSkill(uint32 level, Real &power);
std::string generateItem(uint32 level, Real &power);
std::string generateMonster(uint32 level, uint32 difficulty);
Floor generateFloor(uint32 level);
Export exportFloor(const Floor &floor);
void exportDungeon(PointerRange<const Floor> floors);
