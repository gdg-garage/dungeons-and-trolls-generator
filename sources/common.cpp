#include "dnt.h"

#include <cmath>

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

const char *attributeName(AttributeEnum attribute)
{
	switch (attribute)
	{
		case AttributeEnum::Strength:
			return "strength";
		case AttributeEnum::Dexterity:
			return "dexterity";
		case AttributeEnum::Inteligence:
			return "inteligence";
		case AttributeEnum::Willpower:
			return "willpower";
		case AttributeEnum::Constitution:
			return "constitution";
		case AttributeEnum::SlashArmor:
			return "slash_armor";
		case AttributeEnum::PiercingArmor:
			return "piercing_armor";
		case AttributeEnum::FireResist:
			return "fire_resist";
		case AttributeEnum::PoisonResist:
			return "poison_resist";
		case AttributeEnum::ElectricResist:
			return "electric_resist";
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
			return "main_hand";
		case SlotEnum::OffHand:
			return "off_hand";
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

const char *tileName(TileEnum tile)
{
	switch (tile)
	{
		case TileEnum::Empty:
			return "empty";
		case TileEnum::Decoration:
			return "decoration";
		case TileEnum::Spawn:
			return "spawn";
		case TileEnum::Waypoint:
			return "waypoint";
		case TileEnum::Stairs:
			return "stairs";
		case TileEnum::Door:
			return "door";
		case TileEnum::Chest:
			return "chest";
		case TileEnum::Monster:
			return "monster";
		case TileEnum::Wall:
			return "wall";
		case TileEnum::Outside:
			return "outside";
		default:
			CAGE_THROW_CRITICAL(Exception, "unknown tile enum");
	}
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
