#include "dnt.h"

#include <cage-core/pointerRangeHolder.h>
#include <cage-core/string.h>

Item itemGeneric(const Generate &generate);
Item itemSprayCan();
Skill skillSwordAttack(const Generate &generate);
Skill skillSaberAttack(const Generate &generate);
Skill skillMaceAttack(const Generate &generate);
Skill skillPikeAttack(const Generate &generate);
Skill skillBowAttack(const Generate &generate);
Skill skillStomp(const Generate &generate);
Skill skillFireball(const Generate &generate);
Skill skillMeteor(const Generate &generate);
Skill skillHeal(const Generate &generate);
Skill skillGeneric(const Generate &generate);

Monster::Monster(const Generate &generate) : Thing(generate){};

namespace
{
	void matchAttributesRequirements(Monster &mr)
	{
		// maximum of all requirements
		for (const Item &it : mr.equippedItems)
			for (const auto &rq : it.requirements)
				mr.attributes[rq.first] = max(mr.attributes[rq.first], rq.second);
		// subtract what is already satisfied by attributes provided by items
		for (const Item &it : mr.equippedItems)
			for (const auto &at : it.attributes)
				mr.attributes[at.first] -= at.second;
		std::erase_if(mr.attributes, [](const auto &it) { return it.second <= 0; });
	}

	uint32 availablePointsToSpend(Monster &mr)
	{
		sint32 spent = 0;
		for (const auto &it : mr.attributes)
			if (it.first <= AttributeEnum::Constitution)
				spent += it.second;
		if (mr.generate.power > spent)
			return mr.generate.power - spent;
		return 0;
	}

	void spendAttributesPoints(Monster &mr, uint32 available)
	{
		if (available == 0)
			return;

		const Real damageWeight = 2 - mr.generate.support;
		std::map<AttributeEnum, Real> weights;
		for (const Item &it : mr.equippedItems)
		{
			for (const Skill &sk : it.skills)
			{
				for (const auto &a : sk.range)
					weights[a.first] += max(a.second, 0);
				for (const auto &a : sk.radius)
					weights[a.first] += max(a.second, 0);
				for (const auto &a : sk.duration)
					weights[a.first] += max(a.second, 0);
				for (const auto &a : sk.damageAmount)
					weights[a.first] += max(a.second, 0) * damageWeight;
				for (const auto &a : sk.caster.attributes)
					for (const auto &b : a.second)
						weights[b.first] += max(b.second, 0);
				for (const auto &a : sk.target.attributes)
					for (const auto &b : a.second)
						weights[b.first] += max(b.second, 0);
			}
		}

		for (AttributeEnum a = AttributeEnum::SlashResist; a <= AttributeEnum::Constant; a = AttributeEnum((uint32)a + 1))
			weights.erase(a);

		Real sum = 0;
		for (const auto &w : weights)
			sum += w.second;
		if (sum < 1e-4)
			return; // the monster does not use any attributes

		for (const auto &w : weights)
			mr.attributes[w.first] += numeric_cast<uint32>(available * w.second / sum);

		std::erase_if(mr.attributes, [](const auto &it) { return it.second <= 0; });
	}

	void spendAttributesPoints(Monster &mr)
	{
		spendAttributesPoints(mr, max(availablePointsToSpend(mr), 10u));
	}

	void setupResistances(Monster &mr, Real scale)
	{
		for (AttributeEnum attr : { AttributeEnum::SlashResist, AttributeEnum::PierceResist, AttributeEnum::FireResist, AttributeEnum::PoisonResist, AttributeEnum::ElectricResist })
			if (randomChance() < 0.8)
				mr.attributes[attr] += numeric_cast<uint32>(mr.generate.power * scale * randomRange(0.8, 1.2));
	}

	Monster generateHorrorBase(const Generate &generate, const char *name)
	{
		Monster mr(generate);
		mr.updateName(name, 0.4);
		mr.icon = toLower(String(name));
		mr.algorithm = toLower(String(name));
		mr.faction = "horror";

		mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
		setupResistances(mr, 0.1);

		// gain attributes by simulating equipping items
		for (SlotEnum slot : { SlotEnum::MainHand, SlotEnum::OffHand, SlotEnum::Head, SlotEnum::Body, SlotEnum::Legs, SlotEnum::Neck })
		{
			Generate g = generate;
			g.slot = slot;
			Item item = itemGeneric(g);
			for (const auto &it : item.attributes)
				if (it.second > 0)
					mr.attributes[it.first] += it.second;
			mr.addOther(item, 0.7);
		}

		return mr;
	}
}

Monster monsterZombie(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Zombie");
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::ElectricResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "claws";

	{
		Skill sk = skillSwordAttack(generate);
		sk.damageAmount[AttributeEnum::Strength] *= 0.7;
		if (generate.level > LevelPoison && randomChance() < 0.35)
		{
			sk.name = "Bite";
			sk.range.erase(AttributeEnum::Constant);
			sk.damageType = DamageTypeEnum::Poison;
		}
		else
		{
			sk.name = "Scratch";
			sk.range[AttributeEnum::Constant] = 1;
			sk.damageType = DamageTypeEnum::Slash;
		}
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Claws");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterSkeleton(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Skeleton");
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::PoisonResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "bow";

	{
		Skill sk = skillBowAttack(generate);
		if (generate.level > LevelFire && randomChance() < 0.2)
			sk.damageType = DamageTypeEnum ::Fire;
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Bow");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterOgre(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Ogre");
	mr.attributes[AttributeEnum::SlashResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::PoisonResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "mace";

	{
		Skill sk = skillMaceAttack(generate);
		sk.range[AttributeEnum::Constant] = 2;
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > max(LevelKnockback, LevelAoe))
	{
		Skill sk = skillStomp(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Mace");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterTroll(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Troll");
	mr.attributes[AttributeEnum::FireResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::PoisonResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "spear";

	{
		Skill sk = skillPikeAttack(generate);
		sk.range[AttributeEnum::Constant] = 3;
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	{
		Skill sk(generate);
		sk.name = "Regenerate";
		sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Quick")) * 0.1;
		sk.flags.passive = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Spear");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterVampire(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Vampire");
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::PoisonResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "claws";

	{
		Skill sk(generate);
		sk.name = "Bite";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Constant] = 1;
		sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Sucking")) * 0.3;
		sk.target.attributes[AttributeEnum::Life][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Deep")) * -0.7; // bypasses resistances
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	{
		Skill sk(generate);
		sk.name = "Jump";
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.1;
		sk.range[AttributeEnum::Constant] = 3;
		sk.caster.flags.movement = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > LevelMagic)
	{
		Skill sk = skillHeal(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Claws");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterMedusa(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Medusa");
	mr.attributes[AttributeEnum::SlashResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::ElectricResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "bow";

	{
		Skill sk = skillBowAttack(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > max(LevelStun, LevelAoe))
	{
		Skill sk(generate);
		sk.name = "Petrify";
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Constant] = 3;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.05;
		sk.radius[AttributeEnum::Constant] = 2;
		sk.target.flags.stun = true;
		sk.addPower(1);
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Bow");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterSuccubus(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Succubus");
	mr.attributes[AttributeEnum::FireResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::ElectricResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "claws";

	{
		Skill sk = skillSwordAttack(generate);
		sk.name = "Scratch";
		sk.range[AttributeEnum::Constant] = 1;
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	{
		Skill sk(generate);
		sk.name = "Song";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Luring")) * 0.1;
		sk.range[AttributeEnum::Constant] = 4;
		sk.target.flags.movement = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > LevelPoison)
	{
		Skill sk(generate);
		sk.name = "Kiss";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Constant] = 1;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Wet")) * 0.7;
		sk.damageType = DamageTypeEnum::Poison;
		if (generate.level > LevelDuration)
			sk.duration[AttributeEnum::Constant] = interpolate(1.0, 4.0, sk.addPower(0.8, "Lasting"));
		sk.caster.flags.movement = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Claws");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterImp(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Imp");
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::FireResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "claws";

	{
		Skill sk = skillSwordAttack(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > LevelFire)
	{
		Skill sk = skillFireball(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	if (generate.level > max(max(LevelFire, LevelDuration), max(LevelAoe, LevelGroundEffect)))
	{
		Skill sk = skillMeteor(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Claws");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterGhost(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Ghost");
	mr.attributes[AttributeEnum::SlashResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "glow";

	{
		Skill sk(generate);
		sk.name = "Terror";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Constant] = 1;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Shocking")) * 0.8;
		if (generate.level > LevelElectric && randomChance() < 0.6)
			sk.damageType = DamageTypeEnum::Electric;
		else if (generate.level > LevelFire)
			sk.damageType = DamageTypeEnum::Fire;
		else
			sk.damageType = DamageTypeEnum::Pierce;
		if (generate.level > LevelStun)
		{
			sk.target.flags.stun = true;
			sk.addPower(1, "Stunning");
		}
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Glow");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterBanshee(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Banshee");
	mr.attributes[AttributeEnum::PierceResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::ElectricResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "claws";

	for (AttributeEnum attr : { AttributeEnum::Strength, AttributeEnum::Dexterity, AttributeEnum::Intelligence, AttributeEnum::SlashResist, AttributeEnum::PierceResist })
	{
		Skill sk(generate);
		sk.name = "Scream";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Deafening")) * 0.1;
		sk.range[AttributeEnum::Constant] = 4;
		if (generate.level > LevelDuration)
			sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(0.8, "Lasting"));
		sk.target.attributes[attr][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Crippling")) * -0.15;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Claws");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterLich(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Lich");
	mr.attributes[AttributeEnum::SlashResist] += generate.power * 0.2;
	mr.attributes[AttributeEnum::FireResist] += generate.power * 0.2;

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "staff";

	for (AttributeEnum attr : { AttributeEnum::SlashResist, AttributeEnum::PierceResist, AttributeEnum::FireResist, AttributeEnum::PoisonResist, AttributeEnum::ElectricResist })
	{
		Skill sk(generate);
		sk.name = "Resist";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
		sk.range[AttributeEnum::Constant] = 8;
		if (generate.level > LevelDuration)
			sk.duration[AttributeEnum::Constant] = interpolate(3.0, 8.0, sk.addPower(0.9, "Lasting"));
		sk.target.attributes[attr][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Thorough")) * 0.4;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Staff");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

Monster monsterPhantom(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Phantom");

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.icon = "staff";

	for (uint32 i = 0; i < 5; i++)
	{
		Skill sk = skillGeneric(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	it.updateName("Staff");
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	return mr;
}

namespace
{
	Monster selectHorrorMonster(const Generate &generate)
	{
		if (generate.level > LevelSummoning && randomChance() < 0.01)
			return monsterPhantom(generate);
		if (generate.magic < 0.5)
		{ // mundane
			if (generate.support < 0.5)
			{ // combat
				if (generate.defensive < 0.5)
				{ // offensive
					if (generate.ranged < 0.5)
					{ // melee
						return monsterZombie(generate);
					}
					else
					{ // ranged
						return monsterSkeleton(generate);
					}
				}
				else
				{ // defensive
					if (generate.ranged < 0.5)
					{ // melee
						return monsterOgre(generate);
					}
					else
					{ // ranged
						return monsterTroll(generate);
					}
				}
			}
			else
			{ // support
				if (generate.ranged < 0.5)
				{ // melee
					return monsterVampire(generate);
				}
				else
				{ // ranged
					return monsterMedusa(generate);
				}
			}
		}
		else
		{ // magic
			if (generate.support < 0.5)
			{ // combat
				if (generate.defensive < 0.5)
				{ // offensive
					if (generate.ranged < 0.5)
					{ // melee
						return monsterSuccubus(generate);
					}
					else
					{ // ranged
						return monsterImp(generate);
					}
				}
				else
				{ // defensive
					return monsterGhost(generate);
				}
			}
			else
			{ // support
				if (generate.defensive < 0.5)
				{ // offensive
					return monsterBanshee(generate);
				}
				else
				{ // defensive
					return monsterLich(generate);
				}
			}
		}
	}

	Monster generateOutlawBase(const Generate &generate, const char *name)
	{
		Monster mr(generate);
		mr.updateName(name, 0.4);
		mr.icon = toLower(String(name));
		mr.algorithm = toLower(String(name));
		mr.faction = "outlaw";

		mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
		mr.attributes[AttributeEnum::Mana] = numeric_cast<sint32>((generate.power + randomRange(30, 50)) * (0.3 + generate.magic * 0.7));
		mr.attributes[AttributeEnum::Stamina] = numeric_cast<sint32>((generate.power + randomRange(30, 50)) * (1 - generate.magic * 0.5));
		setupResistances(mr, 0.2);

		const auto &equip = [&](SlotEnum slot, Real weight = 1)
		{
			Generate g = generate;
			g.slot = slot;
			Item item = itemGeneric(g);
			mr.addOther(item, 1);
			mr.equippedItems.push_back(std::move(item));
		};
		equip(SlotEnum::MainHand, 1.2);
		if (randomChance() < 0.8)
			equip(SlotEnum::OffHand);
		equip(SlotEnum::Head);
		equip(SlotEnum::Body);
		equip(SlotEnum::Legs);
		if (randomChance() < 0.5)
			equip(SlotEnum::Neck);

		matchAttributesRequirements(mr);
		spendAttributesPoints(mr);
		mr.updateName();
		return mr;
	}
}

Monster monsterBarbarian(const Generate &generate)
{
	return generateOutlawBase(generate, "Barbarian");
}

Monster monsterAssassin(const Generate &generate)
{
	return generateOutlawBase(generate, "Assassin");
}

Monster monsterBandit(const Generate &generate)
{
	return generateOutlawBase(generate, "Bandit");
}

Monster monsterRogue(const Generate &generate)
{
	return generateOutlawBase(generate, "Rogue");
}

Monster monsterSaboteur(const Generate &generate)
{
	return generateOutlawBase(generate, "Saboteur");
}

Monster monsterDruid(const Generate &generate)
{
	return generateOutlawBase(generate, "Druid");
}

Monster monsterWarlock(const Generate &generate)
{
	return generateOutlawBase(generate, "Warlock");
}

Monster monsterOccultist(const Generate &generate)
{
	return generateOutlawBase(generate, "Occultist");
}

Monster monsterShaman(const Generate &generate)
{
	return generateOutlawBase(generate, "Shaman");
}

namespace
{
	Monster selectOutlawMonster(const Generate &generate)
	{
		if (generate.magic < 0.5)
		{ // mundane
			if (generate.support < 0.5)
			{ // combat
				if (generate.defensive < 0.5)
				{ // offensive
					if (generate.ranged < 0.5)
					{ // melee
						return monsterBarbarian(generate);
					}
					else
					{ // ranged
						return monsterAssassin(generate);
					}
				}
				else
				{ // defensive
					return monsterBandit(generate);
				}
			}
			else
			{ // support
				if (generate.ranged < 0.5)
				{ // melee
					return monsterRogue(generate);
				}
				else
				{ // ranged
					return monsterSaboteur(generate);
				}
			}
		}
		else
		{ // magic
			if (generate.support < 0.5)
			{ // combat
				if (generate.ranged < 0.5)
				{ // melee
					return monsterDruid(generate);
				}
				else
				{ // ranged
					return monsterWarlock(generate);
				}
			}
			else
			{ // support
				if (generate.defensive < 0.5)
				{ // offensive
					return monsterOccultist(generate);
				}
				else
				{ // defensive
					return monsterShaman(generate);
				}
			}
		}
	}

	Monster generateMonsterImpl(const Generate &generate_, Monster (*generator)(const Generate &))
	{
		CAGE_ASSERT(generate_.valid());
		CAGE_ASSERT(generate_.slot == SlotEnum::None);

		// additional empower
		// https://www.wolframalpha.com/input?i=plot+exp%28%28x+-+100%29+%2F+200%29%3B+x+%3D+100+..+1000
		const Real empower = max(pow((Real(generate_.power) - 100) / 200), 1);
		Generate generate = generate_;
		generate.power = numeric_cast<uint32>(generate_.power * empower);

		// generate several monsters and pick the strongest
		// 0 .. 39 -> 1 attempt
		// 40 .. 59 -> 2 attempts
		// 60 .. 79 -> 3 attempts
		// 80 .. 99 -> 4 attempts
		// 100 and more -> 5 attempts
		Monster mr = generator(generate);
		const uint32 attempts = min(generate_.power / 20, 5); // use the original power
		for (uint32 i = 1; i < attempts; i++) // starting from one -> one attempt has already been generated above
		{
			Monster mr2 = generator(generate);
			if (mr2.goldCost > mr.goldCost)
				std::swap(mr, mr2);
		}

		// loot
		if (randomChance() < 0.15)
			mr.onDeath.push_back(itemGeneric(Generate(generate_.level, generate_.powerOffset()))); // use the original power
		mr.score = mr.goldCost / empower;

		return mr;
	}
}

Monster monsterGeneric(const Generate &generate)
{
	return generateMonsterImpl(generate, isHorrorFloor(generate.level) > 0.5 ? &selectHorrorMonster : &selectOutlawMonster);
}

Monster monsterBallista(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Ballista");
	mr.icon = "totem";
	mr.faction = "inherited";

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.name = "Bow";
	it.icon = "bow";
	{
		Skill sk = skillBowAttack(generate);
		sk.cost.clear();
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	mr.updateName("Ballista");
	return mr;
}

Monster monsterLandMine(const Generate &generate)
{
	Monster mr = generateHorrorBase(generate, "Land Mine");
	mr.icon = "landMine";
	mr.algorithm = "none";
	mr.faction = "neutral";

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.name = "Trigger";
	it.icon = "wire";
	{
		Skill sk(generate);
		sk.name = "Ticking";
		sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Constant] = mr.attributes[AttributeEnum::Life] * -0.1; // duration 10 ticks
		sk.flags.passive = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}
	{
		Skill sk(generate);
		sk.name = "Explosion";
		sk.radius[AttributeEnum::Constant] = interpolate(4.0, 8.0, sk.addPower(1, "Vast"));
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Destructive")) * 0.2;
		sk.damageType = DamageTypeEnum::Fire;
		mr.addOther(sk, 1);
		mr.onDeath.push_back(std::move(sk));
	}
	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	spendAttributesPoints(mr, generate.power);
	mr.updateName("Land Mine");
	return mr;
}

Monster monsterMinion(const Generate &generate_)
{
	CAGE_ASSERT(generate_.valid());
	CAGE_ASSERT(generate_.slot == SlotEnum::None);

	Generate g = generate_;
	g.magic = 0; // minion cannot be magic -> cannot summon other minions

	// no empower
	// no rerolls
	// no loot
	// no score

	Monster mr = isHorrorFloor(g.level) > 0.5 ? selectHorrorMonster(g) : selectOutlawMonster(g);
	mr.faction = "inherited";
	return mr;
}

Monster monsterChest(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot == SlotEnum::None);

	Monster mr(generate);
	mr.name = "Chest";
	mr.icon = "chest";
	mr.algorithm = "none";
	mr.faction = "neutral";
	mr.attributes[AttributeEnum::Life] = 1;

	// loot
	const uint32 cnt = randomRange(5u, 10u);
	for (uint32 i = 0; i < cnt; i++)
		mr.onDeath.push_back(itemGeneric(Generate(generate.level, generate.powerOffset())));

	// trap
	if (randomChance() < 0.05)
		mr.onDeath.push_back(monsterGeneric(Generate(generate.level, generate.powerOffset()))); // empowered monster

	mr.onDeath.push_back(Decoration{ "openChest" });

	return mr;
}

namespace
{
	void addDeadBody(Monster &mr)
	{
		mr.onDeath.push_back(Decoration{ "deadBody", mr.name });
	}
}

std::string floorBossName(uint32 level)
{
	return (Stringizer() + "Guardian Of " + level + "th Floor").value.c_str();
}

namespace
{
	const char *floorBossIconImpl(uint32 bossIndex)
	{
		switch (bossIndex)
		{
			case 1:
				return "Sword";
			case 2:
				return "Bow";
			case 3:
				return "Scythe";
			case 4:
				return "Magic";
			case 5:
				return "Duration";
			case 6:
				return "Support";
			case 7:
				return "Poison";
			case 8:
				return "Ground";
			case 9:
				return "Stun";
			case 10:
				return "Summon";
			case 11:
				return "Electric";
			default:
				return "Scroll";
		}
	}

}

Decoration floorBossStatue(uint32 bossIndex)
{
	return Decoration{ String("statue") + floorBossIconImpl(bossIndex), floorBossName(bossIndexToLevel(bossIndex)) };
}

Decoration floorBossTrophy(uint32 bossIndex)
{
	return Decoration{ String("trophy") + floorBossIconImpl(bossIndex), floorBossIconImpl(bossIndex) };
}

Monster monsterFloorBoss(uint32 level)
{
	const auto &levelSwitch = [level](uint32 floor) -> Real
	{
		floor = bossIndexToLevel(levelToBossIndex(floor) + 1); // round up to actual boss
		CAGE_ASSERT(isLevelBoss(floor));
		if (level < floor)
			return 0;
		if (level == floor)
			return 1;
		return Generate::randomTowardsTarget(randomChance() < 0.5 ? 0 : 1);
	};

	Generate g = Generate(level, numeric_cast<sint32>(pow(level / 5, 1.3)));
	g.magic = levelSwitch(LevelMagic);
	g.ranged = levelSwitch(LevelRanged);
	g.defensive = levelSwitch(LevelDefensive);
	g.support = levelSwitch(LevelSupport);

	Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
	mr.attributes[AttributeEnum::Life] *= 5;
	mr.icon = Stringizer() + "guardian" + floorBossIconImpl(levelToBossIndex(level));
	mr.algorithm = "guardian";
	mr.faction = "monster";

	mr.updateName(floorBossName(level));
	addDeadBody(mr);
	return mr;
}

Holder<PointerRange<Monster>> generateAntiHeroes()
{
	PointerRangeHolder<Monster> result;

	{ // vit - barbarian
		Generate g(100, 50);
		g.magic = 0;
		g.ranged = 0;
		g.defensive = 0;
		g.support = 0;
		Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
		mr.attributes[AttributeEnum::Life] *= 5;
		mr.name = "Vít";
		mr.icon = "barbarian";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		addDeadBody(mr);
		result.push_back(std::move(mr));
	}

	{ // eva - sorcerer
		Generate g(100, 50);
		g.magic = 1;
		g.ranged = 1;
		g.defensive = 0;
		g.support = 0;
		Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
		mr.attributes[AttributeEnum::Life] *= 5;
		mr.name = "Eva";
		mr.icon = "sorcerer";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		addDeadBody(mr);
		result.push_back(std::move(mr));
	}

	{ // simon - druid
		Generate g(100, 50);
		g.ranged = 0;
		g.defensive = 1;
		g.support = 0;
		Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
		mr.attributes[AttributeEnum::Life] *= 5;
		mr.name = "Šimon";
		mr.icon = "druid";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		addDeadBody(mr);
		result.push_back(std::move(mr));
	}

	{ // tomas - necromancer
		Generate g(100, 50);
		g.magic = 1;
		g.support = 1;
		Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
		mr.attributes[AttributeEnum::Life] *= 5;
		mr.name = "Tomáš";
		mr.icon = "necromancer";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		addDeadBody(mr);
		result.push_back(std::move(mr));
	}

	// druid wolves
	for (uint32 i = 0; i < 3; i++)
	{
		Generate g(100, 40);
		g.magic = 0;
		g.ranged = 0;
		g.support = 0;
		Monster mr = generateMonsterImpl(g, &monsterZombie);
		mr.attributes[AttributeEnum::Life] *= 3;
		mr.updateName("Wolf");
		mr.icon = "wolf";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		result.push_back(std::move(mr));
	}

	// necromancer skeletons
	for (uint32 i = 0; i < 7; i++)
	{
		Generate g(100, 35);
		g.magic = 0;
		g.ranged = 1;
		g.defensive = 0;
		g.support = 0;
		Monster mr = generateMonsterImpl(g, &monsterSkeleton);
		mr.attributes[AttributeEnum::Life] *= 2;
		mr.updateName("Skeleton");
		mr.icon = "skeleton";
		mr.algorithm = "antihero";
		mr.faction = "monster";
		result.push_back(std::move(mr));
	}

	return result;
}

Monster monsterButcher(uint32 level)
{
	Generate g = Generate(level, level / 3);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 0;
	g.support = 0;

	Monster mr = generateMonsterImpl(g, &monsterBarbarian);
	mr.icon = "butcher";
	mr.algorithm = "butcher";
	mr.faction = "monster";

	// todo add some skills

	mr.updateName("Butcher");
	addDeadBody(mr);
	return mr;
}

Monster monsterWitch(uint32 level)
{
	Generate g = Generate(level, level / 10);
	g.magic = 1;
	g.ranged = 1;
	g.defensive = 0;
	g.support = randomChance();

	Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
	mr.icon = "witch";
	mr.algorithm = "witch";
	mr.faction = "monster";

	// todo add broomstick skill

	mr.updateName("Witch");
	addDeadBody(mr);
	return mr;
}

Monster monsterTemplar(uint32 level)
{
	Generate g = Generate(level, level / 10);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 1;
	g.support = randomChance();

	Monster mr = generateMonsterImpl(g, &selectOutlawMonster);
	mr.icon = "templar";
	mr.algorithm = "templar";
	mr.faction = "templar";
	mr.score = 0;

	mr.updateName("Templar Knight");
	addDeadBody(mr);
	return mr;
}

namespace
{
	Monster generateZerglingImpl(const Generate &generate)
	{
		Monster mr = generateHorrorBase(generate, "Zergling");
		mr.faction = "monster";

		Item it(generate);
		it.slot = SlotEnum::MainHand;
		it.name = "Claws";
		it.icon = "claws";

		{
			Skill sk = skillSaberAttack(generate);
			sk.name = "Scratch";
			sk.range[AttributeEnum::Constant] = 3;
			sk.cost.clear();
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		mr.addOther(it, 1);
		mr.equippedItems.push_back(std::move(it));

		spendAttributesPoints(mr, generate.power);
		mr.updateName();
		return mr;
	}
}

Monster monsterZergling(uint32 level)
{
	Generate g = Generate(level, 0);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 0;
	g.support = 0;
	return generateMonsterImpl(g, &generateZerglingImpl);
}

Monster monsterHealingTotem(uint32 level)
{
	Monster mr = Monster(Generate(level, 0));
	mr.icon = "totem";
	mr.algorithm = "none";
	mr.faction = "neutral";

	mr.attributes[AttributeEnum::Life] = level * 5 + randomRange(0, 200);

	Item it(mr.generate);
	it.slot = SlotEnum::MainHand;
	it.name = "Glow";
	it.icon = "glow";

	{
		Skill sk(mr.generate);
		sk.name = "Blessed Healing";
		sk.radius[AttributeEnum::Constant] = randomRange(2.0, 4.0);
		sk.duration[AttributeEnum::Constant] = randomRange(1.0, 2.0);
		sk.target.attributes[AttributeEnum::Life][AttributeEnum::Constant] = level * 0.1;
		sk.caster.flags.groundEffect = true;
		sk.flags.passive = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	mr.updateName("Healing Totem");
	return mr;
}

namespace
{
	Monster generateHydraImpl(const Generate &generate, uint32 nested)
	{
		Monster mr = generateHorrorBase(generate, "Hydra");
		mr.faction = "monster";

		Item it(generate);
		it.slot = SlotEnum::MainHand;
		it.name = "Claws";
		it.icon = "claws";

		{
			Skill sk(generate);
			sk.targetType = SkillTargetEnum::Character;
			if (generate.ranged > 0.5)
			{
				sk.name = "Smash";
				sk.range[AttributeEnum::Constant] = 3;
				sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Accurate"));
				sk.damageType = DamageTypeEnum::Slash;
			}
			else
			{
				sk.name = "Bite";
				sk.range[AttributeEnum::Constant] = 1;
				sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Strong"));
				sk.damageType = DamageTypeEnum::Pierce;
			}
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Jump";
			sk.targetType = SkillTargetEnum::Position;
			sk.range[AttributeEnum::Constant] = 3 + nested / 2;
			sk.caster.flags.movement = true;
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		mr.addOther(it, 1);
		mr.equippedItems.push_back(std::move(it));

		if (nested < 5)
		{
			Generate g = generate;
			g.power = 9 * g.power / 10;
			g.ranged = randomChance();
			mr.onDeath.push_back(generateHydraImpl(g, nested + 1));
			mr.onDeath.push_back(generateHydraImpl(g, nested + 1));
		}

		spendAttributesPoints(mr, generate.power);
		mr.updateName();
		return mr;
	}

	Monster generateHydraImplEntry(const Generate &generate)
	{
		return generateHydraImpl(generate, 0);
	}
}

Monster monsterHydra(uint32 level)
{
	Generate g = Generate(level, 0);
	g.magic = 0;
	g.ranged = randomChance();
	g.defensive = 0;
	g.support = 0;
	return generateMonsterImpl(g, &generateHydraImplEntry);
}

Monster monsterSatyr(uint32 level)
{
	Generate generate = Generate(level, 0);
	generate.magic = 1;
	generate.ranged = 0;
	generate.defensive = 0;
	generate.support = 1;

	Monster mr(generate);
	mr.icon = "satyr";
	mr.algorithm = "random";
	mr.faction = "neutral";

	mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);

	Item it(generate);
	it.slot = SlotEnum::MainHand;
	it.name = "Claws";
	it.icon = "claws";

	{
		Skill sk(generate);
		sk.name = "Abduct";
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Constant] = 30;
		sk.caster.flags.movement = true;
		sk.target.flags.movement = true;
		it.addOther(sk, 1);
		it.skills.push_back(std::move(sk));
	}

	mr.addOther(it, 1);
	mr.equippedItems.push_back(std::move(it));

	mr.updateName("Satyr");
	return mr;
}

namespace
{
	Monster generateElementalImpl(const Generate &generate)
	{
		Monster mr = generateHorrorBase(generate, "Elemental");
		mr.faction = "monster";

		const auto icon = [&]()
		{
			std::vector<std::pair<DamageTypeEnum, AttributeEnum>> cns;
			if (generate.level > LevelFire)
				cns.push_back({ DamageTypeEnum::Fire, AttributeEnum::FireResist });
			if (generate.level > LevelPoison)
				cns.push_back({ DamageTypeEnum::Poison, AttributeEnum::PoisonResist });
			if (generate.level > LevelElectric)
				cns.push_back({ DamageTypeEnum::Electric, AttributeEnum::ElectricResist });
			CAGE_ASSERT(!cns.empty());
			return cns[randomRange(std::size_t(), cns.size())];
		}();

		mr.attributes[icon.second] += 1000000;

		Item it(generate);
		it.slot = SlotEnum::MainHand;
		it.name = "Glow";
		it.icon = "glow";

		{
			Skill sk(generate);
			sk.name = "Shedding";
			sk.radius[AttributeEnum::Constant] = 2;
			sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Damaging")) * 0.5;
			sk.damageType = icon.first;
			if (generate.level > LevelDuration)
				sk.duration[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Lasting")) * 0.1;
			if (generate.level > LevelGroundEffect)
				sk.caster.flags.groundEffect = true;
			sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Constant] = generate.power * 0.1;
			sk.flags.passive = true;
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Jump";
			sk.targetType = SkillTargetEnum::Position;
			sk.range[AttributeEnum::Constant] = 3;
			sk.caster.flags.movement = true;
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		mr.addOther(it, 1);
		mr.equippedItems.push_back(std::move(it));

		spendAttributesPoints(mr, generate.power);
		mr.updateName();
		return mr;
	}
}

Monster monsterElemental(uint32 level)
{
	Generate g = Generate(level, 0);
	g.magic = 1;
	g.ranged = 0;
	g.defensive = 0;
	g.support = 0;
	return generateMonsterImpl(g, &generateElementalImpl);
}

Monster monsterVandal(uint32 level)
{
	Generate generate = Generate(level, 0);
	generate.magic = 0;
	generate.ranged = 0;
	generate.defensive = 0;
	generate.support = 1;

	Monster mr(generate);
	mr.icon = "vandal";
	mr.algorithm = "random";
	mr.faction = "neutral";

	mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
	mr.equippedItems.push_back(itemSprayCan());

	mr.updateName("Vandal");
	return mr;
}

Monster monsterNuclearBomb(uint32 level)
{
	Generate generate = Generate(level, 0);
	generate.magic = 0;
	generate.ranged = 1;
	generate.defensive = 0;
	generate.support = 0;

	Monster mr(generate);
	mr.icon = "nuclearBomb";
	mr.algorithm = "none";
	mr.faction = "monster";

	mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);

	{
		Skill sk(generate);
		sk.name = "Explode";
		sk.radius[AttributeEnum::Constant] = 100;
		sk.damageAmount[AttributeEnum::Constant] = generate.level * 10 + 100;
		sk.damageType = DamageTypeEnum::Fire;
		mr.onDeath.push_back(std::move(sk));
	}

	{
		Skill sk(generate);
		sk.name = "Fallout";
		sk.radius[AttributeEnum::Constant] = 100;
		sk.duration[AttributeEnum::Constant] = 1000000;
		sk.damageAmount[AttributeEnum::Constant] = generate.level / 5 + 5;
		sk.damageType = DamageTypeEnum::Poison;
		sk.target.flags.groundEffect = true;
		mr.onDeath.push_back(std::move(sk));
	}

	mr.updateName("Nuclear Bomb");
	return mr;
}

Monster monsterGollum(uint32 level)
{
	Generate generate = Generate(level, 0);
	generate.magic = 0;
	generate.ranged = 0;
	generate.defensive = 0;
	generate.support = 0;

	Monster mr(generate);
	mr.icon = "gollum";
	mr.algorithm = "random";
	mr.faction = "neutral";

	mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);

	const auto &item = [](uint32 level)
	{
		Generate g(level, 0);
		g.magic = 1;
		g.ranged = 0;
		g.defensive = 1;
		g.support = 0;

		Item it(g);
		it.slot = SlotEnum::OffHand;
		it.name = "Ring";
		it.icon = "ring";

		{
			Skill sk(g);
			sk.name = "Invisibility";
			sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Constant] = g.power * 0.1 + 5;
			sk.flags.passive = true;
			it.addOther(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		return it;
	};

	{
		Item it = item(level);
		mr.addOther(it, 1);
		mr.equippedItems.push_back(std::move(it));
	}

	mr.onDeath.push_back(item(level));

	mr.updateName("Gollum");
	return mr;
}
