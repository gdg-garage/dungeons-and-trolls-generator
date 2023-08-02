#include "dnt.h"

namespace
{
	constexpr const char *Alone = "\"alone\""; // requires that the caster is alone (no other creature (player or monster) in visible range)
	constexpr const char *LineOfSight = "\"lineOfSight\""; // requires the target position is visible from the caster position
	constexpr const char *Moves = "\"moves\""; // moves the caster to the target position, or the target to the caster position
	constexpr const char *Knockback = "\"knockback\""; // moves the caster/target one tile away from the other
	constexpr const char *Stun = "\"stun\""; // prevents the caster/target from performing any actions for one tick, and grants immunity to stun for the following tick
	constexpr const char *GroundEffect = "\"groundEffect\""; // creates ground effect at caster/target position, which applies the effects of the skill
}

Skill generateSkill(const Generate &generate)
{
	CAGE_ASSERT(valid(generate.magic) && valid(generate.ranged) && valid(generate.support));

	Skill sk;

	if (randomChance() < 0.3)
		sk.target = SkillTargetEnum::Character;
	else if (randomChance() < 0.5)
		sk.target = SkillTargetEnum::Position;

	if (randomChance() < 0.6)
		sk.cost.stamina = randomRange(2, 5);
	if (randomChance() < 0.4)
		sk.cost.mana = randomRange(2, 5);

	if (randomChance() < 0.8)
		sk.range[AttributeEnum::Scalar] = randomRange(20, 100);
	if (randomChance() < 0.1)
		sk.range[AttributeEnum::Intelligence] = randomRange(20, 60);

	if (randomChance() < 0.4)
		sk.radius[AttributeEnum::Scalar] = randomRange(10, 30);
	if (randomChance() < 0.05)
		sk.radius[AttributeEnum::Willpower] = randomRange(10, 30);

	if (randomChance() < 0.4)
		sk.duration[AttributeEnum::Scalar] = randomRange(10, 30);
	if (randomChance() < 0.05)
		sk.duration[AttributeEnum::Constitution] = randomRange(10, 30);

	if (randomChance() < 0.5)
		sk.damageAmount[AttributeEnum::Scalar] = randomRange(40, 80);
	if (randomChance() < 0.5)
		sk.damageAmount[AttributeEnum::Strength] = randomRange(30, 70);

	if (randomChance() < 0.3)
		sk.damageType = DamageTypeEnum::Slash;
	else if (randomChance() < 0.5)
		sk.damageType = DamageTypeEnum::Fire;

	const auto &sae = []() -> SkillAttributesEffects
	{
		SkillAttributesEffects r;
		auto &k = r[AttributeEnum(randomRange(0u, (uint32)AttributeEnum::Scalar))];
		if (randomChance() < 0.4)
			k[AttributeEnum(randomRange(0, 5))] = randomRange(20, 50);
		if (randomChance() < 0.4)
			k[AttributeEnum(randomRange(0, 5))] = randomRange(20, 50);
		k[AttributeEnum::Scalar] = randomRange(20, 50);
		return r;
	};
	sk.casterAttributes = sae();
	sk.targetAttributes = sae();

	if (randomChance() < 0.8)
		sk.casterFlags.push_back(LineOfSight);
	if (randomChance() < 0.1)
		sk.casterFlags.push_back(Alone);
	if (randomChance() < 0.3)
		sk.casterFlags.push_back(Moves);

	if (randomChance() < 0.2)
		sk.targetFlags.push_back(Knockback);
	if (randomChance() < 0.1)
		sk.targetFlags.push_back(Stun);
	if (randomChance() < 0.2)
		sk.targetFlags.push_back(GroundEffect);
	if (randomChance() < 0.1)
		sk.targetFlags.push_back(Moves);

	if (randomChance() < 0.02)
	{
		std::string json;
		json += "{\n";
		json += "\"class\":\"summon\",\n";
		json += "\"data\":" + exportMonster(generateMonster(Generate(generate.level))) + "\n";
		json += "}";
		sk.targetFlags.push_back(std::move(json));
	}

	return sk;
}

namespace
{
	Item generateSword(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);

		Item item;
		item.slot = SlotEnum::MainHand;
		item.goldCost = randomRange(20, 30);

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(5, 10);
			sk.damageAmount[AttributeEnum::Strength] = sk.addPower(2, AffixEnum::Prefix, "Strong") + 0.5;
			sk.damageAmount[AttributeEnum::Scalar] = interpolate(1u, generate.ll(), sk.addPower(1, AffixEnum::Prefix, "Powerful"));
			sk.damageType = DamageTypeEnum::Slash;
			sk.range[AttributeEnum::Scalar] = 1;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Attack";
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
		item.slot = SlotEnum::MainHand;
		item.goldCost = randomRange(25, 35);

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(8, 13);
			sk.damageAmount[AttributeEnum::Dexterity] = sk.addPower(2, AffixEnum::Prefix, "Swift") + 0.5;
			sk.damageAmount[AttributeEnum::Scalar] = interpolate(1u, generate.ll(), sk.addPower(1, AffixEnum::Prefix, "Powerful"));
			sk.damageType = DamageTypeEnum::Piercing;
			sk.range[AttributeEnum::Strength] = sk.addPower(1, AffixEnum::Prefix, "Long") * 0.2;
			sk.range[AttributeEnum::Scalar] = 3;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.name = "Attack";
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
		item.slot = SlotEnum::OffHand;
		item.goldCost = randomRange(10, 15);

		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Impenetrable"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.cost.stamina = randomRange(1, 6);
			sk.range[AttributeEnum::Scalar] = 1;
			sk.target = SkillTargetEnum::Character;
			sk.casterFlags.push_back(LineOfSight);
			sk.targetFlags.push_back(Knockback);
			sk.name = "Push";
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
		item.slot = SlotEnum::Head;
		item.goldCost = randomRange(15, 25);

		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.7)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		item.name = item.makeName("Helmet");
		return item;
	}

	Item generateBodyArmor(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);

		Item item;
		item.slot = SlotEnum::Body;
		item.goldCost = randomRange(30, 50);

		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.ll() * 2, item.addPower(2, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.8)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.ll() * 2, item.addPower(2, AffixEnum::Prefix, "Impenetrable"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.3)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.target = SkillTargetEnum::None;
			sk.casterFlags.push_back(Alone);
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = interpolate(0.5, 1.0, sk.addPower(1));
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = interpolate(1, 10, sk.addPower(0.5));
			sk.name = "Rest";
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
		item.slot = SlotEnum::Body;
		item.goldCost = randomRange(25, 55);

		if (randomChance() < 0.4)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.ll() * 2, item.addPower(2, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.4)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.ll() * 2, item.addPower(2, AffixEnum::Prefix, "Impenetrable"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::FireResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Watering"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::PoisonResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Antidote"));
		if (randomChance() < 0.6)
			item.attributes[AttributeEnum::ElectricResist] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Suffix, "of Lightning rod"));

		{
			Skill sk;
			sk.goldCost = randomRange(3, 5);
			sk.target = SkillTargetEnum::None;
			sk.casterFlags.push_back(Alone);
			sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Willpower] = interpolate(0.5, 1.0, sk.addPower(1));
			sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Scalar] = interpolate(1, 10, sk.addPower(0.5));
			sk.name = "Meditate";
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
		item.slot = SlotEnum::Legs;
		item.goldCost = randomRange(20, 30);

		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::SlashArmor] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Plated"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::PiercingArmor] = interpolate(1u, generate.ll(), item.addPower(1, AffixEnum::Prefix, "Impenetrable"));

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
	CAGE_ASSERT(valid(generate.magic) && valid(generate.ranged) && valid(generate.support));

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

Monster generateMonster(const Generate &generate)
{
	CAGE_ASSERT(valid(generate.magic) && valid(generate.ranged) && valid(generate.support));
	CAGE_ASSERT(generate.slot == SlotEnum::None);

	Monster mr;

	const uint32 level = generate.ll();

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
		mr.onDeath.push_back(generateSkill(Generate(SlotEnum::MainHand, generate.level, generate.difficultyOffset)));

	mr.score = numeric_cast<uint32>(mr.goldCost);

	return mr;
}

Monster generateChest(const Generate &generate)
{
	CAGE_ASSERT(valid(generate.magic) && valid(generate.ranged) && valid(generate.support));
	CAGE_ASSERT(generate.slot == SlotEnum::None);

	Monster mr;

	mr.name = "Chest";
	mr.attributes[AttributeEnum::Life] = 1;

	const uint32 cnt = randomRange(3u, 6u);
	for (uint32 i = 0; i < cnt; i++)
		mr.onDeath.push_back(generateItem(Generate(generate.level, generate.difficultyOffset)));

	return mr;
}
