#include "dnt.h"

namespace
{
	SlotEnum randomSlot()
	{
		const uint32 v = randomRange(0, 100);
		if (v < 5)
			return SlotEnum::Neck;
		if (v < 20)
			return SlotEnum::Legs;
		if (v < 35)
			return SlotEnum::Head;
		if (v < 50)
			return SlotEnum::OffHand;
		if (v < 70)
			return SlotEnum::Body;
		return SlotEnum::MainHand;
	}
}

Monster generateMonster(uint32 level, uint32 difficulty)
{
	Monster mr;

	if (randomChance() < 0.5)
		mr.attributes[AttributeEnum::Strength] = randomRange(1u, level);
	if (randomChance() < 0.5)
		mr.attributes[AttributeEnum::Inteligence] = randomRange(1u, level);
	if (randomChance() < 0.5)
		mr.attributes[AttributeEnum::Dexterity] = randomRange(1u, level);
	if (randomChance() < 0.5)
		mr.attributes[AttributeEnum::Willpower] = randomRange(1u, level);
	if (randomChance() < 0.5)
		mr.attributes[AttributeEnum::Constitution] = randomRange(1u, level);

	mr.attributes[AttributeEnum::Life] = randomRange(10u, level * 2 + 10);
	mr.attributes[AttributeEnum::Mana] = randomRange(10u, level * 2 + 10);
	mr.attributes[AttributeEnum::Stamina] = randomRange(10u, level * 2 + 10);

	mr.equippedItems.push_back(generateItem(level, SlotEnum::MainHand));
	if (randomChance() < 0.5)
		mr.equippedItems.push_back(generateItem(level, SlotEnum::OffHand));
	if (randomChance() < 0.5)
		mr.equippedItems.push_back(generateItem(level, SlotEnum::Head));
	if (randomChance() < 0.5)
		mr.equippedItems.push_back(generateItem(level, SlotEnum::Body));
	if (randomChance() < 0.5)
		mr.equippedItems.push_back(generateItem(level, SlotEnum::Legs));

	if (randomChance() < 0.5)
		mr.onDeath.push_back(std::make_unique<Item>(generateItem(level, randomSlot())));

	return mr;
}

std::string monsterJson(const Monster &monster)
{
	std::string json;
	json += "{\n";
	json += std::string() + "\"name\":\"" + monster.name.c_str() + "\",\n";
	json += "\"attributes\":" + attributesValueMappingJson(monster.attributes) + ",\n";

	json += "\"equippedItems\":[\n";
	for (const Item &item : monster.equippedItems)
		json += itemJson(item) + ",\n";
	removeLastComma(json);
	json += "],\n"; // /equippedItems

	json += "\"onDeath\":[\n";
	for (const auto &it : monster.onDeath)
	{
		std::visit(
			[&](auto &&arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, std::unique_ptr<Skill>>)
					json += "{\n\"type\":\"skill\",\n\"data\":" + skillJson(*arg) + "},\n";
				else if constexpr (std::is_same_v<T, std::unique_ptr<Item>>)
					json += "{\n\"type\":\"item\",\n\"data\":" + itemJson(*arg) + "},\n";
				else if constexpr (std::is_same_v<T, std::unique_ptr<Monster>>)
					json += "{\n\"type\":\"monster\",\n\"data\":" + monsterJson(*arg) + "},\n";
				else
					static_assert(false, "non-exhaustive visitor!");
			},
			it);
	}
	removeLastComma(json);
	json += "]\n"; // /onDeath

	json += "}"; // /root
	return json;
}
