#include "dnt.h"

namespace
{
	std::string thingJson(const Thing &thing)
	{
		std::string r;

		r += "\"affixRelevances\":[\n";
		for (const Affix &affix : thing.affixes)
			r += (Stringizer() + affix.relevance + ",").value.c_str();
		removeLastComma(r);
		r += "],\n"; // /affixRelevances

		r += (Stringizer() + "\"powersCount\":" + thing.powersCount + ",").value.c_str();
		r += (Stringizer() + "\"powerWeight\":" + thing.powerWeight + ",").value.c_str();
		r += (Stringizer() + "\"powerTotal\":" + thing.powerTotal + ",").value.c_str();
		r += (Stringizer() + "\"goldCost\":" + thing.goldCost + ",").value.c_str();

		removeLastComma(r);
		return "{" + r + "}";
	}

	const char *attributeName(AttributeEnum attribute)
	{
		switch (attribute)
		{
			case AttributeEnum::Strength:
				return "strength";
			case AttributeEnum::Dexterity:
				return "dexterity";
			case AttributeEnum::Intelligence:
				return "intelligence";
			case AttributeEnum::Willpower:
				return "willpower";
			case AttributeEnum::Constitution:
				return "constitution";
			case AttributeEnum::SlashArmor:
				return "slashArmor";
			case AttributeEnum::PiercingArmor:
				return "piercingArmor";
			case AttributeEnum::FireResist:
				return "fireResist";
			case AttributeEnum::PoisonResist:
				return "poisonResist";
			case AttributeEnum::ElectricResist:
				return "electricResist";
			case AttributeEnum::Life:
				return "life";
			case AttributeEnum::Mana:
				return "mana";
			case AttributeEnum::Stamina:
				return "stamina";
			case AttributeEnum::Scalar:
				return "scalar";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown attribute enum");
		}
	}

	template<class AttributesValueMapping>
	requires(std::is_same_v<AttributesValueMapping, AttributesValueMappingInt> || std::is_same_v<AttributesValueMapping, AttributesValueMappingFloat>)
	std::string attributesValueMappingJson(const AttributesValueMapping &attributesValues)
	{
		std::string r;
		for (const auto &it : attributesValues)
		{
			const std::string k = attributeName(it.first);
			const std::string v = (Stringizer() + it.second).value.c_str();
			r += "\"" + k + "\":" + v + ",";
		}
		removeLastComma(r);
		return "{" + r + "}";
	}

	const char *damageTypeName(DamageTypeEnum damageType)
	{
		switch (damageType)
		{
			case DamageTypeEnum::None:
				return "none";
			case DamageTypeEnum::Slash:
				return "slash";
			case DamageTypeEnum::Piercing:
				return "piercing";
			case DamageTypeEnum::Fire:
				return "fire";
			case DamageTypeEnum::Poison:
				return "poison";
			case DamageTypeEnum::Electric:
				return "electric";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown damage type enum");
		}
	}

	const char *skillTargetName(SkillTargetEnum skillTarget)
	{
		switch (skillTarget)
		{
			case SkillTargetEnum::None:
				return "none";
			case SkillTargetEnum::Position:
				return "position";
			case SkillTargetEnum::Character:
				return "character";
			case SkillTargetEnum::Item:
				return "item";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown skill target enum");
		}
	}

	std::string skillCostJson(const SkillCost &cost)
	{
		std::string r;
		if (cost.life != 0)
			r += (Stringizer() + "\"life\":" + cost.life + ",").value.c_str();
		if (cost.mana != 0)
			r += (Stringizer() + "\"mana\":" + cost.mana + ",").value.c_str();
		if (cost.stamina != 0)
			r += (Stringizer() + "\"stamina\":" + cost.stamina + ",").value.c_str();
		removeLastComma(r);
		return "{" + r + "}";
	}

	std::string skillAttributesEffectsJson(const SkillAttributesEffects &effects)
	{
		std::string r;
		for (const auto &it : effects)
		{
			const std::string k = attributeName(it.first);
			const std::string v = attributesValueMappingJson(it.second);
			r += "\"" + k + "\":" + v + ",";
		}
		removeLastComma(r);
		return "{" + r + "}";
	}

	const char *slotName(SlotEnum slot)
	{
		switch (slot)
		{
			case SlotEnum::None:
				return "none";
			case SlotEnum::MainHand:
				return "mainHand";
			case SlotEnum::OffHand:
				return "offHand";
			case SlotEnum::Head:
				return "head";
			case SlotEnum::Body:
				return "body";
			case SlotEnum::Legs:
				return "legs";
			case SlotEnum::Neck:
				return "neck";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown slot enum");
		}
	}
}

std::string exportVariant(const Variant &variant)
{
	return std::visit(
		[](const auto &arg) -> std::string
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::string>)
				return arg;
			else if constexpr (std::is_same_v<T, Skill>)
				return exportSkill(arg) + "\n";
			else if constexpr (std::is_same_v<T, Item>)
				return exportItem(arg) + "\n";
			else if constexpr (std::is_same_v<T, Monster>)
				return exportMonster(arg) + "\n";
			else
				static_assert(always_false<T>, "non-exhaustive visitor");
		},
		variant);
}

std::string exportSkill(const Skill &skill)
{
	std::string json;
	json += "{\n";
	json += "\"class\":\"skill\",\n";
	json += std::string() + "\"name\":\"" + skill.name.c_str() + "\",\n";
	json += std::string() + "\"target\":\"" + skillTargetName(skill.target) + "\",\n";
	json += std::string() + "\"cost\":" + skillCostJson(skill.cost) + ",\n";
	json += std::string() + "\"range\":" + attributesValueMappingJson(skill.range) + ",\n";
	json += std::string() + "\"radius\":" + attributesValueMappingJson(skill.radius) + ",\n";
	json += std::string() + "\"duration\":" + attributesValueMappingJson(skill.duration) + ",\n";
	json += std::string() + "\"damageAmount\":" + attributesValueMappingJson(skill.damageAmount) + ",\n";
	json += std::string() + "\"damageType\":\"" + damageTypeName(skill.damageType) + "\",\n";
	json += std::string() + "\"casterAttributes\":" + skillAttributesEffectsJson(skill.casterAttributes) + ",\n";
	json += std::string() + "\"targetAttributes\":" + skillAttributesEffectsJson(skill.targetAttributes) + ",\n";

	json += "\"casterFlags\":[\n";
	for (const std::string &flag : skill.casterFlags)
		json += flag + ",\n";
	removeLastComma(json);
	json += "],\n"; // /casterFlags

	json += "\"targetFlags\":[\n";
	for (const std::string &flag : skill.targetFlags)
		json += flag + ",\n";
	removeLastComma(json);
	json += "],\n"; // /targetFlags

#ifdef CAGE_DEBUG
	json += "\"_debug\":" + thingJson(skill) + ",\n";
#endif // CAGE_DEBUG

	removeLastComma(json);
	json += "}"; // /root
	return json;
}

std::string exportItem(const Item &item)
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
		json += exportSkill(skill) + ",\n";
	removeLastComma(json);
	json += "],\n"; // /skills

	json += "\"flags\":[\n";
	for (const std::string &flag : item.flags)
		json += flag + ",\n";
	removeLastComma(json);
	json += "],\n"; // /casterFlags

	json += (Stringizer() + "\"buyPrice\":" + item.buyPrice + ",").value.c_str();

#ifdef CAGE_DEBUG
	json += "\"_debug\":" + thingJson(item) + ",\n";
#endif // CAGE_DEBUG

	removeLastComma(json);
	json += "}"; // /root
	return json;
}

std::string exportMonster(const Monster &monster)
{
	std::string json;
	json += "{\n";
	json += "\"class\":\"monster\",\n";
	json += std::string() + "\"name\":\"" + monster.name.c_str() + "\",\n";
	json += "\"attributes\":" + attributesValueMappingJson(monster.attributes) + ",\n";

	json += "\"equippedItems\":[\n";
	for (const Item &item : monster.equippedItems)
		json += exportItem(item) + ",\n";
	removeLastComma(json);
	json += "],\n"; // /equippedItems

	json += "\"onDeath\":[\n";
	for (const auto &it : monster.onDeath)
		json += exportVariant(it) + ",\n";
	removeLastComma(json);
	json += "],\n"; // onDeath

	json += (Stringizer() + "\"score\":" + monster.score + ",").value.c_str();

#ifdef CAGE_DEBUG
	json += "\"_debug\":" + thingJson(monster) + ",\n";
#endif // CAGE_DEBUG

	removeLastComma(json);
	json += "}"; // /root
	return json;
}
