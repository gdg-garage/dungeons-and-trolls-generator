#include "dnt.h"

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

	uint32 distance(Vec2i a, Vec2i b)
	{
		return cage::abs(a[0] - b[0]) + cage::abs(a[1] - b[1]);
	}

	Vec2i findAny(const Floor &f, TileEnum c)
	{
		uint32 cnt = 0;
		for (TileEnum a : f.tiles)
			cnt += a == c;
		if (cnt == 0)
			return Vec2i(-1);
		cnt = randomRange(0u, cnt);
		for (uint32 i = 0; i < f.tiles.size(); i++)
			if (f.tiles[i] == c)
				if (cnt-- == 0)
					return Vec2i(i % f.width, i / f.width);
		return Vec2i(-1);
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
		Floor f;
		f.width = f_.width;
		f.height = f_.height;
		f.tiles = f_.tiles;
		seedReplace(f, findAny(f, TileEnum::Empty), TileEnum::Empty, tmp);
		return countCells(f, TileEnum::Empty) == 0;
	}

	void findOutlineWalls(Floor &f)
	{
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
							neighs += occupancy(f.tile(p + Vec2i(i, j))) != OccupancyEnum::Block;
				if (neighs > 0)
					c = TileEnum::Wall;
			}
		}
		CAGE_ASSERT(isConnected(f));
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
					f.extra(x, y) = std::move(f.extra(x, y + 1));
				}
			}
			f.height--;
			f.tiles.resize(f.width * f.height);
			f.extras.resize(f.width * f.height);
		};
		const auto &transpose = [&]
		{
			Floor g;
			g.tiles.resize(f.tiles.size());
			g.extras.resize(f.extras.size());
			g.width = f.height;
			g.height = f.width;
			for (uint32 y = 0; y < f.height; y++)
			{
				for (uint32 x = 0; x < f.width; x++)
				{
					g.tile(y, x) = f.tile(x, y);
					g.extra(y, x) = std::move(f.extra(x, y));
				}
			}
			std::swap(f.width, f.height);
			std::swap(f.tiles, g.tiles);
			std::swap(f.extras, g.extras);
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

	void makeShopItems(TileExtra &extra, uint32 maxLevel)
	{
		// todo increase number of generated items
		for (uint32 i = 0; i < 1; i++)
		{
			Generate gen;
			Item item = generateItem(Generate(randomRange(1u, maxLevel)));
			item.buyPrice = numeric_cast<uint32>(item.goldCost);
			extra.push_back(std::move(item));
		}
	}

	void generateShopFloor(Floor &f, uint32 maxLevel)
	{
		const uint32 portals = levelToBossIndex(maxLevel);
		const uint32 w = 11 + portals * 2, h = 9;
		floorResize(f, Vec2i(w, h));

		const auto &isDecoration = [=](uint32 x, uint32 y) -> bool { return ((x == 2 || x == w - 3) && (y >= 2 && y <= h - 3)) || ((y == 2 || y == h - 3) && (x >= 2 && x <= w - 3)); };

		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (y == 0 || x == 0 || y + 1 == h || x + 1 == w)
					f.tile(x, y) = TileEnum::Wall;
				else if (isDecoration(x, y))
				{
					f.tile(x, y) = TileEnum::Decoration;
					makeShopItems(f.extra(x, y), maxLevel);
				}
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		f.tile(4, h / 2) = TileEnum::Spawn;
		f.tile(6, h / 2) = TileEnum::Stairs;
		for (uint32 i = 0; i < portals; i++)
		{
			f.tile(8 + i * 2, h / 2) = TileEnum::Waypoint;
			f.extra(8 + i * 2, h / 2).push_back(std::string() + (Stringizer() + "{\"class\":\"waypoint\",\"destinationFloor\":" + bossIndexToLevel(i + 1) + "}").value.c_str());
		}
	}

	Item generateKeyToAllDoors(const Floor &f)
	{
		Item item;
		item.name = "Key";

		std::vector<Vec2i> doors;
		const uint32 cnt = f.width * f.height;
		for (uint32 i = 0; i < cnt; i++)
			if (f.tiles[i] == TileEnum::Door)
				doors.push_back(Vec2i(i % f.width, i / f.width));
		std::string json;
		json += "{\n";
		json += "\"class\":\"key\",\n";
		json += "\"doors\":[\n";
		for (const Vec2i &door : doors)
		{
			json += "{\n";
			json += (Stringizer() + "\"x\":" + door[0] + ",\n").value.c_str();
			json += (Stringizer() + "\"y\":" + door[1] + "\n").value.c_str();
			json += "},"; // /door
		}
		removeLastComma(json);
		json += "]\n"; // /doors
		json += "}"; // /root
		item.flags.push_back(std::move(json));

		return item;
	}

	Monster generateBossMonster(const Floor &f)
	{
		Monster mr = generateMonster(Generate(f.level, levelToBossIndex(f.level)));
		mr.name = Stringizer() + "Guardian of " + f.level + "th floor";
		mr.onDeath.push_back(generateKeyToAllDoors(f));
		return mr;
	}

	void generateBossFloor(Floor &f)
	{
		CAGE_ASSERT(isLevelBoss(f.level));
		const uint32 bossIndex = levelToBossIndex(f.level);
		const uint32 r = bossIndex * 2 + randomRange(5, 10);
		uint32 w = r * 2 + 7;
		floorResize(f, Vec2i(w));

		// generate circular room
		for (uint32 y = 0; y < w; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (distance(Vec2(x, y), Vec2(w / 2)) > r)
					f.tile(x, y) = TileEnum::Outside;
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		// random pillars
		if (r > 10)
		{
			const uint32 cnt = bossIndex / 2 + 1;
			for (uint32 i = 0; i < cnt; i++)
			{
				while (true)
				{
					const uint32 x = randomRange(3u, w - 3);
					const uint32 y = randomRange(3u, w - 3);
					const Vec2i p = Vec2i(x, y);
					{
						const Real d = distance(Vec2(p), Vec2(w / 2));
						if (d + 3 > r || d < 4)
							continue;
					}
					const Vec2i ps[9] = {
						p + Vec2i(-1, -1),
						p + Vec2i(-1, +0),
						p + Vec2i(-1, +1),
						p + Vec2i(+0, -1),
						p + Vec2i(+0, +0),
						p + Vec2i(+0, +1),
						p + Vec2i(+1, -1),
						p + Vec2i(+1, +0),
						p + Vec2i(+1, +1),
					};
					{
						bool bad = false;
						for (Vec2i k : ps)
							if (f.tile(k) != TileEnum::Empty)
								bad = true;
						if (bad)
							continue;
					}
					for (Vec2i k : ps)
						f.tile(k) = TileEnum::Outside;
					break;
				}
			}
		}

		f.tile(w / 2, 2) = TileEnum::Spawn;
		f.tile(w / 2, w - 4) = TileEnum::Door;
		f.tile(w / 2, w - 3) = TileEnum::Stairs;
		f.tile(3, w / 2) = TileEnum::Door;
		f.tile(2, w / 2) = TileEnum::Chest;
		f.tile(w - 4, w / 2) = TileEnum::Door;
		f.tile(w - 3, w / 2) = TileEnum::Chest;

		findOutlineWalls(f);
		cutoutFloor(f); // make sure that all coordinates stored in extra jsons are correct
		w = f.width;

		f.tile(w / 2, w / 2) = TileEnum::Monster; // the boss
		f.extra(w / 2, w / 2).push_back(generateBossMonster(f));

		{ // additional monsters
			const uint32 cnt = bossIndex * 2;
			for (uint32 i = 0; i < cnt; i++)
			{
				while (true)
				{
					const uint32 x = randomRange(3u, w - 3);
					const uint32 y = randomRange(3u, w - 3);
					if (f.tile(x, y) != TileEnum::Empty)
						continue;
					if (distance(Vec2(x, y), Vec2(w / 2)) + 1 > r)
						continue;
					f.tile(x, y) = TileEnum::Monster;
					break;
				}
			}
		}
	}

	void generateMazeFloor(Floor &f)
	{
		const uint32 w = randomRange(f.level + 20, f.level * 2 + 30);
		const uint32 h = randomRange(f.level / 3 + 15, f.level / 2 + 20);
		floorResize(f, Vec2i(w, h));

		{ // carve out the maze
			f.tile(w / 2, h / 2) = TileEnum::Empty;
			const uint32 attempts = w * h * 20;
			for (uint32 i = 0; i < attempts; i++)
			{
				const Vec2i p = findAny(f, TileEnum::Empty);
				if (p[0] < 2 || p[0] > w - 3 || p[1] < 2 || p[1] > h - 3)
					continue;
				static constexpr Vec2i ns[4] = {
					Vec2i(-1, 0),
					Vec2i(+1, 0),
					Vec2i(0, -1),
					Vec2i(0, +1),
				};
				const Vec2i c = p + ns[randomRange(0, 4)];
				if (f.tile(c) != TileEnum::Outside)
					continue;
				uint32 en = 0;
				for (Vec2i n : ns)
					en += f.tile(c + n) == TileEnum::Empty;
				if (en != 1)
					continue;
				f.tile(c) = TileEnum::Empty;
			}
		}

		{ // find outline walls
			findOutlineWalls(f);
		}

		{ // place stairs
			const Vec2i a = findAny(f, TileEnum::Empty);
			const Vec2i b = findFarthest(f, a, TileEnum::Empty);
			f.tile(a) = TileEnum::Stairs;
			f.tile(b) = TileEnum::Spawn;
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
			findOutlineWalls(f);
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

	void fillExtras(Floor &f)
	{
		const uint32 cnt = f.width * f.height;
		for (uint32 i = 0; i < cnt; i++)
		{
			switch (f.tiles[i])
			{
				case TileEnum::Monster:
				{
					if (f.extras[i].empty())
						f.extras[i].push_back(generateMonster(Generate(f.level)));
					break;
				}
				case TileEnum::Chest:
				{
					if (f.extras[i].empty())
						f.extras[i].push_back(generateChest(Generate(f.level)));
					break;
				}
			}
		}
	}
}

Floor generateFloor(uint32 level, uint32 maxLevel)
{
	Floor f;
	f.level = level;
	if (level == 0)
		generateShopFloor(f, maxLevel);
	else if (isLevelBoss(level))
		generateBossFloor(f);
	else if (level > 10 && level < 90 && randomChance() < 0.05)
		generateMazeFloor(f);
	else
		generateGenericFloor(f);
	cutoutFloor(f);
	fillExtras(f);
	return f;
}
