#include "dnt.h"

namespace
{
	Item generateBasicItem(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot != SlotEnum::None);
		Item item(generate);
		item.slot = generate.slot;

		if (item.addPower(0.5, AffixEnum::Prefix, "Skilled") > 0.7)
		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 0.8);
			item.skills.push_back(std::move(sk));
		}

		return item;
	}
}

namespace
{
	Item generateSword(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.target = SkillTargetEnum::Character;
			sk.range[AttributeEnum::Scalar] = 1;
			sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Strong");
			sk.damageType = DamageTypeEnum::Slash;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
			sk.updateName("Attack");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Sword");
		return item;
	}

	Item generateScythe(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.radius[AttributeEnum::Scalar] = interpolate(1.0, 3.0, sk.addPower(1, AffixEnum::Prefix, "Wide"));
			sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Strong") * 0.3;
			sk.damageType = DamageTypeEnum::Slash;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
			sk.updateName("Attack");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Scythe");
		return item;
	}

	Item generateBow(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.target = SkillTargetEnum::Character;
			sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Far") * 0.1;
			sk.range[AttributeEnum::Scalar] = 4;
			sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 1, "Accurate") * 0.5;
			sk.damageType = DamageTypeEnum::Piercing;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
			sk.updateName("Attack");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Bow");
		return item;
	}

	Item generateShield(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		item.updateName("Shield");
		return item;
	}

	Item generateHelmet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head);
		Item item = generateBasicItem(generate);

		item.updateName("Helmet");
		return item;
	}

	Item generateBodyArmor(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 1, "Refreshing");
			sk.casterFlags.push_back(SkillAlone);
			sk.updateName("Rest");
			item.addPower(sk, 0.7);
			item.skills.push_back(std::move(sk));
		}

		item.updateName(generate.defensive < 0.5 ? "Leather Armor" : "Plated Armor");
		return item;
	}

	Item generateBoots(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Legs);
		Item item = generateBasicItem(generate);

		if (randomChance() < 0.7)
		{
			Skill sk(generate);
			sk.target = SkillTargetEnum::Position;
			sk.range[AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 1, "Enduring") * 0.1;
			sk.range[AttributeEnum::Scalar] = 1;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 5);
			sk.updateName("Walk");
			item.addPower(sk, 0.6);
			item.skills.push_back(std::move(sk));
		}
		else
		{
			Skill sk(generate);
			sk.target = SkillTargetEnum::Position;
			sk.range[AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 1, "Enduring") * 0.075;
			sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Fast") * 0.075;
			sk.range[AttributeEnum::Scalar] = 2;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
			sk.updateName("Run");
			item.addPower(sk, 0.8);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Boots");
		return item;
	}
}

namespace
{
	Item generateRestoringTattoos(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head || generate.slot == SlotEnum::Body);
		Item item(generate); // no generateBasicItem -> no stats
		item.slot = generate.slot;

		{
			Skill sk(generate);
			if (randomChance() < 0.5)
				sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = interpolate(1.0, 6.0, sk.addPower(1, AffixEnum::Prefix, "Refreshing"));
			else
				sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Scalar] = interpolate(1.0, 6.0, sk.addPower(1, AffixEnum::Prefix, "Energizing"));
			sk.casterFlags.push_back(SkillPassive);
			sk.updateName("Glow");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Tattoos");
		return item;
	}

	Item generateProtectiveTattoos(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head || generate.slot == SlotEnum::Body);
		Item item(generate); // no generateBasicItem -> no stats
		item.slot = generate.slot;

		if (generate.level > 16)
			item.attributes[AttributeEnum::FireResist] = interpolate(0.0, 10.0, item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (generate.level > 37)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(0.0, 10.0, item.addPower(1, AffixEnum::Prefix, "Antidote"));
		if (generate.level > 79)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(0.0, 10.0, item.addPower(1, AffixEnum::Prefix, "Isolating"));

		item.updateName("Tattoos");
		return item;
	}

	Item generateStaff(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Staff");
		return item;
	}

	Item generateWand(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand || generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName(
			[]()
			{
				switch (randomRange(0, 3))
				{
					case 0:
						return "Wand";
					case 1:
						return "Book";
					case 2:
						return "Scroll";
				}
			}());
		return item;
	}

	Item generateCape(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Energizing") * 0.8;
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 0.7, "Refreshing") * 0.2;
			sk.casterFlags.push_back(SkillAlone);
			sk.updateName("Meditation");
			item.addPower(sk, 0.7);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Cape");
		return item;
	}

	Item generateAmulet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Neck);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Amulet");
		return item;
	}
}

namespace
{
	Item generateTrinket(const Generate &generate)
	{
		Item item = generateBasicItem(generate);
		item.updateName("Trinket");
		return item;
	}
}

Item generateItem(const Generate &generate)
{
	if (generate.slot == SlotEnum::None)
	{
		Generate g = generate;
		g.slot = []()
		{
			switch (randomRange(0, 13))
			{
				case 0:
					return SlotEnum::Neck;
				case 1:
				case 2:
					return SlotEnum::OffHand;
				case 3:
				case 4:
					return SlotEnum::Head;
				case 5:
				case 6:
					return SlotEnum::Legs;
				case 7:
				case 8:
				case 9:
					return SlotEnum::Body;
				default:
					return SlotEnum::MainHand;
			}
		}();
		return generateItem(g);
	}

	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot != SlotEnum::None);

	Candidates<Item> candidates(generate);
	candidates.slotMismatchPenalty = 10;

	static constexpr float H = 0.5;
	static constexpr uint32 Nothing = 0;

	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelSlash }, generateSword);
	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelSlash, LevelAoe }, generateScythe);
	candidates.add(0, 1, 0, 0, SlotEnum::MainHand, { LevelPierce }, generateBow);
	candidates.add(0, 0, 1, 0, SlotEnum::OffHand, { Nothing }, generateShield);
	candidates.add(0, H, H, 0, SlotEnum::Head, { Nothing }, generateHelmet);
	candidates.add(0, H, H, 0, SlotEnum::Body, { Nothing }, generateBodyArmor);
	candidates.add(0, H, H, 0, SlotEnum::Legs, { Nothing }, generateBoots);

	candidates.add(1, H, 0, 1, SlotEnum::Head, { Nothing }, generateRestoringTattoos);
	candidates.add(1, H, 0, 1, SlotEnum::Body, { Nothing }, generateRestoringTattoos);
	candidates.add(1, H, 1, 1, SlotEnum::Head, { Nothing }, generateProtectiveTattoos);
	candidates.add(1, H, 1, 1, SlotEnum::Body, { Nothing }, generateProtectiveTattoos);
	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { Nothing }, generateStaff);
	candidates.add(1, 1, H, H, SlotEnum::MainHand, { Nothing }, generateWand);
	candidates.add(1, H, H, H, SlotEnum::OffHand, { Nothing }, generateWand);
	candidates.add(1, H, 1, 0, SlotEnum::Body, { Nothing }, generateCape);
	candidates.add(1, H, H, 1, SlotEnum::Neck, { Nothing }, generateAmulet);

	candidates.fallback(generateTrinket);
	return candidates.call();
}
