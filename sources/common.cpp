#include "dnt.h"

#include <cmath>

namespace
{
	Real skewedChance()
	{
		return randomChance() * randomChance();
	}
}

Real Thing::addPower(Real weight)
{
	return addPower(skewedChance(), weight);
}

Real Thing::addPower(Real roll, Real weight)
{
	CAGE_ASSERT(roll >= 0 && roll <= 1);
	CAGE_ASSERT(weight >= 0);
	powersCount++;
	powerWeight += weight;
	powerTotal += roll * weight;
	goldCost *= 1 + interpolate(0.01, 4.0, sqr(roll)) * weight;
	return roll;
}

Real Thing::addPower(Real weight, AffixEnum affix, const String &name)
{
	return addPower(skewedChance(), weight, affix, name);
}

Real Thing::addPower(Real roll, Real weight, AffixEnum affix, const String &name)
{
	addAffix(roll * weight, affix, name);
	return addPower(roll, weight);
}

void Thing::addPower(const Thing &other, Real weight)
{
	if (other.powerWeight > 1e-3)
		addPower(other.powerTotal / other.powerWeight, weight);
	else
		powersCount++;
	goldCost += other.goldCost;
}

void Thing::addAffix(Real relevance, AffixEnum affix, const String &name)
{
	CAGE_ASSERT(affix < AffixEnum::_Total);
	Affix &a = affixes[(uint32)affix];
	if (relevance > a.relevance)
	{
		a.relevance = relevance;
		a.name = name;
	}
}

String Thing::makeName(const String &basicName, Real relevance) const
{
	Thing t;
	t.affixes = affixes;
	t.addAffix(relevance, AffixEnum::Infix, basicName);
	String r;
	for (const Affix &a : t.affixes)
	{
		if (a.name.empty())
			continue;
		if (a.relevance < 0.4)
			continue;
		if (!r.empty())
			r += " ";
		r += a.name;
	}
	if (r.empty())
		r = t.affixes[(uint32)AffixEnum::Infix].name;
	return r;
}

Generate::Generate(uint32 level, sint32 difficultyOffset) : level(level), difficultyOffset(difficultyOffset)
{
	randomize();
}

Generate::Generate(SlotEnum slot, uint32 level, sint32 difficultyOffset) : slot(slot), level(level), difficultyOffset(difficultyOffset)
{
	randomize();
}

void Generate::randomize()
{
	magic = randomChance();
	ranged = randomChance();
	support = randomChance();
}

uint32 Generate::ll() const
{
	const sint32 l = (sint32)level + difficultyOffset;
	CAGE_ASSERT(l >= 1);
	return l;
}

void removeLastComma(std::string &json)
{
	auto c = json.find_last_of(',');
	if (c == json.npos)
		return; // there is no comma
	for (auto p = c + 1; p < json.size(); p++)
		if (!std::isspace(json[p]))
			return; // there is more data after the comma
	json = json.substr(0, c);
}

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

bool isLevelBoss(uint32 level)
{
	if (level < 6)
		return false;
	const uint32 k = std::sqrt(8 * level + 1);
	return k * k == 8 * level + 1;
}

uint32 levelToBossIndex(uint32 level)
{
	if (level < 6)
		return 0;
	const uint32 k = std::sqrt(8 * level + 1);
	return (k - 1) / 2 - 2;
}

uint32 bossIndexToLevel(uint32 index)
{
	if (index == 0)
		return 0;
	index += 2;
	return index * (index + 1) / 2;
}

namespace
{
	struct BossLevelTest
	{
		BossLevelTest()
		{
			CAGE_ASSERT(!isLevelBoss(0));
			CAGE_ASSERT(!isLevelBoss(1));
			CAGE_ASSERT(!isLevelBoss(2));
			CAGE_ASSERT(!isLevelBoss(3));
			CAGE_ASSERT(!isLevelBoss(4));
			CAGE_ASSERT(!isLevelBoss(5));
			CAGE_ASSERT(isLevelBoss(6));
			CAGE_ASSERT(!isLevelBoss(7));
			CAGE_ASSERT(!isLevelBoss(8));
			CAGE_ASSERT(!isLevelBoss(9));
			CAGE_ASSERT(isLevelBoss(10));

			CAGE_ASSERT(levelToBossIndex(0) == 0);
			CAGE_ASSERT(levelToBossIndex(1) == 0);
			CAGE_ASSERT(levelToBossIndex(2) == 0);
			CAGE_ASSERT(levelToBossIndex(3) == 0);
			CAGE_ASSERT(levelToBossIndex(4) == 0);
			CAGE_ASSERT(levelToBossIndex(5) == 0);
			CAGE_ASSERT(levelToBossIndex(6) == 1);
			CAGE_ASSERT(levelToBossIndex(7) == 1);
			CAGE_ASSERT(levelToBossIndex(8) == 1);
			CAGE_ASSERT(levelToBossIndex(9) == 1);
			CAGE_ASSERT(levelToBossIndex(10) == 2);

			CAGE_ASSERT(bossIndexToLevel(0) == 0);
			CAGE_ASSERT(bossIndexToLevel(1) == 6);
			CAGE_ASSERT(bossIndexToLevel(2) == 10);
			CAGE_ASSERT(bossIndexToLevel(3) == 15);
			CAGE_ASSERT(bossIndexToLevel(4) == 21);
			CAGE_ASSERT(bossIndexToLevel(5) == 28);
			CAGE_ASSERT(bossIndexToLevel(6) == 36);

			uint32 last = 0;
			uint32 count = 0;
			for (uint32 i = 0; i < 100; i++)
			{
				const bool is = isLevelBoss(i);
				if (is)
				{
					CAGE_ASSERT(bossIndexToLevel(levelToBossIndex(i)) == i);
					last = i;
					count++;
				}
				const uint32 boss = levelToBossIndex(i);
				CAGE_ASSERT(bossIndexToLevel(boss) == last);
			}
			CAGE_ASSERT(last == 91);
			CAGE_ASSERT(count == 11);
		}
	} bossLevelTestInstance;
}
