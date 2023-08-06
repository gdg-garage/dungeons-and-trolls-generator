#include "dnt.h"

namespace
{
	Monster generateRandomMonster(const Generate &generate)
	{
		Monster mr;

		const uint32 level = generate.power;

		if (randomChance() < 0.5)
			mr.attributes[AttributeEnum::Strength] = randomRange(1u, level);
		if (randomChance() < 0.5)
			mr.attributes[AttributeEnum::Intelligence] = randomRange(1u, level);
		if (randomChance() < 0.5)
			mr.attributes[AttributeEnum::Dexterity] = randomRange(1u, level);
		if (randomChance() < 0.5)
			mr.attributes[AttributeEnum::Willpower] = randomRange(1u, level);
		if (randomChance() < 0.5)
			mr.attributes[AttributeEnum::Constitution] = randomRange(1u, level);

		mr.attributes[AttributeEnum::Life] = randomRange(10u, level * 2 + 10);
		mr.attributes[AttributeEnum::Mana] = randomRange(10u, level * 2 + 10);
		mr.attributes[AttributeEnum::Stamina] = randomRange(10u, level * 2 + 10);

		const auto &equip = [&](SlotEnum slot)
		{
			Generate g = generate;
			g.slot = slot;
			Item item = generateItem(g);
			mr.addPower(item, 1);
			mr.equippedItems.push_back(std::move(item));
		};
		equip(SlotEnum::MainHand);
		if (randomChance() < 0.3)
			equip(SlotEnum::OffHand);
		if (randomChance() < 0.3)
			equip(SlotEnum::Head);
		if (randomChance() < 0.6)
			equip(SlotEnum::Body);
		if (randomChance() < 0.3)
			equip(SlotEnum::Legs);
		if (randomChance() < 0.1)
			equip(SlotEnum::Neck);

		if (randomChance() < 0.5)
			mr.onDeath.push_back(generateItem(generate));
		if (randomChance() < 0.1)
			mr.onDeath.push_back(generateMonster(generate));
		if (randomChance() < 0.1)
			mr.onDeath.push_back(generateSkill(Generate(generate.level, generate.powerOffset(), SlotEnum::MainHand)));

		mr.score = numeric_cast<uint32>(mr.goldCost);

		return mr;
	}

	Monster generateUndead(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.name = "Undead";
		mr.icon = "undead";
		mr.faction = "hell";
		return mr;
	}

	Monster generateDemon(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.name = "Demon";
		mr.icon = "demon";
		mr.faction = "hell";
		return mr;
	}

	Monster generateBandit(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.name = "Bandit";
		mr.icon = "bandit";
		mr.faction = "bandit";
		return mr;
	}
}

Monster generateMonster(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot == SlotEnum::None);

	// https://www.wolframalpha.com/input?i=plot+sin%28floor%28x%2B89%29+*+2+*+pi+*+0.029%29+-+sin%28floor%28x%2B69%29+*+2+*+pi+*+0.017%29+%3B+x+%3D+0+..+20
	const Real hellish = cage::sin((generate.level + 89) * Rads::Full() * 0.029) - cage::sin((generate.level + 69) * Rads::Full() * 0.017) + (randomChance() - 0.5) * 0.2;
	if (hellish > 0)
	{
		if (generate.magic < 0.5)
			return generateUndead(generate);
		else
			return generateDemon(generate);
	}
	return generateBandit(generate);
}

Monster generateChest(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot == SlotEnum::None);

	Monster mr;

	mr.name = "Chest";
	mr.icon = "chest";
	mr.algorithm = "none";
	mr.faction = "neutral";
	mr.attributes[AttributeEnum::Life] = 1;

	const uint32 cnt = randomRange(5u, 10u);
	for (uint32 i = 0; i < cnt; i++)
		mr.onDeath.push_back(generateItem(Generate(generate.level, generate.powerOffset())));

	return mr;
}
