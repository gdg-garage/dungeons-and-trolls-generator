#include "dnt.h"

namespace
{
	void matchAttributesRequirements(Monster &mr)
	{
		for (const Item &it : mr.equippedItems)
			for (const auto &rq : it.requirements)
				mr.attributes[rq.first] = max(mr.attributes[rq.first], rq.second);
	}

	void spendAttributesPoints(Monster &mr)
	{
		const uint32 available = [&]() -> uint32
		{
			sint32 spent = 0;
			for (const auto &it : mr.attributes)
				if (it.first <= AttributeEnum::Constitution)
					spent += it.second;
			if (mr.generate.power > spent)
				return mr.generate.power - spent;
			return 0;
		}();
		if (available == 0)
			return;

		std::map<AttributeEnum, Real> weights;
		for (const Item &it : mr.equippedItems)
		{
			for (const Skill &sk : it.skills)
			{
				for (const auto &a : sk.range)
					weights[a.first] += a.second;
				for (const auto &a : sk.radius)
					weights[a.first] += a.second;
				for (const auto &a : sk.duration)
					weights[a.first] += a.second;
				for (const auto &a : sk.damageAmount)
					weights[a.first] += a.second;
				for (const auto &a : sk.casterAttributes)
					for (const auto &b : a.second)
						weights[b.first] += b.second;
				for (const auto &a : sk.targetAttributes)
					for (const auto &b : a.second)
						weights[b.first] += abs(b.second);
			}
		}

		for (AttributeEnum a = AttributeEnum::SlashArmor; a <= AttributeEnum::Scalar; a = AttributeEnum((uint32)a + 1))
			weights.erase(a);

		Real sum = 0;
		for (const auto &w : weights)
			sum += w.second;
		if (sum < 1)
			return; // the monster does not use any attributes

		for (const auto &w : weights)
			mr.attributes[w.first] += numeric_cast<uint32>(available * w.second / sum);
	}

	// todo remove
	Monster generateRandomMonster(const Generate &generate)
	{
		Monster mr(generate);

		const uint32 level = generate.power;

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

		//if (randomChance() < 0.1)
		//	mr.onDeath.push_back(generateMinion(generate));
		//if (randomChance() < 0.1)
		//	mr.onDeath.push_back(generateSkill(Generate(generate.level, generate.powerOffset(), SlotEnum::MainHand)));

		matchAttributesRequirements(mr);
		spendAttributesPoints(mr);

		return mr;
	}
}

namespace
{
	// mundane melee offensive combat
	// teeth, poison
	// slashing armor, poison resist
	Monster generateZombie(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Zombie");
		mr.icon = "zombie";
		mr.faction = "horror";
		return mr;
	}

	// mundane ranged offensive combat
	// bow, fire arrows
	// piercing armor, poison resist
	Monster generateSkeleton(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Skeleton");
		mr.icon = "skeleton";
		mr.faction = "horror";
		return mr;
	}

	// mundane melee defensive combat
	// mace + shield
	// knockback stomp
	Monster generateOgre(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Ogre");
		mr.icon = "ogre";
		mr.faction = "horror";
		return mr;
	}

	// mundane ranged defensive combat
	// speer
	// regeneration
	Monster generateTroll(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Troll");
		mr.icon = "troll";
		mr.faction = "horror";
		return mr;
	}

	// mundane melee --- support
	// teeth, lifeleech
	// healing
	Monster generateVampire(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Vampire");
		mr.icon = "vampire";
		mr.faction = "horror";
		return mr;
	}

	// mundane ranged --- support
	// bow
	// aoe stun stare
	Monster generateMedusa(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Medusa");
		mr.icon = "medusa";
		mr.faction = "horror";
		return mr;
	}

	// magic melee offensive combat
	// claws, poison, pull enemy
	//
	Monster generateSuccubus(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Succubus");
		mr.icon = "succubus";
		mr.faction = "horror";
		return mr;
	}

	// magic ranged offensive combat
	// fireballs
	// fire resist
	Monster generateImp(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Imp");
		mr.icon = "imp";
		mr.faction = "horror";
		return mr;
	}

	// magic --- defensive combat
	// electric shock
	// slashing and piercing armor
	Monster generateGhost(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Ghost");
		mr.icon = "ghost";
		mr.faction = "horror";
		return mr;
	}

	// magic --- offensive support
	// curses
	// electric resist
	Monster generateBanshee(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Banshee");
		mr.icon = "banshee";
		mr.faction = "horror";
		return mr;
	}

	// magic --- defensive support
	// buffs
	// fire resist
	Monster generateLich(const Generate &generate)
	{
		Monster mr = generateRandomMonster(generate);
		mr.updateName("Lich");
		mr.icon = "lich";
		mr.faction = "horror";
		return mr;
	}

	Monster generateHorror(const Generate &generate)
	{
		if (generate.magic < 0.5)
		{ // mundane
			if (generate.support < 0.5)
			{ // combat
				if (generate.defensive < 0.5)
				{ // offensive
					if (generate.ranged < 0.5)
					{ // melee
						return generateZombie(generate);
					}
					else
					{ // ranged
						return generateSkeleton(generate);
					}
				}
				else
				{ // defensive
					if (generate.ranged < 0.5)
					{ // melee
						return generateOgre(generate);
					}
					else
					{ // ranged
						return generateTroll(generate);
					}
				}
			}
			else
			{ // support
				if (generate.ranged < 0.5)
				{ // melee
					return generateVampire(generate);
				}
				else
				{ // ranged
					return generateMedusa(generate);
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
						return generateSuccubus(generate);
					}
					else
					{ // ranged
						return generateImp(generate);
					}
				}
				else
				{ // defensive
					return generateGhost(generate);
				}
			}
			else
			{ // support
				if (generate.defensive < 0.5)
				{ // offensive
					return generateBanshee(generate);
				}
				else
				{ // defensive
					return generateLich(generate);
				}
			}
		}
	}
}

namespace
{
	Monster generateOutlawBase(const Generate &generate)
	{
		Monster mr(generate);

		mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
		mr.attributes[AttributeEnum::Mana] = numeric_cast<uint32>((generate.power + randomRange(30, 50)) * generate.magic);
		mr.attributes[AttributeEnum::Stamina] = numeric_cast<uint32>((generate.power + randomRange(30, 50)) * (1 - generate.magic));

		const auto &equip = [&](SlotEnum slot, Real weight = 1)
		{
			Generate g = generate;
			g.slot = slot;
			Item item = generateItem(g);
			mr.addPower(item, 1);
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

		return mr;
	}

	// mundane melee offensive combat
	Monster generateBarbarian(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Barbarian");
		mr.icon = "barbarian";
		mr.faction = "outlaw";
		return mr;
	}

	// mundane ranged offensive combat
	Monster generateAssassin(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Assassin");
		mr.icon = "assassin";
		mr.faction = "outlaw";
		return mr;
	}

	// mundane --- defensive combat
	Monster generateBandit(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Bandit");
		mr.icon = "bandit";
		mr.faction = "outlaw";
		return mr;
	}

	// mundane melee --- support
	Monster generateRogue(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Rogue");
		mr.icon = "rogue";
		mr.faction = "outlaw";
		return mr;
	}

	// mundane ranged --- support
	Monster generateSaboteur(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Saboteur");
		mr.icon = "saboteur";
		mr.faction = "outlaw";
		return mr;
	}

	// magic melee --- combat
	Monster generateDruid(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Druid");
		mr.icon = "druid";
		mr.faction = "outlaw";
		return mr;
	}

	// magic ranged offensive combat
	Monster generateWarlock(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Warlock");
		mr.icon = "warlock";
		mr.faction = "outlaw";
		return mr;
	}

	// magic ranged offensive support
	Monster generateOccultist(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Occultist");
		mr.icon = "occultist";
		mr.faction = "outlaw";
		return mr;
	}

	// magic ranged defensive support
	Monster generateShaman(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Shaman");
		mr.icon = "shaman";
		mr.faction = "outlaw";
		return mr;
	}

	// magic --- --- support
	Monster generateNecromancer(const Generate &generate)
	{
		Monster mr = generateOutlawBase(generate);
		mr.updateName("Necromancer");
		mr.icon = "necromancer";
		mr.faction = "outlaw";
		return mr;
	}

	Monster generateOutlaw(const Generate &generate)
	{
		if (generate.magic < 0.5)
		{ // mundane
			if (generate.support < 0.5)
			{ // combat
				if (generate.defensive < 0.5)
				{ // offensive
					if (generate.ranged < 0.5)
					{ // melee
						return generateBarbarian(generate);
					}
					else
					{ // ranged
						return generateAssassin(generate);
					}
				}
				else
				{ // defensive
					return generateBandit(generate);
				}
			}
			else
			{ // support
				if (generate.ranged < 0.5)
				{ // melee
					return generateRogue(generate);
				}
				else
				{ // ranged
					return generateSaboteur(generate);
				}
			}
		}
		else
		{ // magic
			if (generate.support < 0.5)
			{ // combat
				return generateWarlock(generate);
			}
			else
			{ // support
				if (randomChance() > 0.2)
				{ // likely
					if (generate.defensive < 0.5)
					{ // offensive
						return generateOccultist(generate);
					}
					else
					{ // defensive
						return generateShaman(generate);
					}
				}
				else
				{ // unlikely
					return generateNecromancer(generate);
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
		const uint32 attempts = min(generate_.power / 20, 5u); // use the original power
		for (uint32 i = 1; i < attempts; i++) // starting from one -> one attempt has already been generated above
		{
			Monster mr2 = generator(generate);
			if (mr2.goldCost > mr.goldCost)
				std::swap(mr, mr2);
		}

		// loot
		if (randomChance() < 0.4)
			mr.onDeath.push_back(generateItem(Generate(generate_.level, generate_.powerOffset()))); // use the original power
		mr.score = numeric_cast<uint32>(mr.goldCost / empower);

		return mr;
	}
}

Monster generateMonster(const Generate &generate)
{
	return generateMonsterImpl(generate, isHorrorFloor(generate.level) > 0.5 ? &generateHorror : &generateOutlaw);
}

namespace
{
	bool hasSummonMinionSkill(const Monster &mr)
	{
		const auto &isSummoning = [](const std::string &s) -> bool { return s.find("Minion") != s.npos; };
		for (const Item &it : mr.equippedItems)
		{
			for (const Skill &sk : it.skills)
			{
				for (const auto &str : sk.casterFlags)
					if (isSummoning(str))
						return true;
				for (const auto &str : sk.targetFlags)
					if (isSummoning(str))
						return true;
			}
		}
		return false;
	}
}

Monster generateMinion(const Generate &generate_)
{
	CAGE_ASSERT(generate_.valid());
	CAGE_ASSERT(generate_.slot == SlotEnum::None);

	Generate g = generate_;
	g.magic = 0; // minion cannot be magic -> cannot summon other minions

	// no empower
	// no rerolls
	// no loot
	// no score

	Monster mr = isHorrorFloor(g.level) > 0.5 ? generateHorror(g) : generateOutlaw(g);
	mr.faction = "inherited";
	CAGE_ASSERT(!hasSummonMinionSkill(mr));

	return mr;
}

Monster generateChest(const Generate &generate)
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
		mr.onDeath.push_back(generateItem(Generate(generate.level, generate.powerOffset())));

	// trap
	if (randomChance() < 0.05)
		mr.onDeath.push_back(generateMonster(Generate(generate.level, generate.powerOffset()))); // empowered monster

	return mr;
}

namespace
{
	void addDeadBody(Monster &mr)
	{
		mr.onDeath.push_back(std::string() + "{\"class\":\"decoration\",\"type\":\"deadBody\",\"name\":\"" + mr.name.c_str() + "\"}");
	}
}

Monster generateFloorBoss(uint32 level)
{
	const auto &levelSwitch = [level](uint32 floor) -> Real
	{
		floor = bossIndexToLevel(levelToBossIndex(floor) + 1); // round up to actual boss
		CAGE_ASSERT(isLevelBoss(floor));
		if (level < floor)
			return 0;
		if (level == floor)
			return 1;
		return randomChance();
	};

	Generate g = Generate(level, level / 5);
	g.magic = levelSwitch(LevelMagic);
	g.ranged = levelSwitch(LevelRanged);
	g.defensive = randomChance();
	g.support = levelSwitch(LevelSupport);

	Monster mr = generateMonsterImpl(g, &generateOutlaw);
	mr.updateName((Stringizer() + "Guardian of " + level + "th floor").value.c_str());
	mr.icon = "guardian";
	mr.algorithm = "guardian";
	mr.faction = "monster";

	addDeadBody(mr);
	return mr;
}

Monster generateButcher(uint32 level)
{
	Generate g = Generate(level, level / 3);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 0;
	g.support = 0;

	Monster mr = generateMonsterImpl(g, &generateBarbarian);
	mr.updateName("Butcher");
	mr.icon = "butcher";
	mr.algorithm = "butcher";
	mr.faction = "monster";

	// todo add some skills

	addDeadBody(mr);
	return mr;
}

Monster generateWitch(uint32 level)
{
	Generate g = Generate(level, level / 10);
	g.magic = 1;
	g.ranged = 1;
	g.defensive = 0;
	g.support = randomChance();

	Monster mr = generateMonsterImpl(g, g.support < 0.5 ? &generateWarlock : &generateOccultist);
	mr.updateName("Witch");
	mr.icon = "witch";
	mr.algorithm = "witch";
	mr.faction = "monster";

	// todo add broomstick skill

	addDeadBody(mr);
	return mr;
}

Monster generateTemplar(uint32 level)
{
	Generate g = Generate(level, level / 10);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 1;
	g.support = randomChance();

	Monster mr = generateMonsterImpl(g, &generateOutlaw);
	mr.updateName("Templar Knight");
	mr.icon = "templar";
	mr.algorithm = "templar";
	mr.faction = "templar";
	mr.score = 0;

	addDeadBody(mr);
	return mr;
}

namespace
{
	Monster generateZerglingImpl(const Generate &generate)
	{
		Monster mr(generate);
		mr.updateName("Zergling");
		mr.icon = "zergling";
		mr.algorithm = "zergling";
		mr.faction = "monster";

		mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
		mr.attributes[AttributeEnum::Stamina] = generate.power + randomRange(30, 50);

		Item it(generate);
		it.slot = SlotEnum::MainHand;
		it.name = "Claws";

		{
			Skill sk(generate);
			sk.name = "Scratch";
			sk.radius[AttributeEnum::Scalar] = 1;
			sk.damageAmount[AttributeEnum::Scalar] = generate.power * 0.1 + 5;
			sk.damageType = DamageTypeEnum::Slash;
			sk.cost[AttributeEnum::Stamina] = 10;
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Jump";
			sk.radius[AttributeEnum::Scalar] = 2;
			sk.cost[AttributeEnum::Stamina] = 5;
			sk.casterFlags.push_back(SkillMoves);
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Regenerate";
			sk.casterAttributes[AttributeEnum::Life][AttributeEnum::Scalar] = generate.power * 0.02 + 1;
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = generate.power * 0.02 + 1;
			sk.casterFlags.push_back(SkillPassive);
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		mr.addPower(it, 1);
		mr.equippedItems.push_back(std::move(it));
		return mr;
	}
}

Monster generateZergling(uint32 level)
{
	Generate g = Generate(level, 0);
	g.magic = 0;
	g.ranged = 0;
	g.defensive = 0;
	g.support = 0;
	return generateMonsterImpl(g, &generateZerglingImpl);
}

namespace
{
	Monster generateHydraImpl(const Generate &generate, uint32 nested)
	{
		Monster mr(generate);
		mr.updateName("Hydra");
		mr.icon = "hydra";
		mr.algorithm = "hydra";
		mr.faction = "monster";

		mr.attributes[AttributeEnum::Life] = generate.power + randomRange(30, 50);
		mr.attributes[AttributeEnum::Stamina] = generate.power + randomRange(30, 50);

		Item it(generate);
		it.slot = SlotEnum::Head;
		it.name = "Head";

		{
			Skill sk(generate);
			if (generate.ranged > 0.5)
			{
				sk.name = "Smash";
				sk.damageAmount[AttributeEnum::Scalar] = generate.power * 0.10 + 5;
				sk.radius[AttributeEnum::Scalar] = 3;
				sk.damageType = DamageTypeEnum::Slash;
			}
			else
			{
				sk.name = "Bite";
				sk.radius[AttributeEnum::Scalar] = 1;
				sk.damageAmount[AttributeEnum::Scalar] = generate.power * 0.15 + 5;
				sk.damageType = DamageTypeEnum::Piercing;
			}
			sk.cost[AttributeEnum::Stamina] = 10;
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Jump";
			sk.radius[AttributeEnum::Scalar] = 2;
			sk.cost[AttributeEnum::Stamina] = 5;
			sk.casterFlags.push_back(SkillMoves);
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		{
			Skill sk(generate);
			sk.name = "Regenerate";
			sk.casterAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = generate.power * 0.02 + 1;
			sk.casterFlags.push_back(SkillPassive);
			it.addPower(sk, 1);
			it.skills.push_back(std::move(sk));
		}

		mr.addPower(it, 1);
		mr.equippedItems.push_back(std::move(it));

		if (nested < 5)
		{
			Generate g = generate;
			g.power = 9 * g.power / 10;
			g.ranged = randomChance();
			mr.onDeath.push_back(generateHydraImpl(g, nested + 1));
			mr.onDeath.push_back(generateHydraImpl(g, nested + 1));
		}

		return mr;
	}

	Monster generateHydraImplEntry(const Generate &generate)
	{
		return generateHydraImpl(generate, 0);
	}
}

Monster generateHydra(uint32 level)
{
	Generate g = Generate(level, 0);
	g.magic = 0;
	g.ranged = randomChance();
	g.defensive = 0;
	g.support = 0;
	return generateMonsterImpl(g, &generateHydraImplEntry);
}
