#include "dnt.h"

Thing::Thing(const Generate &generate) : generate(generate)
{
	goldCost = 10 + generate.power * 2 + randomRange(0, generate.power);
}

Real Thing::weightedRoll() const
{
	if (totalWeight > 1e-3)
		return totalRolls / totalWeight;
	return 0.5;
}

Real Thing::averageWeight() const
{
	if (totalRolls > 1e-3)
		return totalWeight / totalRolls;
	return 0.5;
}

Real Thing::addPower(Real weight)
{
	return addPower(randomChance(), weight);
}

Real Thing::addPower(Real roll, Real weight)
{
	CAGE_ASSERT(generate.level > 0); // generate was not initialized
	CAGE_ASSERT(roll >= 0 && roll <= 1);
	CAGE_ASSERT(weight >= 0);
	totalRolls += roll * weight;
	totalWeight += weight;
	goldCost *= 1 + interpolate(0.1, 0.5, sqr(roll)) * weight;
	return roll;
}

Real Thing::subtractPower(Real weight, const std::string &affixName, AffixEnum affixPos)
{
	CAGE_ASSERT(generate.level > 0); // generate was not initialized
	CAGE_ASSERT(weight >= 0);
	const Real roll = randomChance();
	totalRolls += roll * weight;
	totalWeight += weight;
	goldCost /= 1 + interpolate(0.1, 0.5, sqr(roll)) * weight;
	addAffix(roll * weight, affixName, affixPos);
	return roll;
}

Real Thing::addPower(Real weight, const std::string &affixName, AffixEnum affixPos)
{
	return addPower(randomChance(), weight, affixName, affixPos);
}

Real Thing::addPower(Real roll, Real weight, const std::string &affixName, AffixEnum affixPos)
{
	addAffix(roll * weight, affixName, affixPos);
	return addPower(roll, weight);
}

void Thing::addOther(const Thing &other, Real weight)
{
	if (other.totalWeight > 1e-3)
	{
		const Real r = other.weightedRoll();
		addPower(r, weight);
		std::string s = other.name;
		{
			auto it = s.find(" With");
			if (it != s.npos)
				s.erase(it);
		}
		addAffix(r * weight, "With " + s, AffixEnum::Suffix);
	}
	else
		goldCost += other.goldCost * weight;
}

void Thing::addAffix(Real relevance, const std::string &affixName, AffixEnum affixPos)
{
	CAGE_ASSERT(affixPos < AffixEnum::_Total);
	CAGE_ASSERT(!name.empty());
	Affix &a = affixes[(uint32)affixPos];
	if (relevance > a.relevance)
	{
		a.relevance = relevance;
		a.name = affixName;
	}
}

void Thing::updateName()
{
	std::string r;
	for (const Affix &a : affixes)
	{
		if (a.name.empty())
			continue;
		if (a.relevance < 0.3)
			continue;
		if (!r.empty())
			r += " ";
		r += a.name;
	}
	if (r.empty())
		r = affixes[(uint32)AffixEnum::Infix].name;
	name = r;
}

void Thing::updateName(const std::string &basicName, Real relevance)
{
	addAffix(relevance, basicName, AffixEnum::Infix);
	updateName();
}

Generate::Generate(sint32 level, sint32 powerOffset, SlotEnum slot) : level(level), power(level + powerOffset), slot(slot)
{
	CAGE_ASSERT(level + powerOffset > 0);
	randomize();
}

void Generate::randomize()
{
	const auto &gen = [this](uint32 minLevel, Real probability) -> Real
	{
		if (level < minLevel)
			return 0;
		const Real tg = randomChance() < probability ? 1 : 0;
		return Generate::randomTowardsTarget(tg);
	};
	magic = gen(LevelMagic, 0.35);
	ranged = gen(LevelRanged, 0.5);
	defensive = gen(LevelDefensive, 0.25);
	support = gen(LevelSupport, 0.15);
}

bool Generate::valid() const
{
	return cage::valid(magic) && cage::valid(ranged) && cage::valid(defensive) && cage::valid(support) && level > 0 && power > 0;
}

sint32 Generate::powerOffset() const
{
	return power - level;
}

Real Generate::randomTowardsTarget(Real tg)
{
	Real a = 0.5;
	Real b = tg;
	if (b < a)
		std::swap(a, b);
	a = randomRange(a, b);
	b = tg;
	if (b < a)
		std::swap(a, b);
	a = randomRange(a, b);
	return a;
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
	const uint32 k = sqrt(8 * level + 1);
	return k * k == 8 * level + 1;
}

uint32 levelToBossIndex(uint32 level)
{
	if (level < 6)
		return 0;
	const uint32 k = sqrt(8 * level + 1);
	return (k - 1) / 2 - 2;
}

uint32 bossIndexToLevel(uint32 index)
{
	if (index == 0)
		return 0;
	index += 2;
	return index * (index + 1) / 2;
}

Real isHorrorFloor(uint32 level)
{
	// https://www.wolframalpha.com/input?i=plot+sin%28floor%28x%2B89%29+*+2+*+pi+*+0.029%29+-+sin%28floor%28x%2B69%29+*+2+*+pi+*+0.017%29+%3B+x+%3D+0+..+20
	const Real horrorish = cage::sin((level + 89) * Rads::Full() * 0.029) - cage::sin((level + 69) * Rads::Full() * 0.017);
	return saturate(horrorish * 0.25 + 0.5 + (randomChance() - 0.5) * 0.05);
}

Real makeAttrFactor(uint32 power, Real roll)
{
	return (power * 0.004 + 0.4) * interpolate(0.8, 1.2, roll);
}

uint32 makeCost(Thing &sk, Real default_)
{
	const Real roll = sk.addPower(0.6, "Thrifty");
	const Real p = default_ * interpolate(1.2, 0.8, roll);
	return numeric_cast<uint32>(max(p, 1));
}

AttributesValuesList monsterTotalAttributes(const Monster &monster)
{
	AttributesValuesList totalAttributes = monster.attributes;
	for (const Item &it : monster.equippedItems)
	{
		for (const auto &at : it.attributes)
			totalAttributes[at.first] += at.second;
		// count ethereal attributes
		for (const auto &sk : it.skills)
			if (sk.flags.passive)
				for (const auto &at : sk.caster.attributes)
					if (at.second.count(AttributeEnum::Constant))
						totalAttributes[at.first] += numeric_cast<sint32>(at.second.at(AttributeEnum::Constant));
	}
	return totalAttributes;
}

Real attributesSum(const AttributesValuesList &attr)
{
	sint32 sum = 0;
	for (const auto &it : attr)
		sum += it.second;
	return sum;
}

bool attributesCompare(const AttributesValuesList &cost, const AttributesValuesList &attr)
{
	for (const auto &it : cost)
		if (!attr.count(it.first) || it.second > attr.at(it.first))
			return false;
	return true;
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
