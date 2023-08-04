#include "dnt.h"

namespace
{
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
		CAGE_ASSERT(countCells(f_, TileEnum::Placeholder) == 0);
		Floor f;
		f.width = f_.width;
		f.height = f_.height;
		f.tiles = f_.tiles;
		seedReplace(f, findAny(f, TileEnum::Empty), TileEnum::Empty, TileEnum::Placeholder);
		return countCells(f, TileEnum::Empty) == 0;
	}

	std::pair<uint32, uint32> defaultFloorSize(uint32 level)
	{
		const uint32 s = min(level * 2, 50u) + 10;
		const uint32 v = min(level / 3, 30u) + 2;
		const uint32 w = (s + randomRange(0u, v));
		const uint32 h = (s + randomRange(0u, v)) / 2;
		return { w, h };
	}

	void resizeFloor(Floor &f, Vec2i size)
	{
		f.tiles.clear();
		f.tiles.resize(size[0] * size[1], TileEnum::Outside);
		f.extras.clear();
		f.extras.resize(size[0] * size[1]);
		f.width = size[0];
		f.height = size[1];
	}

	void findOutlineWalls(Floor &f)
	{
		CAGE_ASSERT(isConnected(f));
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

	void placeMonsters(Floor &f, sint32 powerOffset)
	{
		const uint32 a = f.level / 2 + 1;
		const uint32 b = f.width * f.height / 50;
		const uint32 cnt = min(randomRange(min(a, b), max(a, b)), 40u);
		CAGE_ASSERT(cnt * 2 < countCells(f, TileEnum::Empty));
		for (uint32 i = 0; i < cnt; i++)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(generateMonster(Generate(f.level, powerOffset)));
		}
	}

	void placeSpawnAndStairs(Floor &f)
	{
		const Vec2i a = findAny(f, TileEnum::Empty);
		const Vec2i b = findFarthest(f, a, TileEnum::Empty);
		const Vec2i c = findFarthest(f, b, TileEnum::Empty);
		f.tile(b) = TileEnum::Spawn;
		f.tile(c) = TileEnum::Stairs;
	}

	void generateShopFloor(Floor &f, uint32 maxLevel)
	{
		const uint32 portals = levelToBossIndex(maxLevel - 1);
		const uint32 w = 11 + portals * 2, h = 9;
		resizeFloor(f, Vec2i(w, h));

		const auto &isDecoration = [=](uint32 x, uint32 y) -> bool { return ((x == 2 || x == w - 3) && (y >= 2 && y <= h - 3)) || ((y == 2 || y == h - 3) && (x >= 2 && x <= w - 3)); };

		const auto &makeShopItems = [&](TileExtra &extra)
		{
			for (uint32 i = 0; i < 10; i++)
			{
				Generate gen;
				switch (randomRange(0u, 4u))
				{
					case 0:
						gen = Generate(randomRange(1u, maxLevel), 0); // default
						break;
					case 1:
					case 2:
						gen = Generate(randomRange(max(maxLevel * 3 / 4, 1u), maxLevel), 0); // stronger than default
						break;
					case 3:
						gen = Generate(maxLevel, -randomRange(0u, maxLevel)); // any features, but possibly weak
						break;
				}
				CAGE_ASSERT(gen.level > 0);
				Item item = generateItem(gen);
				item.buyPrice = numeric_cast<uint32>(item.goldCost);
				extra.push_back(std::move(item));
			}
		};

		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (y == 0 || x == 0 || y + 1 == h || x + 1 == w)
					f.tile(x, y) = TileEnum::Wall;
				else if (isDecoration(x, y))
				{
					f.tile(x, y) = TileEnum::Decoration;
					makeShopItems(f.extra(x, y));
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
			f.extra(8 + i * 2, h / 2).push_back(std::string() + (Stringizer() + "{\"class\":\"waypoint\",\"destinationFloor\":" + (bossIndexToLevel(i + 1) + 1) + "}").value.c_str());
		}
	}

	void generateBossFloor(Floor &f)
	{
		CAGE_ASSERT(isLevelBoss(f.level));
		const uint32 bossIndex = levelToBossIndex(f.level);
		const uint32 r = (min(f.level * 2, 50u) + 10 + randomRange(0u, min(f.level / 3, 30u) + 2)) / 2;
		uint32 w = r * 2 + 7;
		uint32 h = r + 4;
		resizeFloor(f, Vec2i(w, h));

		const auto &dist = [&](Vec2i p) -> Real { return distance(Vec2(p[0], p[1] * 2), Vec2(w / 2)); };

		// generate circular room
		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				const Vec2i p = Vec2i(x, y);
				if (dist(p) < r)
					f.tile(x, y) = TileEnum::Empty;
				else
					f.tile(x, y) = TileEnum::Outside;
			}
		}

		// random pillars
		if (r > 10)
		{
			const uint32 cnt = min(bossIndex + 1, 10u);
			for (uint32 i = 0; i < cnt; i++)
			{
				while (true)
				{
					const uint32 x = randomRange(3u, w - 3);
					const uint32 y = randomRange(3u, h - 3);
					const Vec2i p = Vec2i(x, y);
					{
						const Real d = dist(p);
						if (d + 6 > r || d < 4)
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

		f.tile(w / 2, 1) = TileEnum::Spawn;

		f.tile(w / 2 - 1, h - 3) = TileEnum::Wall;
		f.tile(w / 2, h - 3) = TileEnum::Door;
		f.tile(w / 2 + 1, h - 3) = TileEnum::Wall;
		f.tile(w / 2, h - 2) = TileEnum::Stairs;

		f.tile(3, h / 2) = TileEnum::Door;
		f.tile(2, h / 2) = TileEnum::Chest;
		f.extra(2, h / 2).push_back(generateChest(Generate(f.level, f.level / 10)));

		f.tile(w - 4, h / 2) = TileEnum::Door;
		f.tile(w - 3, h / 2) = TileEnum::Chest;
		f.extra(w - 3, h / 2).push_back(generateChest(Generate(f.level, f.level / 10)));

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		const auto &generateBossMonster = [&]()
		{
			const auto &generateKeyToAllDoors = [&]()
			{
				Item item;
				item.name = "Key";
				item.icon = "key";

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
			};

			Monster mr = generateMonster(Generate(f.level, f.level / 5));
			mr.name = Stringizer() + "Guardian of " + f.level + "th floor";
			mr.icon = "boss";
			mr.onDeath.push_back(generateKeyToAllDoors());
			return mr;
		};
		f.tile(w / 2, h / 2) = TileEnum::Monster;
		f.extra(w / 2, h / 2).push_back(generateBossMonster());

		placeMonsters(f, f.level / 20);
	}

	void generateSingleRoomFloor(Floor &f)
	{
		const auto [w, h] = defaultFloorSize(f.level);
		resizeFloor(f, Vec2i(w, h));
		rectReplace(f, Vec2i(1), Vec2i(w, h) - 1, TileEnum::Outside, TileEnum::Empty);
		findOutlineWalls(f);
		placeSpawnAndStairs(f);
		placeMonsters(f, 0);
	}

	void generateMazeFloor(Floor &f)
	{
		auto [w, h] = defaultFloorSize(f.level);
		resizeFloor(f, Vec2i(w, h));

		{ // carve out the maze
			f.tile(w / 2, h / 2) = TileEnum::Empty;
			while (countCells(f, TileEnum::Empty) < w * h * 9 / 20)
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

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		{ // place stairs
			const Vec2i a = findAny(f, TileEnum::Empty);
			const Vec2i b = findFarthest(f, a, TileEnum::Empty);
			f.tile(a) = TileEnum::Stairs;
			f.tile(b) = TileEnum::Spawn;
		}

		placeMonsters(f, 2);
	}

	void generateStripesFloor(Floor &f)
	{
		auto [w, h] = defaultFloorSize(f.level);
		resizeFloor(f, Vec2i(w, h));

		{ // rooms
			uint32 x = 1;
			while (x + 5 < w)
			{
				const uint32 s = randomRange(1, 5);
				rectReplace(f, Vec2i(x, 1), Vec2i(x + s, h - 1), TileEnum::Outside, TileEnum::Empty);
				if (x > 1)
				{
					const uint32 y = randomRange(3u, h - 5);
					f.tile(x - 1, y) = TileEnum::Empty;
				}
				x += s + 1;
			}
		}

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		placeSpawnAndStairs(f);
		placeMonsters(f, 0);
	}

	void generateGenericFloor(Floor &f)
	{
		auto [w, h] = defaultFloorSize(f.level);
		resizeFloor(f, Vec2i(w, h));

		// add rooms
		while (countCells(f, TileEnum::Empty) < w * h / 2)
		{
			const uint32 s = randomRange(0u, h / 5) + 3;
			const Vec2i a = Vec2i(randomRange(1u, w - s), randomRange(1u, h - s));
			const Vec2i b = a + randomRange2i(3, s + 1);
			rectReplace(f, a, b, TileEnum::Outside, TileEnum::Empty);
		}

		// add barriers
		while (countCells(f, TileEnum::Empty) > w * h / 3)
		{
			const uint32 s = randomRange(0u, h / 5) + 3;
			const Vec2i a = Vec2i(randomRange(1u, w - s), randomRange(1u, h - s));
			const Vec2i b = a + randomRange2i(3, s + 1);
			rectReplace(f, a, b, TileEnum::Empty, TileEnum::Outside);
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
		}

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		placeSpawnAndStairs(f);

		// random chest
		if (f.level > 15 && randomChance() < 0.5)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Chest;
			f.extra(p).push_back(generateChest(Generate(f.level, f.level / 20)));
		}

		// place waypoint
		if (f.level > 25 && randomChance() < 0.05)
		{
			const Vec2i a = findAny(f, TileEnum::Empty);
			uint32 neighs = 0;
			for (sint32 j = -1; j < 2; j++)
			{
				for (sint32 i = -1; i < 2; i++)
				{
					const Vec2i b = a + Vec2i(i, j);
					CAGE_ASSERT(f.inside(b));
					neighs += f.tile(b) == TileEnum::Empty;
				}
			}
			if (neighs == 9)
			{
				CAGE_ASSERT(f.tile(a) == TileEnum::Empty);
				CAGE_ASSERT(f.extra(a).empty());
				f.tile(a) = TileEnum::Waypoint;
				f.extra(a).push_back(std::string() + (Stringizer() + "{\"class\":\"waypoint\",\"destinationFloor\":" + (randomChance() < 0.3 ? 0 : f.level / 2) + "}").value.c_str());
			}
		}

		// the butcher
		if (f.level > 40 && randomChance() < 0.03)
		{
			Generate g = Generate(f.level, f.level / 3);
			g.magic = 0;
			g.ranged = 0;
			g.support = 0;
			Monster mr = generateMonster(g);
			mr.name = "The Butcher";
			mr.icon = "boss";
			mr.algorithm = "butcher";
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(std::move(mr));
		}

		placeMonsters(f, 0);
	}
}

Floor generateFloor(uint32 level, uint32 maxLevel)
{
	if (maxLevel == 0 || level > maxLevel)
		CAGE_THROW_ERROR(Exception, "invalid floor level");
	Floor f;
	f.level = level;
	if (level == 0)
		generateShopFloor(f, maxLevel);
	else if (isLevelBoss(level))
		generateBossFloor(f);
	else if (level < 4 || (level > 30 && randomChance() < 0.03))
		generateSingleRoomFloor(f);
	else if (level > 20 && randomChance() < 0.03)
		generateMazeFloor(f);
	else if (level > 10 && randomChance() < 0.03)
		generateStripesFloor(f);
	else
		generateGenericFloor(f);
	CAGE_ASSERT(countCells(f, TileEnum::Spawn) == 1);
	CAGE_ASSERT(countCells(f, TileEnum::Stairs) == 1);
	return f;
}
