#include "dnt.h"

namespace
{
	Item generateSword(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);

		Item item;
		item.icon = "sword";
		item.slot = SlotEnum::MainHand;
		item.goldCost = randomRange(20, 30);

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(5, 10);
			sk.damageAmount[AttributeEnum::Strength] = sk.addPower(2, AffixEnum::Prefix, "Strong") + 0.5;
			sk.damageAmount[AttributeEnum::Scalar] = interpolate(1u, generate.power, sk.addPower(1, AffixEnum::Prefix, "Powerful"));
			sk.damageType = DamageTypeEnum::Slash;
			sk.range[AttributeEnum::Scalar] = 1;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Attack";
			sk.icon = "attack";
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Sword");
		return item;
	}

	Item generateBow(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);

		Item item;
		item.icon = "bow";
		item.slot = SlotEnum::MainHand;
		item.goldCost = randomRange(25, 35);

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(8, 13);
			sk.damageAmount[AttributeEnum::Dexterity] = sk.addPower(2, AffixEnum::Prefix, "Swift") + 0.5;
			sk.damageAmount[AttributeEnum::Scalar] = interpolate(1u, generate.power, sk.addPower(1, AffixEnum::Prefix, "Powerful"));
			sk.damageType = DamageTypeEnum::Piercing;
			sk.range[AttributeEnum::Strength] = sk.addPower(1, AffixEnum::Prefix, "Long") * 0.2;
			sk.range[AttributeEnum::Scalar] = 3;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Attack";
			sk.icon = "attack";
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Bow");
		return item;
	}

	Item generateShield(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);

		Item item;
		item.icon = "shield";
		item.slot = SlotEnum::OffHand;
		item.goldCost = randomRange(10, 15);

		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Impenetrable"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(1, 6);
			sk.range[AttributeEnum::Scalar] = 1;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.targetFlags.push_back(Knockback);
			sk.name = "Push";
			sk.icon = "attack";
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Shield");
		return item;
	}

	Item generateWand(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand || generate.slot == SlotEnum::OffHand);

		Item item;
		item.icon = "wand";
		item.slot = generate.slot;
		item.goldCost = randomRange(30, 60);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Wand");
		return item;
	}

	Item generateHelmet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head);

		Item item;
		item.icon = "helmet";
		item.slot = SlotEnum::Head;
		item.goldCost = randomRange(15, 25);

		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		item.name = item.makeName("Helmet");
		return item;
	}

	Item generateBodyArmor(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);

		Item item;
		item.icon = "bodyArmor";
		item.slot = SlotEnum::Body;
		item.goldCost = randomRange(30, 50);

		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.power * 2, item.addPower(2, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.power * 2, item.addPower(2, AffixEnum::Prefix, "Impenetrable"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.target = SkillTargetEnum::None;
			sk.casterFlags.push_back(Alone);
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = interpolate(0.5, 1.0, sk.addPower(1));
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = interpolate(1, 10, sk.addPower(0.5));
			sk.name = "Rest";
			sk.icon = "rest";
			item.addPower(sk, 0.5);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Body Armor");
		return item;
	}

	Item generateCape(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);

		Item item;
		item.icon = "cape";
		item.slot = SlotEnum::Body;
		item.goldCost = randomRange(25, 55);

		if (randomChance() < 0.4)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.power * 2, item.addPower(2, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.4)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.power * 2, item.addPower(2, AffixEnum::Prefix, "Impenetrable"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.target = SkillTargetEnum::None;
			sk.casterFlags.push_back(Alone);
			sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Willpower] = interpolate(0.5, 1.0, sk.addPower(1));
			sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Scalar] = interpolate(1, 10, sk.addPower(0.5));
			sk.name = "Meditate";
			sk.icon = "rest";
			item.addPower(sk, 0.5);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Cape");
		return item;
	}

	Item generateBoots(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Legs);

		Item item;
		item.icon = "boots";
		item.slot = SlotEnum::Legs;
		item.goldCost = randomRange(20, 30);

		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.power, item.addPower(1, AffixEnum::Prefix, "Impenetrable"));

		if (randomChance() < 0.7)
		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(2, 4);
			sk.range[AttributeEnum::Strength] = sk.addPower(0.5) * 0.1;
			sk.range[AttributeEnum::Scalar] = 2;
			sk.target = SkillTargetEnum::Position;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Walk";
			sk.icon = "move";
			item.addPower(sk, 0.5);
			item.skills.push_back(std::move(sk));
		}
		else
		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(3, 6);
			sk.range[AttributeEnum::Constitution] = sk.addPower(0.5) * 0.1;
			sk.range[AttributeEnum::Scalar] = 3;
			sk.target = SkillTargetEnum::Position;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Run";
			sk.icon = "move";
			item.addPower(sk, 0.5);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Boots");
		return item;
	}

	Item generateAmulet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Neck);

		Item item;
		item.icon = "amulet";
		item.slot = SlotEnum::Neck;
		item.goldCost = randomRange(40, 60);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.name = item.makeName("Amulet");
		return item;
	}
}

Item generateItem(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	// may or may not have slot

	switch (generate.slot)
	{
		case SlotEnum::None:
		{
			Generate g = generate;
			g.slot = []()
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
			}();
			return generateItem(g);
		}
		case SlotEnum::MainHand:
		{
			if (generate.magic > 0.5)
				return generateWand(generate);
			else
			{
				if (generate.ranged > 0.5)
					return generateBow(generate);
				else
					return generateSword(generate);
			}
		}
		case SlotEnum::OffHand:
		{
			if (generate.magic > 0.5)
				return generateWand(generate);
			else
				return generateShield(generate);
		}
		case SlotEnum::Head:
		{
			return generateHelmet(generate);
		}
		case SlotEnum::Body:
		{
			if (generate.magic > 0.5)
				return generateCape(generate);
			else
				return generateBodyArmor(generate);
		}
		case SlotEnum::Legs:
		{
			return generateBoots(generate);
		}
		case SlotEnum::Neck:
		{
			return generateAmulet(generate);
		}
		default:
			CAGE_THROW_CRITICAL(Exception, "invalid slot enum for generating item");
	}
}
