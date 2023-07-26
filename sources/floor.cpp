#include "dnt.h"

OccupancyEnum occupancy(TileEnum tile)
{
	switch (tile)
	{
		case TileEnum::Outside:
		case TileEnum::Wall:
			return OccupancyEnum::Block;
		case TileEnum::Decoration:
		case TileEnum::Empty:
			return OccupancyEnum::Free;
		default:
			return OccupancyEnum::Prop;
	}
}

namespace
{
	void floorResize(Floor &f, Vec2i size)
	{
		f.tiles.clear();
		f.tiles.resize(size[0] * size[1], TileEnum::Outside);
		f.extras.clear();
		f.extras.resize(size[0] * size[1]);
		f.width = size[0];
		f.height = size[1];
	}

	void generateShopFloor(Floor &f)
	{
		static constexpr uint32 w = 17, h = 9;
		floorResize(f, Vec2i(w, h));

		const auto &isDecoration = [](uint32 x, uint32 y) -> bool { return ((x == 2 || x == w - 3) && (y >= 2 && y <= h - 3)) || ((y == 2 || y == h - 3) && (x >= 2 && x <= w - 3)); };

		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (y == 0 || x == 0 || y + 1 == h || x + 1 == w)
					f.tile(x, y) = TileEnum::Wall;
				else if (isDecoration(x, y))
					f.tile(x, y) = TileEnum::Decoration;
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		f.tile(w / 3, h / 2) = TileEnum::Stairs;
		f.tile(w / 2, h / 2) = TileEnum::Spawn;
		f.tile(2 * w / 3, h / 2) = TileEnum::Waypoint;
	}

	void generateBossFloor(Floor &f)
	{
		const uint32 w = randomRange(40, 60);
		const uint32 h = randomRange(10, 20);
		floorResize(f, Vec2i(w, h));

		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (y == 0 || x == 0 || y + 1 == h || x + 1 == w)
					f.tile(x, y) = TileEnum::Wall;
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		f.tile(1, 1) = TileEnum::Spawn;
		f.tile(w - 2, h - 2) = TileEnum::Stairs;
		f.tile(w - 2, 1) = TileEnum::Waypoint;
		f.tile(1, h - 2) = TileEnum::Chest; // random loot
		f.tile(w / 2 - 1, h / 2 - 1) = TileEnum::Monster; // the boss

		const uint32 cnt = f.level / 20 + 2;
		for (uint32 i = 0; i < cnt; i++)
			f.tile(randomRange(2u, w - 2), randomRange(2u, h - 2)) = TileEnum::Monster; // few additional monsters
	}

	uint32 distance(Vec2i a, Vec2i b)
	{
		return cage::abs(a[0] - b[0]) + cage::abs(a[1] - b[1]);
	}

	Vec2i findAny(const Floor &f, TileEnum c)
	{
		std::vector<Vec2i> tmp;
		tmp.reserve(f.width * f.height);
		for (uint32 i = 0; i < f.tiles.size(); i++)
			if (f.tiles[i] == c)
				tmp.push_back(Vec2i(i % f.width, i / f.width));
		if (tmp.empty())
			return Vec2i(-1);
		return tmp[randomRange(0u, numeric_cast<uint32>(tmp.size()))];
	}

	Vec2i findNearest(const Floor &f, Vec2i s, TileEnum c)
	{
		uint32 dst = m;
		Vec2i res = Vec2i(-1);
		for (uint32 i = 0; i < f.tiles.size(); i++)
		{
			if (f.tiles[i] == c)
			{
				const Vec2i p = Vec2i(i % f.width, i / f.width);
				const uint32 d = distance(s, p);
				if (d < dst)
				{
					dst = d;
					res = p;
				}
			}
		}
		return res;
	}

	Vec2i findFarthest(const Floor &f, Vec2i s, TileEnum c)
	{
		uint32 dst = 0;
		Vec2i res = Vec2i(-1);
		for (uint32 i = 0; i < f.tiles.size(); i++)
		{
			if (f.tiles[i] == c)
			{
				const Vec2i p = Vec2i(i % f.width, i / f.width);
				const uint32 d = distance(s, p);
				if (d > dst)
				{
					dst = d;
					res = p;
				}
			}
		}
		return res;
	}

	void rectReplace(Floor &f, Vec2i a, Vec2i b, TileEnum what, TileEnum with)
	{
		for (sint32 y = a[1]; y < b[1]; y++)
		{
			for (sint32 x = a[0]; x < b[0]; x++)
			{
				TileEnum &c = f.tile(x, y);
				if (c == what)
					c = with;
			}
		}
	}

	void pathReplace(Floor &f, Vec2i a, Vec2i b, TileEnum what, TileEnum with)
	{
		const auto &move = [](sint32 a, sint32 b) -> sint32
		{
			CAGE_ASSERT(a != b);
			return b > a ? a + 1 : a - 1;
		};
		while (a != b)
		{
			if (f.tile(a) == what)
				f.tile(a) = with;
			if (a[0] != b[0])
			{
				if (a[1] != b[1])
				{
					if (randomChance() < 0.5)
						a[0] = move(a[0], b[0]);
					else
						a[1] = move(a[1], b[1]);
				}
				else
					a[0] = move(a[0], b[0]);
			}
			else
				a[1] = move(a[1], b[1]);
		}
	}

	void seedReplace(Floor &f, Vec2i seed, TileEnum what, TileEnum with)
	{
		CAGE_ASSERT(what != with);
		std::vector<Vec2i> q;
		q.push_back(seed);
		while (!q.empty())
		{
			const Vec2i p = q.back();
			q.pop_back();
			TileEnum &c = f.tile(p);
			if (c != what)
				continue;
			c = with;
			for (Vec2i it : { p + Vec2i(-1, 0), p + Vec2i(1, 0), p + Vec2i(0, -1), p + Vec2i(0, 1) })
				if (f.tile(it) == what)
					q.push_back(it);
		}
	}

	uint32 countCells(const Floor &f, TileEnum t)
	{
		uint32 cnt = 0;
		for (TileEnum c : f.tiles)
			if (c == t)
				cnt++;
		return cnt;
	}

	bool isConnected(const Floor &f_)
	{
		static constexpr TileEnum tmp = TileEnum::Monster;
		CAGE_ASSERT(countCells(f_, tmp) == 0);
		Floor f = f_;
		seedReplace(f, findAny(f, TileEnum::Empty), TileEnum::Empty, tmp);
		return countCells(f, TileEnum::Empty) == 0;
	}

	void generateGenericFloor(Floor &f)
	{
		const uint32 w = randomRange(f.level + 20, f.level * 2 + 30);
		const uint32 h = randomRange(f.level / 3 + 15, f.level / 2 + 20);
		floorResize(f, Vec2i(w, h));

		{ // rooms
			const uint32 cnt = randomRange(f.level / 15, f.level / 5 + 10) + 3;
			for (uint32 i = 0; i < cnt; i++)
			{
				const uint32 s = f.level / 10 + randomRange(5, 10);
				const Vec2i a = Vec2i(randomRange(1u, w - s), randomRange(1u, h - s));
				const Vec2i b = a + randomRange2i(3, s + 1);
				rectReplace(f, a, b, TileEnum::Outside, TileEnum::Empty);
			}
		}

		{ // corridors
			static constexpr TileEnum tmp = TileEnum::Monster;
			seedReplace(f, findAny(f, TileEnum::Empty), TileEnum::Empty, tmp);
			while (countCells(f, TileEnum::Empty) > 0)
			{
				const Vec2i a = findAny(f, TileEnum::Empty);
				const Vec2i b = findNearest(f, a, tmp);
				const Vec2i c = findNearest(f, b, TileEnum::Empty);
				pathReplace(f, b, c, TileEnum::Outside, tmp);
				f.tile(c) = TileEnum::Empty;
				seedReplace(f, c, TileEnum::Empty, tmp);
			}
			rectReplace(f, Vec2i(), Vec2i(f.width, f.height), tmp, TileEnum::Empty); // restore back
			CAGE_ASSERT(isConnected(f));
		}

		{ // find outline walls
			for (uint32 y = 0; y < f.height; y++)
			{
				for (uint32 x = 0; x < f.width; x++)
				{
					const Vec2i p = Vec2i(x, y);
					TileEnum &c = f.tile(p);
					if (c != TileEnum::Outside)
						continue;
					uint32 neighs = 0;
					for (sint32 j = -1; j < 2; j++)
						for (sint32 i = -1; i < 2; i++)
							if (f.inside(p + Vec2i(i, j)))
								neighs += f.tile(p + Vec2i(i, j)) == TileEnum::Empty;
					if (neighs > 0)
						c = TileEnum::Wall;
				}
			}
			CAGE_ASSERT(isConnected(f));
		}

		{ // place stairs
			const Vec2i a = findAny(f, TileEnum::Empty);
			const Vec2i b = findFarthest(f, a, TileEnum::Empty);
			const Vec2i c = findFarthest(f, b, TileEnum::Empty);
			f.tile(b) = TileEnum::Spawn;
			f.tile(c) = TileEnum::Stairs;
		}

		{ // place some monsters
			const uint32 cnt = randomRange(f.level / 6, f.level / 3 + 1) + 1;
			for (uint32 i = 0; i < cnt; i++)
			{
				const Vec2i p = findAny(f, TileEnum::Empty);
				if (p == Vec2i(-1))
					break;
				f.tile(p) = TileEnum::Monster;
			}
		}
	}

	void cutoutFloor(Floor &f)
	{
		const auto &rowOutside = [&](uint32 y) -> bool
		{
			for (uint32 x = 0; x < f.width; x++)
				if (f.tile(x, y) != TileEnum::Outside)
					return false;
			return true;
		};
		const auto &cutRow = [&](uint32 y)
		{
			for (; y < f.height - 1; y++)
			{
				for (uint32 x = 0; x < f.width; x++)
				{
					f.tile(x, y) = f.tile(x, y + 1);
					f.extra(x, y) = f.extra(x, y + 1);
				}
			}
			f.height--;
			f.tiles.resize(f.width * f.height);
			f.extras.resize(f.width * f.height);
		};
		const auto &transpose = [&]
		{
			Floor g = f;
			std::swap(g.width, g.height);
			for (uint32 y = 0; y < f.height; y++)
			{
				for (uint32 x = 0; x < f.width; x++)
				{
					g.tile(y, x) = f.tile(x, y);
					g.extra(y, x) = f.extra(x, y);
				}
			}
			std::swap(f, g);
		};

		for (uint32 i = 0; i < 2; i++)
		{
			while (rowOutside(0))
				cutRow(0);
			while (rowOutside(f.height - 1))
				cutRow(f.height - 1);
			transpose();
		}
	}
}

Floor generateFloor(uint32 level)
{
	Floor f;
	f.level = level;
	if (level == 0)
		generateShopFloor(f);
	else if (level > 20 && (level % 13) == 0)
		generateBossFloor(f);
	else
		generateGenericFloor(f);
	cutoutFloor(f);
	return f;
}
