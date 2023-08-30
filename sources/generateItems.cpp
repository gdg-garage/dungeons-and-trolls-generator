#include "dnt.h"

Item::Item(const Generate &generate) : Thing(generate){};

namespace
{
	void addNothing(Item &item) {}

	template<AttributeEnum Attr>
	void addBoost(Item &item)
	{
		const Real a = item.generate.power / 20 + 1;
		const Real b = item.generate.power / 10 + 3;
		static constexpr const char *names[] = {
			"Strengthening",
			"Dexterous",
			"Intelligent",
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
		static_assert(sizeof(names) / sizeof(names[0]) == (uint32)AttributeEnum::Scalar);
		Real r = interpolate(a, b, item.addPower(0.8, AffixEnum::Prefix, names[(uint32)Attr]));
		if constexpr (Attr >= AttributeEnum::SlashArmor)
			r *= 1.5;
		item.attributes[Attr] += numeric_cast<sint32>(r + 0.5);
	}

	void makeBoost(Item &item)
	{
		Candidates<void (*)(Item &)> candidates(item.generate);

		candidates.add(0, 0, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(0, 1, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, 0, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, H, 1, H, SlotEnum::MainHand, { Nothing }, addBoost<AttributeEnum::SlashArmor>);

		candidates.add(0, 0, 0, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(0, 1, 0, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, H, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::Willpower>);
		candidates.add(0, H, 1, H, SlotEnum::OffHand, { Nothing }, addBoost<AttributeEnum::PiercingArmor>);

		candidates.add(0, H, H, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::Constitution>);
		candidates.add(1, H, 0, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, 0, 1, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::SlashArmor>);
		candidates.add(0, 1, 1, H, SlotEnum::Body, { Nothing }, addBoost<AttributeEnum::PiercingArmor>);
		candidates.add(1, H, 1, H, SlotEnum::Body, { LevelFire }, addBoost<AttributeEnum::FireResist>);

		candidates.add(0, H, 0, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::Dexterity>);
		candidates.add(1, H, 0, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::Intelligence>);
		candidates.add(0, 1, 1, H, SlotEnum::Head, { Nothing }, addBoost<AttributeEnum::PiercingArmor>);
		candidates.add(1, H, 1, H, SlotEnum::Head, { LevelPoison }, addBoost<AttributeEnum::PoisonResist>);

		candidates.add(0, H, 0, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Strength>);
		candidates.add(1, H, H, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Willpower>);
		candidates.add(0, H, H, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::Constitution>);
		candidates.add(0, 0, 1, H, SlotEnum::Legs, { Nothing }, addBoost<AttributeEnum::SlashArmor>);
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
		const Real a = item.generate.power / 6 + 1;
		const Real b = item.generate.power / 3 + 3;
		const Real r = interpolate(a, b, item.subtractPower(0.6, AffixEnum::Suffix, "Of Need"));
		item.requirements[Attr] += numeric_cast<sint32>(r + 0.5);
	}

	void makeRequirement(Item &item)
	{
		Candidates<void (*)(Item &)> candidates(item.generate);

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
		item.slot = generate.slot;

		makeBoost(item);
		if (item.addPower(0.5, AffixEnum::Prefix, "Rare") > 0.7)
			makeBoost(item);

		if (generate.level > 22)
		{
			makeRequirement(item);
			if ((randomChance() < 0.3))
				makeRequirement(item);
		}

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
			sk.targetType = SkillTargetEnum::Character;
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

	Item generatePike(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::MainHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.targetType = SkillTargetEnum::Character;
			sk.range[AttributeEnum::Scalar] = 2;
			sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Stout") * 0.5;
			sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 1, "Piercing") * 0.5;
			sk.damageType = DamageTypeEnum::Piercing;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 13);
			sk.updateName("Attack");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Pike");
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
			sk.targetType = SkillTargetEnum::Character;
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

	Item generateDagger(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk(generate);
			sk.targetType = SkillTargetEnum::Character;
			sk.range[AttributeEnum::Scalar] = 1;
			sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Strong") * 0.5;
			sk.damageType = DamageTypeEnum::Slash;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 7);
			sk.updateName("Attack");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Dagger");
		return item;
	}

	Item generateShield(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		makeBoost(item);

		item.updateName("Shield");
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

		item.updateName("Wand");
		return item;
	}

	Item generateScroll(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Scroll");
		return item;
	}

	Item generateTalisman(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::OffHand);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Talisman");
		return item;
	}

	Item generateBodyArmor(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);
		Item item = generateBasicItem(generate);

		makeBoost(item);
		makeBoost(item);

		{
			Skill sk(generate);
			sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 1, "Refreshing");
			sk.caster.flags.push_back(SkillAlone);
			sk.updateName("Rest");
			item.addPower(sk, 0.7);
			item.skills.push_back(std::move(sk));
		}

		item.updateName(generate.defensive < 0.5 ? "Leather Mail" : "Plated Mail");
		return item;
	}

	Item generateCape(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Energizing") * 0.8;
			sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 0.7, "Refreshing") * 0.2;
			sk.caster.flags.push_back(SkillAlone);
			sk.updateName("Meditation");
			item.addPower(sk, 0.7);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Cape");
		return item;
	}

	Item generateHelmet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head);
		Item item = generateBasicItem(generate);

		makeBoost(item);

		item.updateName("Helmet");
		return item;
	}

	Item generateCirclet(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head);
		Item item = generateBasicItem(generate);

		{
			Skill sk = generateSkill(generate);
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Circlet");
		return item;
	}

	Item generateProtectiveTattoos(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Body);
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

	Item generateRestoringTattoos(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Head);
		Item item(generate); // no generateBasicItem -> no stats
		item.slot = generate.slot;

		{
			Skill sk(generate);
			if (randomChance() < 0.5)
				sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = interpolate(1.0, 6.0, sk.addPower(1, AffixEnum::Prefix, "Refreshing"));
			else
				sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Scalar] = interpolate(1.0, 6.0, sk.addPower(1, AffixEnum::Prefix, "Energizing"));
			sk.caster.flags.push_back(SkillPassive);
			sk.updateName("Glow");
			item.addPower(sk, 1);
			item.skills.push_back(std::move(sk));
		}

		item.updateName("Tattoos");
		return item;
	}

	Item generateBoots(const Generate &generate)
	{
		CAGE_ASSERT(generate.slot == SlotEnum::Legs);
		Item item = generateBasicItem(generate);

		makeBoost(item);

		if (randomChance() < 0.7)
		{
			Skill sk(generate);
			sk.targetType = SkillTargetEnum::Position;
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
			sk.targetType = SkillTargetEnum::Position;
			sk.range[AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 1, "Enduring") * 0.075;
			sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Fast") * 0.075;
			sk.range[AttributeEnum::Scalar] = 2;
			sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
			sk.updateName("Run");
			item.addPower(sk, 0.8);
			item.skills.push_back(std::move(sk));
		}

		item.updateName(
			[]()
			{
				switch (randomRange(0, 2))
				{
					case 0:
						return "Boots";
					case 1:
						return "Shoes";
					default:
						CAGE_THROW_CRITICAL(Exception, "random out of range");
				}
			}());
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

		item.updateName(
			[]()
			{
				switch (randomRange(0, 3))
				{
					case 0:
						return "Amulet";
					case 1:
						return "Pendant";
					case 2:
						return "Necklace";
					default:
						CAGE_THROW_CRITICAL(Exception, "random out of range");
				}
			}());
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

	Candidates<Item (*)(const Generate &generate)> candidates(generate);
	candidates.slotMismatchPenalty = 10;

	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelSlash }, generateSword);
	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelPierce }, generatePike);
	candidates.add(0, H, 0, 0, SlotEnum::MainHand, { LevelSlash, LevelAoe }, generateScythe);
	candidates.add(0, 1, 0, 0, SlotEnum::MainHand, { LevelPierce }, generateBow);
	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { Nothing }, generateStaff);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { Nothing }, generateWand);
	candidates.add(0, 0, 0, 0, SlotEnum::OffHand, { Nothing }, generateDagger);
	candidates.add(0, H, 1, 0, SlotEnum::OffHand, { Nothing }, generateShield);
	candidates.add(1, 1, 0, H, SlotEnum::OffHand, { Nothing }, generateWand);
	candidates.add(1, 0, H, 0, SlotEnum::OffHand, { Nothing }, generateScroll);
	candidates.add(1, H, 1, 1, SlotEnum::OffHand, { Nothing }, generateTalisman);
	candidates.add(0, H, 1, 0, SlotEnum::Body, { Nothing }, generateBodyArmor);
	candidates.add(1, H, 1, 0, SlotEnum::Body, { Nothing }, generateCape);
	candidates.add(1, H, 1, 0, SlotEnum::Body, { Nothing }, generateProtectiveTattoos);
	candidates.add(0, H, 1, 0, SlotEnum::Head, { Nothing }, generateHelmet);
	candidates.add(1, H, 0, H, SlotEnum::Head, { Nothing }, generateCirclet);
	candidates.add(1, H, 0, 1, SlotEnum::Head, { Nothing }, generateRestoringTattoos);
	candidates.add(H, H, 1, 0, SlotEnum::Legs, { Nothing }, generateBoots);
	candidates.add(1, H, 1, H, SlotEnum::Neck, { Nothing }, generateAmulet);

	candidates.fallback(generateTrinket);
	return candidates.pick()(generate);
}

Item generateSprayCan()
{
	Item item(Generate(1, 0, SlotEnum::MainHand));
	item.slot = SlotEnum::MainHand;
	item.updateName("Spray Can");

	for (char c = 'A'; c <= 'Z'; c++)
	{
		Skill sk(item.generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 1;
		sk.duration[AttributeEnum::Scalar] = 120;
		sk.target.summons.push_back(Decoration{ "sprayPaint", std::string(1, c) });
		sk.updateName("Spray");
		item.skills.push_back(std::move(sk));
	}

	return item;
}
