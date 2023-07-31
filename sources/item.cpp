#include "dnt.h"

Item generateItem(uint32 level, SlotEnum slot)
{
	Item item;
	item.slot = slot;

	if (randomChance() < 0.1)
		item.requirements[AttributeEnum::Strength] = randomRange(1u, level);
	if (randomChance() < 0.1)
		item.requirements[AttributeEnum::Inteligence] = randomRange(1u, level);
	if (randomChance() < 0.1)
		item.requirements[AttributeEnum::Dexterity] = randomRange(1u, level);
	if (randomChance() < 0.1)
		item.requirements[AttributeEnum::Willpower] = randomRange(1u, level);
	if (randomChance() < 0.1)
		item.requirements[AttributeEnum::Constitution] = randomRange(1u, level);

	if (randomChance() < 0.5)
		item.attributes[AttributeEnum::Strength] = randomRange(1u, level);
	if (randomChance() < 0.5)
		item.attributes[AttributeEnum::Inteligence] = randomRange(1u, level);
	if (randomChance() < 0.5)
		item.attributes[AttributeEnum::Dexterity] = randomRange(1u, level);
	if (randomChance() < 0.5)
		item.attributes[AttributeEnum::Willpower] = randomRange(1u, level);
	if (randomChance() < 0.5)
		item.attributes[AttributeEnum::Constitution] = randomRange(1u, level);

	if (randomChance() < 0.75)
		item.skills.push_back(generateSkill(level, slot));
	if (randomChance() < 0.25)
		item.skills.push_back(generateSkill(level, slot));

	return item;
}

std::string itemJson(const Item &item)
{
	std::string json;
	json += "{\n";
	json += "\"class\":\"item\",\n";
	json += std::string() + "\"name\":\"" + item.name.c_str() + "\",\n";
	json += std::string() + "\"slot\":\"" + slotName(item.slot) + "\",\n";
	json += "\"requirements\":" + attributesValueMappingJson(item.requirements) + ",\n";
	json += "\"attributes\":" + attributesValueMappingJson(item.attributes) + ",\n";

	json += "\"skills\":[\n";
	for (const Skill &skill : item.skills)
		json += skillJson(skill) + ",\n";
	removeLastComma(json);
	json += "],\n"; // /skills

	json += "\"flags\":[\n";
	for (const std::string &flag : item.flags)
		json += flag + ",\n";
	removeLastComma(json);
	json += "],\n"; // /casterFlags

	removeLastComma(json);
	json += "}"; // /root
	return json;
}
