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

Skill generateMagicSpell(uint32 level, SlotEnum slot)
{
	CAGE_ASSERT(level >= 1);

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
		json += "\"data\":" + exportMonster(generateSummonedMinion(level)) + "\n";
		json += "}";
		sk.targetFlags.push_back(std::move(json));
	}

	return sk;
}

namespace
{
	void generateRandomItemAttributes(uint32 level, Item &item)
	{
		if (randomChance() < 0.1)
			item.requirements[AttributeEnum::Strength] = interpolate(level, 1u, item.addPower(1));
		if (randomChance() < 0.1)
			item.requirements[AttributeEnum::Intelligence] = interpolate(level, 1u, item.addPower(1));
		if (randomChance() < 0.1)
			item.requirements[AttributeEnum::Dexterity] = interpolate(level, 1u, item.addPower(1));
		if (randomChance() < 0.1)
			item.requirements[AttributeEnum::Willpower] = interpolate(level, 1u, item.addPower(1));
		if (randomChance() < 0.1)
			item.requirements[AttributeEnum::Constitution] = interpolate(level, 1u, item.addPower(1));

		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::Strength] = interpolate(1u, level, item.addPower(1, AffixEnum::Suffix, "of Strength"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::Intelligence] = interpolate(1u, level, item.addPower(1, AffixEnum::Suffix, "of Intelligence"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::Dexterity] = interpolate(1u, level, item.addPower(1, AffixEnum::Suffix, "of Dexterity"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::Willpower] = interpolate(1u, level, item.addPower(1, AffixEnum::Suffix, "of Willpower"));
		if (randomChance() < 0.5)
			item.attributes[AttributeEnum::Constitution] = interpolate(1u, level, item.addPower(1, AffixEnum::Suffix, "of Constitution"));

		if (randomChance() < 0.75)
		{
			Skill s = generateMagicSpell(level, item.slot);
			item.addPower(s, 2);
			item.skills.push_back(std::move(s));
		}
		if (randomChance() < 0.25)
		{
			Skill s = generateMagicSpell(level, item.slot);
			item.addPower(s, 2);
			item.addAffix(randomChance(), AffixEnum::Prefix, "Skilled");
			item.skills.push_back(std::move(s));
		}
	}

	Item generateSword(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::MainHand;
		item.goldCost = randomRange(20, 30);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Sword");
		return item;
	}

	Item generateShield(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::OffHand;
		item.goldCost = randomRange(20, 30);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Shield");
		return item;
	}

	Item generateHelmet(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::Head;
		item.goldCost = randomRange(15, 25);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Helmet");
		return item;
	}

	Item generateBodyArmor(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::Body;
		item.goldCost = randomRange(30, 50);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Body Armor");
		return item;
	}

	Item generateShoes(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::Legs;
		item.goldCost = randomRange(20, 30);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Shoes");
		return item;
	}

	Item generateAmulet(uint32 level)
	{
		Item item;
		item.slot = SlotEnum::Neck;
		item.goldCost = randomRange(40, 60);
		generateRandomItemAttributes(level, item);
		item.name = item.makeName("Amulet");
		return item;
	}
}

Item generateEquippedItem(uint32 level, SlotEnum slot)
{
	CAGE_ASSERT(level >= 1);
	switch (slot)
	{
		case SlotEnum::MainHand:
			return generateSword(level);
		case SlotEnum::OffHand:
			return generateShield(level);
		case SlotEnum::Head:
			return generateHelmet(level);
		case SlotEnum::Body:
			return generateBodyArmor(level);
		case SlotEnum::Legs:
			return generateShoes(level);
		case SlotEnum::Neck:
			return generateAmulet(level);
		default:
			CAGE_THROW_CRITICAL(Exception, "invalid slot enum for generating item");
	}
}

Item generateDroppedItem(uint32 level)
{
	const SlotEnum slot = []()
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
	return generateEquippedItem(level, slot);
}

Monster generateMonster(uint32 level, sint32 difficultyOffset)
{
	CAGE_ASSERT(level >= 1);

	Monster mr;

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

	mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::MainHand));
	if (randomChance() < 0.3)
		mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::OffHand));
	if (randomChance() < 0.3)
		mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::Head));
	if (randomChance() < 0.6)
		mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::Body));
	if (randomChance() < 0.3)
		mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::Legs));
	if (randomChance() < 0.1)
		mr.equippedItems.push_back(generateEquippedItem(level, SlotEnum::Neck));

	if (randomChance() < 0.5)
		mr.onDeath.push_back(std::make_unique<Item>(generateDroppedItem(level)));
	if (randomChance() < 0.1)
		mr.onDeath.push_back(std::make_unique<Monster>(generateMonster(level, 0)));
	if (randomChance() < 0.1)
		mr.onDeath.push_back(std::make_unique<Skill>(generateMagicSpell(level, SlotEnum::MainHand)));

	return mr;
}

Monster generateSummonedMinion(uint32 level)
{
	// todo
	return generateMonster(level, 0);
}

Monster generateChest(uint32 level)
{
	Monster mr;

	mr.name = "Chest";
	mr.attributes[AttributeEnum::Life] = 1;

	const uint32 cnt = randomRange(3u, level / 3 + 4);
	for (uint32 i = 0; i < cnt; i++)
		mr.onDeath.push_back(std::make_unique<Item>(generateDroppedItem(level)));

	return mr;
}
