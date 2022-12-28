#include "dnt.h"

Dungeon dungeon;

void generateFloor(Floor &f, uint32 level);

OccupancyEnum occupancy(TileEnum tile)
{
	switch (tile)
	{
	case TileEnum::Outside:
	case TileEnum::Wall:
		return OccupancyEnum::Block;
	case TileEnum::Empty:
	case TileEnum::Rug:
		return OccupancyEnum::Free;
	default:
		return OccupancyEnum::Prop;
	}
}

void generateDungeon()
{
	dungeon.floors.resize(100);
	uint32 level = 0;
	for (Floor &f : dungeon.floors)
		generateFloor(f, level++);
}
