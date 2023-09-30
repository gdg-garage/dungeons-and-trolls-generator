#include "dnt.h"

Skill skillGeneric(const Generate &generate);

Item::Item(const Generate &generate) : Thing(generate), slot(generate.slot) {}

namespace
{
	void addNothing(Item &item) {}

	template<AttributeEnum Attr>
	void addBoost(Item &item)
	{
		static constexpr const char *names[] = {
			"Strengthening",
			"Dexterous",
			"Genius",
			"Strong-willed",
			"Constitutional",
			"Slashproof",
			"Impervious",
			"Fireproof",
			"Antidote",
			"Grounded",
			"Vitalizing",
			"Arcane",
			"Vigorous",
		};
		static_assert(sizeof(names) / sizeof(names[0]) == (uint32)AttributeEnum::Constant);

		const Real a = item.generate.power * 0.1 + 5;
		const Real r = a * interpolate(0.8, 1.2, item.addPower(1, names[(uint32)Attr]));
		item.attributes[Attr] += numeric_cast<sint32>(r);
		item.addPower(1); // make attributes more expensive
	}

	void makeBoost(Item &item)
	{
		Candidates<void (*)(Item &)> candidates(item.generate);
		candidates.slotMismatchPenalty = 0.5;
		candidates.randomness = 1.5;

		candidates.add(0, 0, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(0, 1, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, H, 1, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::SlashResist>);

		candidates.add(0, 0, 0, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(0, 1, 0, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, H, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Willpower>);
		candidates.add(0, H, 1, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::PierceResist>);

		candidates.add(0, H, H, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::Constitution>);
		candidates.add(1, H, 0, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, 0, 1, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::SlashResist>);
		candidates.add(0, 1, 1, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::PierceResist>);
		candidates.add(1, H, 1, H, SlotEnum::Body, { LevelFire }, addBoost<AttributeEnum::FireResist>);

		candidates.add(0, H, 0, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, 0, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, 1, 1, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::PierceResist>);
		candidates.add(1, H, 1, H, SlotEnum::Head, { LevelPoison }, addBoost<AttributeEnum::PoisonResist>);

		candidates.add(0, H, 0, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(1, H, H, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Willpower>);
		candidates.add(0, H, H, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Constitution>);
		candidates.add(0, 0, 1, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::SlashResist>);
		candidates.add(1, H, 1, H, SlotEnum::Legs, { LevelElectric }, addBoost<AttributeEnum::ElectricResist>);

		candidates.add(1, H, 0, H, SlotEnum::Neck, { Nothing }, addBoost<AttributeEnum::Willpower>);
		candidates.add(0, H, 0, H, SlotEnum::Neck, { Nothing }, addBoost<AttributeEnum::Constitution>);
		candidates.add(1, H, 1, H, SlotEnum::Neck, { LevelFire }, addBoost<AttributeEnum::FireResist>);
		candidates.add(1, H, 1, H, SlotEnum::Neck, { LevelPoison }, addBoost<AttributeEnum::PoisonResist>);
		candidates.add(1, H, 1, H, SlotEnum::Neck, { LevelElectric }, addBoost<AttributeEnum::ElectricResist>);

		candidates.fallback(addNothing);
		candidates.pick()(item);
	}

	template<AttributeEnum Attr>
	void addRequirement(Item &item)
	{
		const Real a = item.generate.power * 0.3 + 15;
		const Real r = a * interpolate(0.8, 1.2, item.subtractPower(0.6, "Of Need", AffixEnum::Suffix));
		item.requirements[Attr] += numeric_cast<sint32>(r);
	}

	void makeRequirement(Item &item)
	{
		Candidates<void (*)(Item &)> candidates(item.generate);
		candidates.slotMismatchPenalty = 0.5;
		candidates.randomness = 1.5;

		candidates.add(0, 0, H, H, SlotEnum::MainHand, { Nothing }, addRequirement<AttributeEnum::Strength>);
		candidates.add(0, 1, H, H, SlotEnum::MainHand, { Nothing }, addRequirement<AttributeEnum::Dexterity>);
		candidates.add(1, H, H, H, SlotEnum::MainHand, { Nothing }, addRequirement<AttributeEnum::Intelligence>);

		candidates.add(0, 0, H, H, SlotEnum::OffHand, { Nothing }, addRequirement<AttributeEnum::Strength>);
		candidates.add(0, 1, H, H, SlotEnum::OffHand, { Nothing }, addRequirement<AttributeEnum::Dexterity>);
		candidates.add(1, H, H, H, SlotEnum::OffHand, { Nothing }, addRequirement<AttributeEnum::Intelligence>);

		candidates.add(0, H, H, H, SlotEnum::Body, { Nothing }, addRequirement<AttributeEnum::Strength>);
		candidates.add(0, 1, H, H, SlotEnum::Body, { Nothing }, addRequirement<AttributeEnum::Dexterity>);
		candidates.add(0, 0, H, H, SlotEnum::Body, { Nothing }, addRequirement<AttributeEnum::Constitution>);
		candidates.add(1, H, H, H, SlotEnum::Body, { Nothing }, addRequirement<AttributeEnum::Willpower>);

		candidates.add(0, H, H, H, SlotEnum::Head, { Nothing }, addRequirement<AttributeEnum::Dexterity>);
		candidates.add(1, H, H, H, SlotEnum::Head, { Nothing }, addRequirement<AttributeEnum::Willpower>);

		candidates.add(0, H, H, H, SlotEnum::Legs, { Nothing }, addRequirement<AttributeEnum::Strength>);
		candidates.add(1, H, H, H, SlotEnum::Legs, { Nothing }, addRequirement<AttributeEnum::Willpower>);
		candidates.add(0, H, H, H, SlotEnum::Legs, { Nothing }, addRequirement<AttributeEnum::Constitution>);

		candidates.add(1, H, H, H, SlotEnum::Neck, { Nothing }, addRequirement<AttributeEnum::Intelligence>);
		candidates.add(0, H, H, H, SlotEnum::Neck, { Nothing }, addRequirement<AttributeEnum::Constitution>);

		candidates.fallback(addNothing);
		candidates.pick()(item);
	}

	Item generateBasicItem(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot != SlotEnum::None);
		Item item(generate);

		makeBoost(item);
		if (randomChance() < 0.6)
		{
			item.addAffix(0.7, "Boosting");
			makeBoost(item);
		}
		if (randomChance() < 0.2)
		{
			item.addAffix(0.85, "Rare");
			makeBoost(item);
		}

		if (generate.level > LevelRequirements)
		{
			makeRequirement(item);
			if (randomChance() < 0.3)
				makeRequirement(item);
		}

		if (randomChance() < 0.15)
		{
			item.addAffix(0.7, "Skilled");
			Skill sk = skillGeneric(generate);
			item.addOther(sk, 0.5);
			item.skills.push_back(std::move(sk));
		}

		return item;
	}

	void finalizeBasicItem(Item &item)
	{
		// ethereal
		if (item.generate.level > LevelEthereal && !item.attributes.empty() && randomChance() < 0.05)
		{
			Generate g = item.generate;
			g.magic = 1;
			g.ranged = 0;
			g.defensive = 0;
			g.support = 0;
			Skill sk(g);
			const Real mult = interpolate(1.2, 1.5, sk.addPower(1, "Celestial"));
			for (const auto &it : item.attributes)
				sk.caster.attributes[it.first][AttributeEnum::Constant] = it.second * mult;
			sk.flags.passive = true;
			sk.updateName("Ether");
			item.addOther(sk, 0.5);
			item.addAffix(0.8, "Ethereal");
			item.skills.push_back(std::move(sk));
			item.attributes.clear();
		}
	}
}

// section skills
namespace
{}

Skill skillSwordAttack(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 1;
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Strong"));
	sk.damageType = DamageTypeEnum::Slash;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
	sk.updateName("Attack");
	return sk;
}

Skill skillPikeAttack(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 2;
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Stout")) * 0.5;
	sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Piercing")) * 0.5;
	sk.damageType = DamageTypeEnum::Pierce;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 13);
	sk.updateName("Attack");
	return sk;
}

Skill skillBowAttack(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Accurate")) * 0.1;
	sk.range[AttributeEnum::Constant] = 4;
	sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Piercing")) * 0.5;
	sk.damageType = DamageTypeEnum::Pierce;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
	sk.updateName("Attack");
	return sk;
}

Skill skillRest(const Generate &generate, Real factor)
{
	Skill sk(generate);
	sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Refreshing")) * factor;
	sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constant] = interpolate(5.0, 15.0, sk.addPower(1, "Energizing")) * factor;
	sk.flags.requiresOutOfCombat = true;
	sk.updateName("Rest");
	return sk;
}

// section items
namespace
{}

Item itemSword(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillSwordAttack(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Sword");
	item.icon = "sword";
	return item;
}

Item itemPike(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillPikeAttack(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Pike");
	item.icon = "pike";
	return item;
}

Item itemScythe(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk(generate);
		sk.radius[AttributeEnum::Constant] = interpolate(1.5, 3.0, sk.addPower(1, "Wide"));
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Strong")) * 0.3;
		sk.damageType = DamageTypeEnum::Slash;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
		sk.updateName("Attack");
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Scythe");
	item.icon = "scythe";
	return item;
}

Item itemBow(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillBowAttack(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Bow");
	item.icon = "bow";
	return item;
}

Item itemStaff(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Staff");
	item.icon = "staff";
	return item;
}

Item itemDagger(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Constant] = 1;
		sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Surprise")) * 0.5;
		sk.damageType = DamageTypeEnum::Slash;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 7);
		sk.updateName("Attack");
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Dagger");
	item.icon = "dagger";
	return item;
}

Item itemShield(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	makeBoost(item);

	finalizeBasicItem(item);
	item.updateName("Shield");
	item.icon = "shield";
	return item;
}

Item itemCrystalBall(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	addBoost<AttributeEnum::Mana>(item);

	finalizeBasicItem(item);
	item.updateName("CrystalBall");
	item.icon = "crystalBall";
	return item;
}

Item itemWand(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::MainHand || generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Wand");
	item.icon = "wand";
	return item;
}

Item itemScroll(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Scroll");
	item.icon = "scroll";
	return item;
}

Item itemTalisman(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Talisman");
	item.icon = "talisman";
	return item;
}

Item itemLeatherArmor(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Body);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	{
		Skill sk = skillRest(generate, 1);
		item.addOther(sk, 0.7);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Leather Armor");
	item.icon = "leatherArmor";
	return item;
}

Item itemRingMail(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Body);
	Item item = generateBasicItem(generate);

	makeBoost(item);

	{
		Skill sk = skillRest(generate, 0.7);
		item.addOther(sk, 0.7);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Ring Mail");
	item.icon = "ringMail";
	return item;
}

Item itemPlatedMail(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Body);
	Item item = generateBasicItem(generate);

	makeBoost(item);
	makeBoost(item);

	{
		Skill sk = skillRest(generate, 0.4);
		item.addOther(sk, 0.7);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Plated Mail");
	item.icon = "platedMail";
	return item;
}

Item itemCape(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Body);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	{
		Skill sk(generate);
		sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Energizing")) * 0.8;
		sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.7, "Refreshing")) * 0.2;
		sk.flags.requiresOutOfCombat = true;
		sk.updateName("Meditation");
		item.addOther(sk, 0.7);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Cape");
	item.icon = "cape";
	return item;
}

Item itemHelmet(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Head);
	Item item = generateBasicItem(generate);

	makeBoost(item);

	finalizeBasicItem(item);
	item.updateName("Helmet");
	item.icon = "helmet";
	return item;
}

Item itemCirclet(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Head);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	item.updateName("Circlet");
	item.icon = "circlet";
	return item;
}

Item itemProtectiveTattoos(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Body);
	Item item(generate); // no generateBasicItem -> no stats

	if (generate.level > LevelFire)
		item.attributes[AttributeEnum::FireResist] = interpolate(8.5, 12.5, item.addPower(1, "Watering"));
	if (generate.level > LevelPoison)
		item.attributes[AttributeEnum::PoisonResist] = interpolate(8.5, 12.5, item.addPower(1, "Antidote"));
	if (generate.level > LevelElectric)
		item.attributes[AttributeEnum::ElectricResist] = interpolate(8.5, 12.5, item.addPower(1, "Isolating"));

	item.updateName("Tattoos");
	item.icon = "tattoos";
	return item;
}

Item itemRestoringTattoos(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Head);
	Item item(generate); // no generateBasicItem -> no stats

	{
		Skill sk(generate);
		if (randomChance() < 0.5)
			sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Refreshing")) * 0.2;
		else
			sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Energizing")) * 0.2;
		sk.flags.passive = true;
		sk.updateName("Glow");
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	item.updateName("Tattoos");
	item.icon = "tattoos";
	return item;
}

Item itemBoots(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Legs);
	Item item = generateBasicItem(generate);

	makeBoost(item);

	if (randomChance() < 0.7)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Enduring")) * 0.05;
		sk.range[AttributeEnum::Constant] = 2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 5);
		sk.caster.flags.movement = true;
		sk.updateName("Walk");
		item.addOther(sk, 0.6);
		item.skills.push_back(std::move(sk));
	}
	else
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Enduring")) * 0.03;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Fast")) * 0.03;
		sk.range[AttributeEnum::Constant] = 3;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
		sk.caster.flags.movement = true;
		sk.updateName("Run");
		item.addOther(sk, 0.8);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	switch (randomRange(0, 2))
	{
		case 0:
			item.updateName("Boots");
			item.icon = "boots";
			break;
		case 1:
			item.updateName("Shoes");
			item.icon = "shoes";
			break;
		default:
			CAGE_THROW_CRITICAL(Exception, "random out of range");
	}
	return item;
}

Item itemAmulet(const Generate &generate)
{
	CAGE_ASSERT(generate.slot == SlotEnum::Neck);
	Item item = generateBasicItem(generate);

	{
		Skill sk = skillGeneric(generate);
		item.addOther(sk, 1);
		item.skills.push_back(std::move(sk));
	}

	finalizeBasicItem(item);
	switch (randomRange(0, 3))
	{
		case 0:
			item.updateName("Amulet");
			item.icon = "amulet";
			break;
		case 1:
			item.updateName("Pendant");
			item.icon = "pendant";
			break;
		case 2:
			item.updateName("Necklace");
			item.icon = "necklace";
			break;
		default:
			CAGE_THROW_CRITICAL(Exception, "random out of range");
	}
	return item;
}

Item itemTrinket(const Generate &generate)
{
	CAGE_ASSERT(generate.slot != SlotEnum::None);
	Item item = generateBasicItem(generate);

	makeBoost(item);

	finalizeBasicItem(item);
	item.updateName("Trinket");
	item.icon = "trinket";
	return item;
}

// section other
namespace
{}

Item itemGeneric(const Generate &generate)
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
		return itemGeneric(g);
	}

	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot != SlotEnum::None);

	Candidates<Item (*)(const Generate &generate)> candidates(generate);
	candidates.slotMismatchPenalty = 10;

	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelSlash }, itemSword);
	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelPierce }, itemPike);
	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelSlash, LevelAoe }, itemScythe);
	candidates.add(0, 1, 0, 0, SlotEnum::MainHand, { LevelPierce }, itemBow);
	candidates.add(H, 0, 1, 0, SlotEnum::MainHand, { LevelMagic }, itemStaff);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { Nothing }, itemWand);
	candidates.add(0, 0, 0, 0, SlotEnum::OffHand, { Nothing }, itemDagger);
	candidates.add(0, H, 1, 0, SlotEnum::OffHand, { Nothing }, itemShield);
	candidates.add(1, H, H, 1, SlotEnum::OffHand, { Nothing }, itemCrystalBall);
	candidates.add(1, 1, 0, H, SlotEnum::OffHand, { Nothing }, itemWand);
	candidates.add(1, 0, H, 0, SlotEnum::OffHand, { Nothing }, itemScroll);
	candidates.add(1, H, 1, 1, SlotEnum::OffHand, { Nothing }, itemTalisman);
	candidates.add(0, H, 0, 0, SlotEnum::Body, { Nothing }, itemLeatherArmor);
	candidates.add(0, H, H, 0, SlotEnum::Body, { Nothing }, itemRingMail);
	candidates.add(0, H, 1, 0, SlotEnum::Body, { Nothing }, itemPlatedMail);
	candidates.add(1, H, 1, 0, SlotEnum::Body, { Nothing }, itemCape);
	candidates.add(1, H, 1, 0, SlotEnum::Body, { Nothing }, itemProtectiveTattoos);
	candidates.add(0, H, 1, 0, SlotEnum::Head, { Nothing }, itemHelmet);
	candidates.add(1, H, 0, H, SlotEnum::Head, { Nothing }, itemCirclet);
	candidates.add(1, H, 0, 1, SlotEnum::Head, { Nothing }, itemRestoringTattoos);
	candidates.add(H, H, 0, 0, SlotEnum::Legs, { Nothing }, itemBoots);
	candidates.add(0, H, 0, H, SlotEnum::Neck, { Nothing }, itemAmulet);

	candidates.fallback(itemTrinket);
	return candidates.pick()(generate);
}

Item itemShop(uint32 maxLevel)
{
	Generate gen;
	switch (randomRange(0u, 4u))
	{
		case 0:
			gen = Generate(randomRange(1u, maxLevel), 0); // default
			break;
		case 1:
			gen = Generate(randomRange(max(maxLevel * 3 / 4, 1u), maxLevel), 0); // stronger than default
			break;
		case 2:
			gen = Generate(maxLevel, -(sint32)randomRange(0u, maxLevel / 4)); // any features, but possibly slightly weak
			break;
		case 3:
			gen = Generate(maxLevel, -(sint32)randomRange(0u, maxLevel)); // any features, but possibly very weak
			break;
	}
	CAGE_ASSERT(gen.level > 0);
	CAGE_ASSERT(gen.power > 0);
	bool unidentified = false;
	Real costMult = 1;
	const Real uniChance = clamp((Real(maxLevel) - 50) * 0.01, 0, 0.5);
	if (gen.power > 10 && randomChance() < uniChance)
	{
		unidentified = true;
		costMult = pow(randomChance() + 0.9, 5);
		Real powMult = pow(randomChance() + 0.4, 2);
		gen.power = numeric_cast<sint32>(gen.power * powMult);
		CAGE_ASSERT(gen.power > 0);
	}
	Item item = itemGeneric(gen);
	item.unidentified = unidentified;
	item.buyPrice = numeric_cast<uint32>(item.goldCost * costMult);
	return item;
}

Item itemPrimitive(SlotEnum slot)
{
	Item item = Item(Generate(1, 0, slot));

	item.addAffix(randomChance() * 10 + 10, "Primitive");
	item.addAffix(randomChance() * 10 + 10, "Pathetic");
	item.addAffix(randomChance() * 10 + 10, "Beginner");
	item.addAffix(randomChance() * 10 + 10, "Useless");
	item.addAffix(randomChance() * 10 + 10, "Worthless");

	switch (slot)
	{
		case SlotEnum::Body:
		{
			item.attributes[randomChance() < 0.5 ? AttributeEnum::Strength : AttributeEnum::Constitution] = randomRange(3, 6);
			{
				Skill sk(item.generate);
				sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constant] = randomRange(0.8, 1.2) * 5;
				sk.flags.requiresOutOfCombat = true;
				sk.updateName("Rest");
				item.skills.push_back(std::move(sk));
			}
			item.updateName("Tunic");
			item.icon = "tunic";
			break;
		}
		case SlotEnum::Head:
		{
			item.attributes[randomChance() < 0.5 ? AttributeEnum::Willpower : AttributeEnum::Intelligence] = randomRange(2, 5);
			item.updateName("Cap");
			item.icon = "cap";
			break;
		}
		case SlotEnum::MainHand:
		{
			item.attributes[randomChance() < 0.5 ? AttributeEnum::Strength : AttributeEnum::Dexterity] = randomRange(3, 6);
			{
				Skill sk(item.generate);
				sk.targetType = SkillTargetEnum::Character;
				sk.range[AttributeEnum::Constant] = 1;
				sk.damageAmount[AttributeEnum::Constant] = randomRange(0.8, 1.2) * 2;
				sk.damageType = DamageTypeEnum::Pierce;
				sk.cost[AttributeEnum::Stamina] = randomRange(0.8, 1.2) * 4;
				sk.updateName("Poke");
				item.skills.push_back(std::move(sk));
			}
			item.updateName("Stick");
			item.icon = "stick";
			break;
		}
		case SlotEnum::Legs:
		{
			item.attributes[randomChance() < 0.5 ? AttributeEnum::Dexterity : AttributeEnum::Constitution] = randomRange(2, 5);
			item.updateName("Slippers");
			item.icon = "slippers";
			break;
		}
		default:
			CAGE_THROW_ERROR(Exception, "invalid slot for primitive item");
	}

	//item.buyPrice = numeric_cast<uint32>(item.goldCost);
	return item;
}

Item itemSprayCan()
{
	Item item(Generate(1, 0, SlotEnum::MainHand));
	item.updateName("Spray Can");
	item.icon = "sprayCan";

	for (char c = 'A'; c <= 'Z'; c++)
	{
		Skill sk(item.generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Constant] = 1;
		sk.duration[AttributeEnum::Constant] = 90;
		sk.target.summons.push_back(Decoration{ "sprayPaint", std::string(1, c) });
		sk.updateName("Spray");
		item.skills.push_back(std::move(sk));
	}

	return item;
}
