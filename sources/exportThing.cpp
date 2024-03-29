#include "dnt.h"

namespace
{
	template<bool Properties>
	std::string thingJson(const Thing &thing)
	{
		std::string r;

		r += (Stringizer() + "\"power\":" + thing.generate.power + ",").value.c_str();
		if constexpr (Properties)
		{
			if (thing.generate.power > 0)
			{
				r += (Stringizer() + "\"level\":" + thing.generate.level + ",").value.c_str();
				r += (Stringizer() + "\"magic\":" + thing.generate.magic + ",").value.c_str();
				r += (Stringizer() + "\"ranged\":" + thing.generate.ranged + ",").value.c_str();
				r += (Stringizer() + "\"defensive\":" + thing.generate.defensive + ",").value.c_str();
				r += (Stringizer() + "\"support\":" + thing.generate.support + ",").value.c_str();
			}
		}

		r += (Stringizer() + "\"weightedRoll\":" + thing.weightedRoll() + ",").value.c_str();
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
			case AttributeEnum::SlashResist:
				return "slashResist";
			case AttributeEnum::PierceResist:
				return "pierceResist";
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
			case AttributeEnum::Constant:
				return "constant";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown attribute enum");
		}
	}

	template<class AttributesValueMapping>
	requires(std::is_same_v<AttributesValueMapping, AttributesValuesList> || std::is_same_v<AttributesValueMapping, AttributesEquationFactors>)
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
			case DamageTypeEnum::Pierce:
				return "pierce";
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
			//case SkillTargetEnum::Item:
			//	return "item";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown skill target enum");
		}
	}

	std::string skillAttributesJson(const SkillAttributes &effects)
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
				CAGE_THROW_ERROR(Exception, "item slot cannot be none");
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

	std::string exportSkillEffects(const SkillEffects &effects)
	{
		std::string json;
		json += "{\n";

		if (!effects.attributes.empty())
			json += std::string() + "\"attributes\":" + skillAttributesJson(effects.attributes) + ",\n";

		{
			json += "\"flags\":{\n";
#define FLAG(F) json += std::string() + "\"" + #F + "\":" + (Stringizer() + effects.flags.F).value.c_str() + ",";
			FLAG(movement);
			FLAG(knockback);
			FLAG(stun);
			FLAG(groundEffect);
#undef FLAG
			removeLastComma(json);
			json += "},\n";
		}

		if (!effects.summons.empty())
		{
			json += "\"summons\":[\n";
			for (const auto &flag : effects.summons)
				json += exportVariant(flag) + ",\n";
			removeLastComma(json);
			json += "],\n";
		}

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportSkill(const Skill &skill)
	{
		std::string json;
		json += "{\n";
		json += std::string() + "\"name\":\"" + skill.name + "\",\n";
		if (skill.targetType != SkillTargetEnum::None)
			json += std::string() + "\"target\":\"" + skillTargetName(skill.targetType) + "\",\n";
		if (!skill.cost.empty())
			json += std::string() + "\"cost\":" + attributesValueMappingJson(skill.cost) + ",\n";
		if (!skill.range.empty())
			json += std::string() + "\"range\":" + attributesValueMappingJson(skill.range) + ",\n";
		if (!skill.radius.empty())
			json += std::string() + "\"radius\":" + attributesValueMappingJson(skill.radius) + ",\n";
		if (!skill.duration.empty())
			json += std::string() + "\"duration\":" + attributesValueMappingJson(skill.duration) + ",\n";
		if (!skill.damageAmount.empty())
			json += std::string() + "\"damageAmount\":" + attributesValueMappingJson(skill.damageAmount) + ",\n";
		if (skill.damageType != DamageTypeEnum::None)
			json += std::string() + "\"damageType\":\"" + damageTypeName(skill.damageType) + "\",\n";
		if (std::string j = exportSkillEffects(skill.caster); j.length() > 3)
			json += std::string() + "\"casterEffects\":" + j + ",\n";
		if (std::string j = exportSkillEffects(skill.target); j.length() > 3)
			json += std::string() + "\"targetEffects\":" + j + ",\n";

		{
			json += "\"flags\":{\n";
#define FLAG(F) json += std::string() + "\"" + #F + "\":" + (Stringizer() + skill.flags.F).value.c_str() + ",";
			FLAG(requiresOutOfCombat);
			FLAG(requiresLineOfSight);
			FLAG(passive);
#undef FLAG
			removeLastComma(json);
			json += "},\n";
		}

#ifdef CAGE_DEBUG
		json += "\"_debug\":" + thingJson<false>(skill) + ",\n";
#endif // CAGE_DEBUG

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportSkill(const Skill &skill, const AttributesValuesList &attr)
	{
		std::string json;
		json += "{\n";
		json += std::string() + "\"name\":\"" + skill.name + "\",\n";
		if (!skill.cost.empty())
			json += (Stringizer() + "\"cost\":" + attributesSum(skill.cost) + ",\n").value.c_str();
		if (!skill.range.empty())
			json += (Stringizer() + "\"range\":" + attributesSum(skill.range, attr) + ",\n").value.c_str();
		if (!skill.radius.empty())
			json += (Stringizer() + "\"radius\":" + attributesSum(skill.radius, attr) + ",\n").value.c_str();
		if (!skill.duration.empty())
			json += (Stringizer() + "\"duration\":" + attributesSum(skill.duration, attr) + ",\n").value.c_str();
		if (!skill.damageAmount.empty())
			json += (Stringizer() + "\"damageAmount\":" + attributesSum(skill.damageAmount, attr) + ",\n").value.c_str();
		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportItem(const Item &item)
	{
		std::string json;
		json += "{\n";
		json += std::string() + "\"name\":\"" + item.name + "\",\n";
		json += std::string() + "\"icon\":\"" + item.icon.c_str() + "\",\n";
		json += std::string() + "\"slot\":\"" + slotName(item.slot) + "\",\n";
		if (item.unidentified)
			json += (Stringizer() + "\"unidentified\":true,").value.c_str();
		if (!item.requirements.empty())
			json += "\"requirements\":" + attributesValueMappingJson(item.requirements) + ",\n";
		if (!item.attributes.empty())
			json += "\"attributes\":" + attributesValueMappingJson(item.attributes) + ",\n";

		if (!item.skills.empty())
		{
			json += "\"skills\":[\n";
			for (const Skill &skill : item.skills)
				json += exportSkill(skill) + ",\n";
			removeLastComma(json);
			json += "],\n";
		}

		if (item.buyPrice > 1e-7)
			json += (Stringizer() + "\"price\":" + item.buyPrice + ",").value.c_str();

#ifdef CAGE_DEBUG
		json += "\"_debug\":" + thingJson<false>(item) + ",\n";
#endif // CAGE_DEBUG

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string monsterStatistics(const Monster &monster)
	{
		std::string json;
		json += "{\n";

		const AttributesValuesList totalAttributes = monsterTotalAttributes(monster);
		json += "\"totalAttributes\":" + attributesValueMappingJson(totalAttributes) + ",\n";

		json += "\"attacks\":[\n";
		for (const Item &it : monster.equippedItems)
			for (const Skill &sk : it.skills)
				if (sk.damageType != DamageTypeEnum::None)
					json += exportSkill(sk, totalAttributes) + ",\n";
		removeLastComma(json);
		json += "],\n"; // attacks

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportMonster(const Monster &monster)
	{
		std::string json;
		json += "{\n";
		json += std::string() + "\"name\":\"" + monster.name + "\",\n";
		json += std::string() + "\"icon\":\"" + monster.icon.c_str() + "\",\n";
		json += std::string() + "\"algorithm\":\"" + monster.algorithm.c_str() + "\",\n";
		json += std::string() + "\"faction\":\"" + monster.faction.c_str() + "\",\n";
		if (!monster.attributes.empty())
			json += "\"attributes\":" + attributesValueMappingJson(monster.attributes) + ",\n";

		if (!monster.equippedItems.empty())
		{
			json += "\"equippedItems\":[\n";
			for (const Item &item : monster.equippedItems)
				json += exportItem(item) + ",\n";
			removeLastComma(json);
			json += "],\n";
		}

		if (!monster.onDeath.empty())
		{
			json += "\"onDeath\":[\n";
			for (const auto &it : monster.onDeath)
				json += exportVariant(it) + ",\n";
			removeLastComma(json);
			json += "],\n";
		}

		json += (Stringizer() + "\"score\":" + monster.score + ",").value.c_str();

#ifdef CAGE_DEBUG
		json += "\"_debug\":" + thingJson<true>(monster) + ",\n";
		json += "\"_statistics\":" + monsterStatistics(monster) + ",\n";
#endif // CAGE_DEBUG

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportDecoration(const Decoration &decor)
	{
		std::string json;
		json += "{\n";
		if (!decor.name.empty())
			json += std::string() + "\"name\":\"" + decor.name + "\",\n";
		if (!decor.icon.empty())
			json += std::string() + "\"icon\":\"" + decor.icon.c_str() + "\",\n";
		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	std::string exportWaypoint(const Waypoint &waypoint)
	{
		const String s = Stringizer() + "{\"destinationFloor\":" + waypoint.destinationFloor + "}";
		return s.c_str();
	}

	std::string exportKey(const Key &key)
	{
		std::string json;
		json += "{\n";
		json += "\"doors\":[\n";
		for (const Vec2i &door : key.doors)
		{
			json += "{\n";
			json += (Stringizer() + "\"positionX\":" + door[0] + ",\n").value.c_str();
			json += (Stringizer() + "\"positionY\":" + door[1] + "\n").value.c_str();
			json += "},"; // /door
		}
		removeLastComma(json);
		json += "]\n"; // /doors
		json += "}"; // /root
		return json;
	}
}

std::string exportVariant(const Variant &variant)
{
	return std::visit(
		[](const auto &arg) -> std::string
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Skill>)
				return "{\"skill\":" + exportSkill(arg) + "}\n";
			else if constexpr (std::is_same_v<T, Item>)
				return "{\"item\":" + exportItem(arg) + "}\n";
			else if constexpr (std::is_same_v<T, Monster>)
				return "{\"monster\":" + exportMonster(arg) + "}\n";
			else if constexpr (std::is_same_v<T, Decoration>)
				return "{\"decoration\":" + exportDecoration(arg) + "}\n";
			else if constexpr (std::is_same_v<T, Waypoint>)
				return "{\"waypoint\":" + exportWaypoint(arg) + "}\n";
			else if constexpr (std::is_same_v<T, Key>)
				return "{\"key\":" + exportKey(arg) + "}\n";
			else
				static_assert(always_false<T>, "non-exhaustive visitor");
		},
		variant);
}
