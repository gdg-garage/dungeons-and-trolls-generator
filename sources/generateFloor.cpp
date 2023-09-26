#include <unordered_map>

#include "dnt.h"

#include <cage-core/noiseFunction.h>

Item itemGeneric(const Generate &generate);
Item itemPrimitive(SlotEnum slot);
Item itemSprayCan();
Monster monsterGeneric(const Generate &generate);
std::string floorBossName(uint32 level);
Monster monsterChest(const Generate &generate);
Monster monsterFloorBoss(uint32 level);
Holder<PointerRange<Monster>> generateAntiHeroes();
Monster monsterButcher(uint32 level);
Monster monsterWitch(uint32 level);
Monster monsterTemplar(uint32 level);
Monster monsterZergling(uint32 level);
Monster monsterHealingTotem(uint32 level);
Monster monsterHydra(uint32 level);
Monster monsterSatyr(uint32 level);
Monster monsterElemental(uint32 level);
Monster monsterVandal(uint32 level);

namespace std
{
	template<>
	struct hash<Vec2i>
	{
		std::size_t operator()(Vec2i x) const noexcept { return cage::hash(x[0]) ^ cage::hash(x[1]); }
	};
}

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

	// makes path toward the goal irrespective of any tiles in the way
	void directPathReplace(Floor &f, Vec2i a, Vec2i b, TileEnum what, TileEnum with)
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

	// finds shortest walkable path
	void shortestPathReplace(Floor &f, Vec2i a, Vec2i b, TileEnum what, TileEnum with)
	{
		CAGE_ASSERT(occupancy(f.tile(a)) != OccupancyEnum::Block);
		CAGE_ASSERT(occupancy(f.tile(b)) != OccupancyEnum::Block);
		std::vector<Vec2i> open;
		open.reserve(f.tiles.size());
		open.push_back(a);
		std::vector<bool> visited;
		visited.resize(f.tiles.size(), false);
		std::unordered_map<Vec2i, Vec2i> prev;
		const auto &idx = [&](Vec2i i) -> uint32 { return i[1] * f.width + i[0]; };
		while (!open.empty())
		{
			const Vec2i p = open[0];
			open.erase(open.begin());
			CAGE_ASSERT(occupancy(f.tile(p)) != OccupancyEnum::Block);
			if (p == b)
				break;
			if (visited[idx(p)])
				continue;
			visited[idx(p)] = true;
			const Vec2i ps[4] = {
				p + Vec2i(0, -1),
				p + Vec2i(0, +1),
				p + Vec2i(-1, 0),
				p + Vec2i(+1, 0),
			};
			for (Vec2i n : ps)
			{
				if (visited[idx(n)])
					continue;
				if (occupancy(f.tile(n)) == OccupancyEnum::Block)
					continue;
				if (prev.count(n) == 0)
					prev[n] = p;
				open.push_back(n);
			}
		}
		CAGE_ASSERT(prev.count(b) == 1);
		while (prev.count(b) == 1)
		{
			if (f.tile(b) == what)
				f.tile(b) = with;
			b = prev[b];
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

	// verify that all walkable areas are connected together
	bool isConnected(const Floor &f_)
	{
		Floor f;
		f.width = f_.width;
		f.height = f_.height;
		f.tiles = f_.tiles;
		for (TileEnum &t : f.tiles)
			if (occupancy(t) != OccupancyEnum::Block)
				t = TileEnum::Empty;
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

	void surroundWithDecorations(Floor &f, Vec2i center, const char *decor)
	{
		const Vec2i ps[] = {
			center + Vec2i(-1, -1),
			center + Vec2i(-1, +0),
			center + Vec2i(-1, +1),
			center + Vec2i(+0, -1),
			center + Vec2i(+0, +1),
			center + Vec2i(+1, -1),
			center + Vec2i(+1, +0),
			center + Vec2i(+1, +1),
		};
		for (Vec2i i : ps)
		{
			if (f.tile(i) == TileEnum::Empty)
			{
				f.tile(i) = TileEnum::Decoration;
				f.extra(i).push_back(Decoration{ decor });
			}
		}
	}

	void placeMonsters(Floor &f, sint32 powerOffset)
	{
		const uint32 a = f.level / 2 + 1;
		const uint32 b = f.width * f.height / 50;
		const uint32 cnt = min(min(randomRange(min(a, b), max(a, b)), 40u), countCells(f, TileEnum::Empty));
		for (uint32 i = 0; i < cnt; i++)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterGeneric(Generate(f.level, powerOffset)));
		}
	}

	void placeSpawnAndStairs(Floor &f)
	{
		const bool hasSpawn = countCells(f, TileEnum::Spawn);
		const bool hasStairs = countCells(f, TileEnum::Stairs);
		if (hasSpawn || hasStairs)
		{
			if (hasSpawn && !hasStairs)
			{
				Vec2i a = findAny(f, TileEnum::Spawn);
				Vec2i b = findFarthest(f, a, TileEnum::Empty);
				f.tile(b) = TileEnum::Stairs;
			}
			if (!hasSpawn && hasStairs)
			{
				Vec2i a = findAny(f, TileEnum::Stairs);
				Vec2i b = findFarthest(f, a, TileEnum::Empty);
				f.tile(b) = TileEnum::Spawn;
			}
			return;
		}

		if (f.level > 15 && randomChance() < 0.3)
		{
			Vec2i a = findAny(f, TileEnum::Empty);
			Vec2i b = findFarthest(f, a, TileEnum::Empty);
			if (randomChance() < 0.5)
				std::swap(a, b);
			f.tile(a) = TileEnum::Stairs;
			f.tile(b) = TileEnum::Spawn;
		}
		else
		{
			const Vec2i a = findAny(f, TileEnum::Empty);
			const Vec2i b = findFarthest(f, a, TileEnum::Empty);
			const Vec2i c = findFarthest(f, b, TileEnum::Empty);
			f.tile(b) = TileEnum::Spawn;
			f.tile(c) = TileEnum::Stairs;
		}
	}

	void placeLavaRiver(Floor &f)
	{
		const uint32 w = f.width;
		const uint32 h = f.height;

		uint32 x1 = randomRange(5u, w - 15);
		uint32 x2 = x1 + randomRange(3u, 10u);
		CAGE_ASSERT(x2 < w - 5);
		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = x1; x < x2; x++)
			{
				if (f.tile(x, y) != TileEnum::Empty)
					continue;
				f.tile(x, y) = TileEnum::Decoration;
				f.extra(x, y).push_back(Decoration{ "lava" });
				Skill sk({});
				sk.name = "Lava";
				sk.duration[AttributeEnum::Constant] = 1000000;
				sk.damageAmount[AttributeEnum::Constant] = randomRange(5.0, 20.0);
				sk.damageType = DamageTypeEnum::Fire;
				sk.caster.flags.groundEffect = true;
				f.extra(x, y).push_back(std::move(sk));
			}
			switch (randomRange(0u, 5u))
			{
				case 0:
					if (x1 > 5)
					{
						x1--;
						x2--;
					}
					break;
				case 1:
					if (x2 < w - 5)
					{
						x1++;
						x2++;
					}
					break;
				default:
					break;
			}
		}
	}

	void placeWitchCoven(Floor &f)
	{
		const uint32 w = f.width;
		const uint32 h = f.height;
		const uint32 r = randomRange(6u, 10u);
		CAGE_ASSERT(r * 2 + 6 < w && r * 2 + 6 < h);
		const Vec2i c = Vec2i(randomRange(r + 3, w - r - 4), randomRange(r + 3, h - r - 4));
		const Vec2i a = Vec2i(c - r - 2);
		const Vec2i b = Vec2i(c + r + 3);

		// generate circular room
		const auto &dist = [=](Vec2i p) -> Real { return distance(Vec2(p), Vec2(c)); };
		for (uint32 y = a[1]; y < b[1]; y++)
		{
			for (uint32 x = a[0]; x < b[0]; x++)
			{
				const Vec2i p = Vec2i(x, y);
				const Real d = dist(p);
				if (d > r)
					f.tile(x, y) = TileEnum::Outside;
				else if (d > r - 1)
				{
					f.tile(x, y) = TileEnum::Decoration;
					f.extra(x, y).push_back(Decoration{ "rune" });
				}
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		// cauldron
		f.tile(c) = TileEnum::Decoration;
		f.extra(c).push_back(Decoration{ "cauldron" });

		// witches
		for (uint32 i = 0; i < 4; i++)
		{
			const Vec2i ps[4] = {
				c + Vec2i(0, -1),
				c + Vec2i(0, +1),
				c + Vec2i(-1, 0),
				c + Vec2i(+1, 0),
			};
			Vec2i p = ps[i];
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterWitch(f.level));
		}
	}

	void placeCastle(Floor &f)
	{
		const Vec2i c = Vec2i(f.width, f.height) / 2;
		const Vec2i a = Vec2i(c - 5);
		const Vec2i b = Vec2i(c + 6);

		// basic shape
		for (uint32 y = a[1]; y < b[1]; y++)
		{
			for (uint32 x = a[0]; x < b[0]; x++)
			{
				const Vec2i p = Vec2i(x, y);
				CAGE_ASSERT(f.extra(p).empty());
				CAGE_ASSERT(f.tile(p) == TileEnum::Empty || f.tile(p) == TileEnum::Outside);
				const uint32 layer = max(cage::abs(p[0] - c[0]), cage::abs(p[1] - c[1]));
				switch (layer)
				{
					case 0:
						f.tile(p) = randomChance() < 0.4 ? TileEnum::Stairs : TileEnum::Spawn;
						break;
					case 1:
						f.tile(p) = TileEnum::Empty;
						break;
					case 2:
					case 4:
						f.tile(p) = TileEnum::Outside;
						break;
					case 3:
					{
						f.tile(p) = TileEnum::Decoration;
						f.extra(p).push_back(Decoration{ "rug" });
						break;
					}
					case 5:
					{
						f.tile(p) = TileEnum::Decoration;
						f.extra(p).push_back(Decoration{ "moat" });
						Skill sk({});
						sk.name = "Moat";
						sk.duration[AttributeEnum::Constant] = 1000000;
						sk.caster.flags.groundEffect = true;
						sk.target.flags.stun = true;
						f.extra(p).push_back(std::move(sk));
						break;
					}
				}
			}
		}

		// hallways
		f.tile(c + Vec2i(0, -2)) = TileEnum::Empty;
		f.tile(c + Vec2i(0, +2)) = TileEnum::Empty;
		f.tile(c + Vec2i(-4, 0)) = TileEnum::Empty;
		f.tile(c + Vec2i(+4, 0)) = TileEnum::Empty;
	}

	void placeCorridors(Floor &f)
	{
		CAGE_ASSERT(countCells(f, TileEnum::Empty) > 0);
		static constexpr TileEnum tmp = TileEnum::Placeholder;
		CAGE_ASSERT(countCells(f, tmp) == 0);
		seedReplace(f, findAny(f, TileEnum::Empty), TileEnum::Empty, tmp);
		while (true)
		{
			const Vec2i a = findAny(f, TileEnum::Empty);
			if (a == Vec2i(-1))
				break;
			const Vec2i b = findNearest(f, a, tmp);
			const Vec2i c = findNearest(f, b, TileEnum::Empty);
			directPathReplace(f, b, c, TileEnum::Outside, tmp);
			f.tile(c) = TileEnum::Empty;
			seedReplace(f, c, TileEnum::Empty, tmp);
		}
		rectReplace(f, Vec2i(), Vec2i(f.width, f.height), tmp, TileEnum::Empty); // restore back
	}

	void makeShopItems(TileExtra &extra, uint32 maxLevel)
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
					gen = Generate(randomRange(max(maxLevel * 3 / 4, 1u), maxLevel), 0); // stronger than default
					break;
				case 2:
					gen = Generate(maxLevel, -randomRange(0u, maxLevel / 4)); // any features, but possibly slightly weak
					break;
				case 3:
					gen = Generate(maxLevel, -randomRange(0u, maxLevel)); // any features, but possibly very weak
					break;
			}
			CAGE_ASSERT(gen.level > 0);
			CAGE_ASSERT(gen.power > 0);
			bool unidentified = false;
			Real costMult = 1;
			const Real uniChance = clamp((Real(maxLevel) - 50) * 0.01, 0, 0.5);
			if (gen.power > 10 && randomChance() < uniChance)
			{
				unidentified = true;
				costMult = pow(randomChance() + 0.9, 5);
				Real powMult = pow(randomChance() + 0.4, 2);
				gen.power = numeric_cast<sint32>(gen.power * powMult);
				CAGE_ASSERT(gen.power > 0);
			}
			Item item = itemGeneric(gen);
			item.unidentified = unidentified;
			item.buyPrice = numeric_cast<uint32>(item.goldCost * costMult);
			extra.push_back(std::move(item));
		}
		if (maxLevel > LevelSummoning && maxLevel > LevelDuration && randomChance() < 0.02)
		{
			Item item = itemSprayCan();
			item.buyPrice = randomRange(10000, 100000);
			extra.push_back(std::move(item));
		}
	}

	Decoration floorBossPedestalDecoration(uint32 bossIndex)
	{
		switch (bossIndex)
		{
			case 1:
				return Decoration{ "sword" };
			case 2:
				return Decoration{ "bow" };
			case 3:
				return Decoration{ "scythe" };
			case 4:
				return Decoration{ "magic" };
			case 5:
				return Decoration{ "duration" };
			case 6:
				return Decoration{ "support" };
			case 7:
				return Decoration{ "poison" };
			case 8:
				return Decoration{ "ground" };
			case 9:
				return Decoration{ "stun" };
			case 10:
				return Decoration{ "summon" };
			case 11:
				return Decoration{ "electric" };
			default:
				return Decoration{ "scroll", "Certificate of excellence" };
		}
	}

	void generateShopFloor(Floor &f, uint32 maxLevel)
	{
		const uint32 portals = levelToBossIndex(maxLevel - 1);
		const uint32 w = 11 + portals * 2, h = 9;
		resizeFloor(f, Vec2i(w, h));

		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				if (y == 0 || x == 0 || y + 1 == h || x + 1 == w)
					f.tile(x, y) = TileEnum::Wall;
				else if (y == 1 || y + 2 == h)
				{
					f.tile(x, y) = TileEnum::Decoration;
					makeShopItems(f.extra(x, y), maxLevel);
				}
				else
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		f.tile(4, 2) = TileEnum::Decoration;
		f.extra(4, 2).push_back(Decoration{ "rack" });
		for (uint32 i = 0; i < 3; i++)
			f.extra(4, 2).push_back(itemPrimitive(SlotEnum::Body));
		f.tile(6, 2) = TileEnum::Decoration;
		f.extra(6, 2).push_back(Decoration{ "rack" });
		for (uint32 i = 0; i < 3; i++)
			f.extra(6, 2).push_back(itemPrimitive(SlotEnum::Head));
		f.tile(4, 6) = TileEnum::Decoration;
		f.extra(4, 6).push_back(Decoration{ "rack" });
		for (uint32 i = 0; i < 3; i++)
			f.extra(4, 6).push_back(itemPrimitive(SlotEnum::MainHand));
		f.tile(6, 6) = TileEnum::Decoration;
		f.extra(6, 6).push_back(Decoration{ "rack" });
		for (uint32 i = 0; i < 3; i++)
			f.extra(6, 6).push_back(itemPrimitive(SlotEnum::Legs));

		f.tile(4, h / 2) = TileEnum::Spawn;
		f.tile(6, h / 2) = TileEnum::Stairs;
		for (uint32 i = 1; i < portals + 1; i++)
		{
			const sint32 x = 6 + i * 2;
			f.tile(x, h / 2) = TileEnum::Waypoint;
			f.extra(x, h / 2).push_back(Waypoint{ bossIndexToLevel(i) + 1 });
			f.tile(x, 2) = TileEnum::Decoration;
			f.extra(x, 2).push_back(Decoration{ "pedestal" });
			f.extra(x, 2).push_back(Decoration{ "trophy", floorBossName(bossIndexToLevel(i)) });
			f.tile(x, 6) = TileEnum::Decoration;
			f.extra(x, 6).push_back(Decoration{ "pedestal" });
			f.extra(x, 6).push_back(floorBossPedestalDecoration(i));
		}

		if (maxLevel > 100)
		{
			f.tile(1, 2) = TileEnum::Decoration;
			f.extra(1, 2).push_back(Decoration{ "pedestal" });
			f.extra(1, 2).push_back(Decoration{ "barbarian", "Vít" });
			f.tile(1, 6) = TileEnum::Decoration;
			f.extra(1, 6).push_back(Decoration{ "pedestal" });
			f.extra(1, 6).push_back(Decoration{ "sorcerer", "Eva" });
			f.tile(w - 2, 2) = TileEnum::Decoration;
			f.extra(w - 2, 2).push_back(Decoration{ "pedestal" });
			f.extra(w - 2, 2).push_back(Decoration{ "druid", "Šimon" });
			f.tile(w - 2, 6) = TileEnum::Decoration;
			f.extra(w - 2, 6).push_back(Decoration{ "pedestal" });
			f.extra(w - 2, 6).push_back(Decoration{ "necromancer", "Tomáš" });
		}
		else
		{
			f.tile(1, 4) = TileEnum::Decoration;
			f.extra(1, 4).push_back(Decoration{ "sign", "Welcome to Dungeons and Trolls" });
			f.tile(w - 2, 4) = TileEnum::Decoration;
			f.extra(w - 2, 4).push_back(Decoration{ "sign", "Motivational Note: 404 (Not Found)" });
		}
	}

	void makeCircularRoom(Floor &f, uint32 r)
	{
		const uint32 w = f.width;
		const uint32 h = f.height;
		const auto &dist = [=](Vec2i p) -> Real { return distance(Vec2(p[0], p[1] * 2), Vec2(w / 2, h)); };

		// empty space
		for (uint32 y = 0; y < h; y++)
		{
			for (uint32 x = 0; x < w; x++)
			{
				const Vec2i p = Vec2i(x, y);
				if (dist(p) < r)
					f.tile(p) = TileEnum::Empty;
				else
					f.tile(p) = TileEnum::Outside;
			}
		}

		// random pillars
		const uint32 cnt = min(levelToBossIndex(f.level) + 1, 10u);
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

	std::vector<Vec2i> findDoors(const Floor &f)
	{
		std::vector<Vec2i> res;
		const uint32 cnt = f.width * f.height;
		for (uint32 i = 0; i < cnt; i++)
			if (f.tiles[i] == TileEnum::Door)
				res.push_back(Vec2i(i % f.width, i / f.width));
		return res;
	}

	Decoration makeRandomSign()
	{
		static constexpr const char *texts[] = {
			"(1/10) The path of the righteous man is beset on all sides",
			"(2/10) By the inequities of the selfish and the tyranny of evil men",
			"(3/10) Blessed is he who, in the name of charity and good will",
			"(4/10) Shepherds the weak through the valley of darkness",
			"(5/10) For he is truly his brother's keeper and the finder of lost children",
			"(6/10) And I will strike down upon thee",
			"(7/10) With great vengeance and furious anger",
			"(8/10) Those who attempt to poison and destroy my brothers",
			"(9/10) And you will know my name is the Lord",
			"(10/10) When I lay my vengeance upon thee",
			"Go left",
			"Go right",
			"Stay away",
			"R.I.P",
			"This is the land of outlaws. You are legally obliged to NOT enter",
			"Horror ahead. You have been warned.",
			"In the name of God: DO NOT ENTER",
			"Walk by faith, not by sight.",
			"Stay on the path. It's not your concern.",
			"You know what's good about no soap? you can smell a hijacker from a mile away.",
			"Nobody",
			"Look up!",
			"Watch out",
			"No entry",
			"Caution",
			"Danger",
			"Private Property",
			"No Trespassing",
			"Trap Door Ahead",
			"Live, Laugh, Leave",
			"Another one bites the dust",
			"Cellar",
			"Eat the Rich",
			"The Name’s Bond. Sommelier Bond.",
			"I Got Problems, But 2 Million Bottles Ain’t One",
			"Ominous sounds playing now...",
			"Beware the Minotaur",
			"Plague",
			"Enter at your own risk",
			"No going back",
			"ᚽᛗ᛫᛫ᛁᛘ᛫ᛁᚾᛁᛁᛒᛁᚾᛁ᛫ᛁᛗ᛫ᛁ᛫ᛘ᛫ᛁᚽ",
			"No return beyond this point",
			"Everybody is a hero, a lover, a fool, a villain.",
			"Fool's errand",
			"Words offer the means to meaning, and for those who will listen, the enunciation of truth.",
			"People shouldn’t be afraid of their government. Governments should be afraid of their people.",
			"It seems strange that my life should end in such a terrible place, but for three years I had roses, and apologized to no one.",
			"If you’re looking for the guilty, you need only look into a mirror.",
			"Love your rage, not your cage.",
			"They say that life’s a game, & then they take the board away.",
			"Your pretty empire took so long to build, now, with a snap of history’s fingers, down it goes.",
			"Our masters have not heard the people’s voice for generations and it is much, much louder than they care to remember.",
			"Artists use lies to tell the truth.",
			"You wear a mask for so long, you forget who you were beneath it.",
			"The past can’t hurt you anymore, not unless you let it.",
			"Knowledge, like air, is vital to life. Like air, no one should be denied it.",
			"Happiness is the most insidious prison of all.",
			"We’re all mad here. I’m mad. You’re mad.",
			"Imagination is the only weapon in the war with reality.",
			"Only a few find the way, some don’t recognize it when they do – most… don’t ever want to.",
			"I don’t see how he can ever finish, if he doesn’t begin.",
			"I am not crazy; my reality is just different from yours.",
			"Every adventure requires a first step.",
			"Not all who wander are lost.",
			"Well, some go this way, and some go that way. But as for me, myself, personally, I prefer the short-cut.",
			"If you don’t know where you are going any road can take you there.",
			"Can you stand on your head?",
			"We are all victims in-waiting.",
			"It is not despair, for despair is only for those who see the end beyond all doubt. We do not.",
			"You cannot pass. You shall not pass.",
			"He that breaks a thing to find out what it is has left the path of wisdom.",
			"There is a bottom, just beyond light and knowledge.",
			"The treacherous are ever distrustful.",
			"Json, motherfucker, do you speak it?",
			"That’s how you’re gonna beat ’em. They keep underestimating you.",
			"And when you gone, you stay gone or you be gone.",
			"Sometimes I have sex with my parent on the stairs. I know its wrong on multiple levels.",
			"You call it an elevator. I call it a lift. We were raised differently.",
			"Where is the IT guy? He probably ransomware.",
			"Have you ever tried blind-folded archery? You dont know what you are missing.",
		};
		return Decoration{ "sign", texts[randomRange({}, sizeof(texts) / sizeof(texts[0]))] };
	}

	void generateBossFloor(Floor &f)
	{
		CAGE_ASSERT(isLevelBoss(f.level));
		const uint32 r = (min(f.level * 2, 50u) + 10 + randomRange(0u, min(f.level / 3, 30u) + 2)) / 2;
		uint32 w = r * 2 + 7;
		uint32 h = r + 6;
		resizeFloor(f, Vec2i(w, h));
		makeCircularRoom(f, r);

		f.tile(w / 2, 2) = TileEnum::Spawn;
		f.tile(w / 2, 3) = TileEnum::Decoration;
		f.extra(w / 2, 3).push_back(Decoration{ "rug" });

		f.tile(w / 2, h - 3) = TileEnum::Door;
		f.tile(w / 2, h - 2) = TileEnum::Stairs;

		f.tile(3, h / 2) = TileEnum::Door;
		f.tile(2, h / 2) = TileEnum::Chest;
		f.extra(2, h / 2).push_back(monsterChest(Generate(f.level, f.level / 10)));

		f.tile(w - 4, h / 2) = TileEnum::Door;
		f.tile(w - 3, h / 2) = TileEnum::Chest;
		f.extra(w - 3, h / 2).push_back(monsterChest(Generate(f.level, f.level / 10)));

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		const auto &generateBossMonster = [&]()
		{
			Monster mr = monsterFloorBoss(f.level);
			mr.onDeath.push_back(Key{ findDoors(f) });
			return mr;
		};
		f.tile(w / 2, h / 2) = TileEnum::Monster;
		f.extra(w / 2, h / 2).push_back(generateBossMonster());

		placeMonsters(f, f.level / 20);
	}

	void generateAntiHeroesFloor(Floor &f)
	{
		CAGE_ASSERT(f.level == 100);
		const uint32 r = 30;
		uint32 w = r * 2 + 11;
		uint32 h = r + 7;
		resizeFloor(f, Vec2i(w, h));
		makeCircularRoom(f, r);

		f.tile(1, h / 2) = TileEnum::Spawn;
		for (uint32 x = 2; x < 2 + 4; x++)
		{
			f.tile(x, h / 2) = TileEnum::Decoration;
			f.extra(x, h / 2).push_back(Decoration{ "rug" });
		}
		for (uint32 x = w - 2 - 4; x < w - 2; x++)
			f.tile(x, h / 2) = TileEnum::Door;
		f.tile(w - 2, h / 2) = TileEnum::Stairs;

		findOutlineWalls(f);

		cutoutFloor(f);
		w = f.width;
		h = f.height;

		std::vector<Vec2i> doors = findDoors(f);
		for (Monster &hero : generateAntiHeroes())
		{
			if (!doors.empty())
			{
				hero.onDeath.push_back(Key{ { doors.back() } });
				doors.pop_back();
			}
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(std::move(hero));
		}

		placeMonsters(f, 10);
	}

	void generateDungeonLayout(Floor &f)
	{
		const uint32 w = f.width;
		const uint32 h = f.height;

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

		if (randomChance() < 0.07)
		{
			switch (randomRange(0, 2))
			{
				case 0: // witches
					if (f.level > 50)
					{
						placeWitchCoven(f);
						placeCorridors(f); // after
						return;
					}
					break;
				case 1: // castle
					if (f.level > 70)
					{
						placeCorridors(f); // before
						placeCastle(f);
						return;
					}
					break;
				default:
					CAGE_THROW_CRITICAL(Exception, "switch value out of range");
			}
		}

		placeCorridors(f);
	}

	void generateNaturalCavesLayout(Floor &f)
	{
		if (f.level <= 10)
			return generateDungeonLayout(f);

		NoiseFunctionCreateConfig cfg;
		cfg.seed = randomRange((uint32)0, (uint32)m);
		cfg.fractalType = NoiseFractalTypeEnum::Fbm;
		cfg.frequency = randomRange(0.02, 0.07);
		Holder<NoiseFunction> noise = newNoiseFunction(cfg);

		for (uint32 y = 1; y < f.height - 1; y++)
		{
			for (uint32 x = 1; x < f.width - 1; x++)
			{
				if (noise->evaluate(Vec2(x, y)) > 0)
					f.tile(x, y) = TileEnum::Empty;
			}
		}

		// witches
		if (f.level > 50 && randomChance() < 0.07)
			placeWitchCoven(f);

		// corridors
		placeCorridors(f);
	}

	void generateShiftedGridLayout(Floor &f)
	{
		if (f.level <= 20)
			return generateDungeonLayout(f);

		std::vector<Vec2i> open;
		open.reserve(f.tiles.size());
		{
			const Vec2i p = Vec2i(f.width, f.height) / 2;
			open.push_back(p);
			f.tile(p) = TileEnum::Empty;
		}

		/*
		*  N
		* W E
		*  S
		*/
		const auto &walk = [&](Vec2i p, const char *path)
		{
			CAGE_ASSERT(f.tile(p) == TileEnum::Empty);
			CAGE_ASSERT(path && *path);
			while (true)
			{
				switch (*path++)
				{
					case 0:
						return;
					case 'N':
						p += Vec2i(0, 1);
						break;
					case 'S':
						p += Vec2i(0, -1);
						break;
					case 'W':
						p += Vec2i(-1, 0);
						break;
					case 'E':
						p += Vec2i(1, 0);
						break;
				}
				if (p[0] <= 1 || p[0] >= f.width - 1 || p[1] <= 1 || p[1] >= f.height - 1)
					return;
				if (*path == 0 && f.tile(p) == TileEnum::Outside)
					open.push_back(p);
				f.tile(p) = TileEnum::Empty;
			}
		};

		while (!open.empty())
		{
			const Vec2i p = open[0];
			open.erase(open.begin());
			walk(p, "ESWN");
			walk(p, "WWNNNWWWN");
			walk(p, "ENNEEENNN");
			walk(p, "SSSWWWSSW");
			walk(p, "SEEESSSEE");
		}
	}

	void generateBoobsLayout(Floor &f)
	{
		if (f.level <= 25)
			return generateDungeonLayout(f);

		const uint32 w = f.width;
		const uint32 h = f.height;
		const Real r = length(Vec2(w, h)) * interpolate(0.25, 0.35, randomChance());
		const Vec2 c1 = Vec2(w * 1 / 10, h * 1 / 3);
		const Vec2 c2 = Vec2(w * 9 / 10, h * 2 / 3);

		for (uint32 y = 1; y < h - 1; y++)
		{
			for (uint32 x = 1; x < w - 1; x++)
			{
				const Vec2i p = Vec2i(x, y);
				const Vec2 pf = Vec2(p);
				if (min(distance(pf, c1), distance(pf, c2)) < r)
					f.tile(p) = TileEnum::Empty;
				else
					f.tile(p) = TileEnum::Outside;
			}
		}

		// witches
		if (f.level > 50 && randomChance() < 0.07)
			placeWitchCoven(f);

		placeCorridors(f);
	}

	void generateHollowRoomLayout(Floor &f)
	{
		if (f.level <= 25)
			return generateDungeonLayout(f);

		const uint32 w = f.width;
		const uint32 h = f.height;

		rectReplace(f, Vec2i(1), Vec2i(w, h) - 1, TileEnum::Outside, TileEnum::Empty);
		rectReplace(f, Vec2i(w, h) / 5 + 1, Vec2i(w, h) * 4 / 5 - 2, TileEnum::Empty, TileEnum::Outside);

		// witches
		if (f.level > 50 && randomChance() < 0.07)
			placeWitchCoven(f);

		placeCorridors(f);
	}

	void generateSingleRoomLayout(Floor &f)
	{
		if (f.level >= 4 && f.level <= 30)
			return generateDungeonLayout(f);

		const uint32 w = f.width;
		const uint32 h = f.height;

		rectReplace(f, Vec2i(1), Vec2i(w, h) - 1, TileEnum::Outside, TileEnum::Empty);

		// random pillars
		if (w > 20 && h > 10)
		{
			const uint32 cnt = randomRange(5u, 15u);
			for (uint32 i = 0; i < cnt; i++)
			{
				const uint32 x = randomRange(3u, w - 3);
				const uint32 y = randomRange(3u, h - 3);
				const Vec2i p = Vec2i(x, y);
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
			}
		}

		// castle
		if (f.level > 70 && randomChance() < 0.07)
			placeCastle(f);

		// always lava river
		if (f.level > 60)
			placeLavaRiver(f);
	}

	void generateTunnelsLayout(Floor &f)
	{
		if (f.level <= 35)
			return generateDungeonLayout(f);

		const uint32 w = f.width;
		const uint32 h = f.height;

		const uint32 cnt = randomRange(10, 30);
		for (uint32 i = 0; i < cnt; i++)
			f.tile(randomRange(1u, w - 1), randomRange(1u, h - 1)) = TileEnum::Empty;

		placeCorridors(f);
	}

	void generateMazeLayout(Floor &f)
	{
		if (f.level <= 40)
			return generateDungeonLayout(f);

		const uint32 w = f.width;
		const uint32 h = f.height;

		static constexpr Vec2i ns[4] = {
			Vec2i(-1, 0),
			Vec2i(+1, 0),
			Vec2i(0, -1),
			Vec2i(0, +1),
		};

		f.tile(w / 2, h / 2) = TileEnum::Empty;
		std::vector<Vec2i> open;
		open.reserve(f.tiles.size());
		open.push_back(Vec2i(w / 2 + 0, h / 2 + 1));
		open.push_back(Vec2i(w / 2 + 1, h / 2 + 0));
		open.push_back(Vec2i(w / 2 + 0, h / 2 - 1));
		open.push_back(Vec2i(w / 2 - 1, h / 2 + 0));
		while (!open.empty())
		{
			const uint32 j = randomRange(std::size_t(0), open.size());
			const Vec2i p = open[j];
			open.erase(open.begin() + j);
			if (f.tile(p) != TileEnum::Outside)
				continue;
			{
				uint32 en = 0;
				for (Vec2i n : ns)
					en += f.tile(p + n) == TileEnum::Empty;
				if (en != 1)
					continue;
			}
			f.tile(p) = TileEnum::Empty;
			for (Vec2i n : ns)
			{
				const Vec2i c = p + n;
				if (c[0] < 2 || c[0] > w - 3 || c[1] < 2 || c[1] > h - 3)
					continue;
				if (f.tile(c) != TileEnum::Outside)
					continue;
				open.push_back(c);
			}
		}

		// castle
		if (f.level > 70 && randomChance() < 0.07)
			placeCastle(f);
	}

	void generateStripesLayout(Floor &f)
	{
		if (f.level <= 50)
			return generateDungeonLayout(f);

		uint32 x = 1;
		while (x + 5 < f.width)
		{
			const uint32 s = randomRange(1, 5);
			rectReplace(f, Vec2i(x, 1), Vec2i(x + s, f.height - 1), TileEnum::Outside, TileEnum::Empty);
			if (x > 1)
			{
				const uint32 y = randomRange(3u, f.height - 5);
				f.tile(x - 1, y) = TileEnum::Empty;
			}
			x += s + 1;
		}
	}

	void generateGenericFloor(Floor &f, uint32 maxLevel)
	{
		{
			auto [w, h] = defaultFloorSize(f.level);
			resizeFloor(f, Vec2i(w, h));
		}

		if (f.level < 4)
			generateSingleRoomLayout(f);
		else
		{
			switch (randomRange(0u, 12u))
			{
				case 0:
					generateDungeonLayout(f);
					break;
				case 1:
					generateShiftedGridLayout(f);
					break;
				case 2:
					generateBoobsLayout(f);
					break;
				case 3:
					generateHollowRoomLayout(f);
					break;
				case 4:
					generateSingleRoomLayout(f);
					break;
				case 5:
					generateTunnelsLayout(f);
					break;
				case 6:
					generateMazeLayout(f);
					break;
				case 7:
					generateStripesLayout(f);
					break;
				default:
					generateNaturalCavesLayout(f);
					break;
			}
		}

		findOutlineWalls(f);
		cutoutFloor(f);
		placeSpawnAndStairs(f);

		// lava river
		if (f.level > 60 && randomChance() < 0.05)
			placeLavaRiver(f);

		// highlight path
		if (f.level > 30 && randomChance() < 0.05)
		{
			CAGE_ASSERT(countCells(f, TileEnum::Placeholder) == 0);
			const Vec2i a = findAny(f, TileEnum::Spawn);
			const Vec2i b = findAny(f, TileEnum::Stairs);
			shortestPathReplace(f, a, b, TileEnum::Empty, TileEnum::Placeholder);
			for (uint32 i = 0; i < f.tiles.size(); i++)
			{
				if (f.tiles[i] == TileEnum::Placeholder)
				{
					f.tiles[i] = TileEnum::Decoration;
					f.extras[i].push_back(Decoration{ "rope" });
				}
			}
		}

		// the vandal
		if (f.level > 10 && randomChance() < 0.05)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterVandal(f.level));
		}

		// the butcher
		if (f.level > 40 && randomChance() < 0.05)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterButcher(maxLevel)); // maxLevel -> the butcher does not scale down
			surroundWithDecorations(f, p, "bones");
		}

		// the satyr
		if (f.level > 50 && randomChance() < 0.05)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterSatyr(f.level));
			surroundWithDecorations(f, p, "garland");
		}

		// the elemental
		if (f.level > 50 && randomChance() < 0.05)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterElemental(f.level));
			surroundWithDecorations(f, p, "rune");
		}

		// the hydra
		if (f.level > 70 && randomChance() < 0.05)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Monster;
			f.extra(p).push_back(monsterHydra(f.level));
			surroundWithDecorations(f, p, "bones");
		}

		// templars
		if (f.level > 70 && randomChance() < 0.05)
		{
			const Vec2i spawn = findAny(f, TileEnum::Spawn);
			const uint32 cnt = randomRange(3u, 7u);
			for (uint32 i = 0; i < cnt; i++)
			{
				Vec2i p;
				for (uint32 a = 0; a < 100; a++)
				{
					p = findAny(f, TileEnum::Empty);
					if (distance(p, spawn) < 20)
						break;
				}

				f.tile(p) = TileEnum::Monster;
				f.extra(p).push_back(monsterTemplar(f.level));

				Vec2i ps[4] = {
					p + Vec2i(-1, +0),
					p + Vec2i(+0, -1),
					p + Vec2i(+1, +0),
					p + Vec2i(+0, +1),
				};
				for (uint32 a = 0; a < 3; a++)
					std::swap(ps[randomRange(0, 4)], ps[randomRange(0, 4)]);
				for (Vec2i n : ps)
				{
					if (f.tile(n) == TileEnum::Empty)
					{
						f.tile(n) = TileEnum::Decoration;
						f.extra(n).push_back(Decoration{ "cross" });
						break;
					}
				}
			}
		}

		// random chest
		if (f.level > 15 && randomChance() < 0.4)
		{
			const Vec2i p = findAny(f, TileEnum::Empty);
			f.tile(p) = TileEnum::Chest;
			f.extra(p).push_back(monsterChest(Generate(f.level, f.level / 20)));
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
			if (neighs == 9) // make sure there is a path around the waypoint
			{
				CAGE_ASSERT(f.tile(a) == TileEnum::Empty);
				CAGE_ASSERT(f.extra(a).empty());
				f.tile(a) = TileEnum::Waypoint;
				const auto &dest = [](uint32 level) -> uint32
				{
					switch (randomRange(0, 4))
					{
						case 0:
							return 0;
						case 1:
							return randomRange(0u, level + 1);
						case 2:
							return bossIndexToLevel(level);
						case 3:
							return bossIndexToLevel(levelToBossIndex(level) + 1);
						default:
							CAGE_THROW_CRITICAL(Exception, "invalid value");
					}
				};
				f.extra(a).push_back(Waypoint{ dest(f.level) });
			}
		}

		// signs
		if (f.level > 1 && isHorrorFloor(f.level) < 0.5)
		{
			for (uint32 i = 0; i < 3; i++)
			{
				if (randomChance() < 0.3)
				{
					const Vec2i p = findAny(f, TileEnum::Empty);
					f.tile(p) = TileEnum::Decoration;
					f.extra(p).push_back(makeRandomSign());
				}
			}
		}

		// monsters
		if (f.level > 70 && randomChance() < 0.05)
		{
			// zerg infestation
			for (uint32 i = 0; i < f.tiles.size(); i++)
			{
				if (f.tiles[i] == TileEnum::Empty && randomChance() < 0.2)
				{
					f.tiles[i] = TileEnum::Monster;
					f.extras[i].push_back(monsterZergling(f.level));
				}
			}
		}
		else
			placeMonsters(f, 0);

		// spikes traps
		if (f.level > 60 && randomChance() < 0.05)
		{
			for (uint32 i = 0; i < f.tiles.size(); i++)
			{
				if (f.tiles[i] == TileEnum::Empty && randomChance() < 0.2)
				{
					f.tiles[i] = TileEnum::Decoration;
					f.extras[i].push_back(Decoration{ "spikesTrap" });
					Skill sk({});
					sk.name = "Spikes";
					sk.duration[AttributeEnum::Constant] = 1000000;
					sk.damageAmount[AttributeEnum::Constant] = randomRange(10.0, 30.0);
					sk.damageType = DamageTypeEnum::Pierce;
					sk.caster.flags.groundEffect = true;
					f.extras[i].push_back(std::move(sk));
				}
			}
		}

		// poison rot
		if (f.level > 60 && randomChance() < 0.05)
		{
			for (uint32 i = 0; i < f.tiles.size(); i++)
			{
				if (f.tiles[i] == TileEnum::Empty && randomChance() < 0.05)
				{
					f.tiles[i] = TileEnum::Decoration;
					f.extras[i].push_back(Decoration{ "rottenPile" });
					Skill sk({});
					sk.name = "Rot";
					sk.radius[AttributeEnum::Constant] = randomRange(1.0, 4.0);
					sk.duration[AttributeEnum::Constant] = 1000000;
					sk.damageAmount[AttributeEnum::Constant] = randomRange(3.0, 15.0);
					sk.damageType = DamageTypeEnum::Poison;
					sk.caster.flags.groundEffect = true;
					f.extras[i].push_back(std::move(sk));
				}
			}
		}

		// healing totems
		if (f.level > 60 && randomChance() < 0.05)
		{
			for (uint32 i = 0; i < f.tiles.size(); i++)
			{
				if (f.tiles[i] == TileEnum::Empty && randomChance() < 0.05)
				{
					f.tiles[i] = TileEnum::Decoration;
					f.extras[i].push_back(monsterHealingTotem(f.level));
				}
			}
		}
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
	else if (level == 100)
		generateAntiHeroesFloor(f);
	else if (isLevelBoss(level))
		generateBossFloor(f);
	else
		generateGenericFloor(f, maxLevel);
	CAGE_ASSERT(countCells(f, TileEnum::Spawn) == 1);
	CAGE_ASSERT(countCells(f, TileEnum::Stairs) == 1);
	CAGE_ASSERT(isConnected(f));
	return f;
}
