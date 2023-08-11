#include "dnt.h"

namespace
{
	// todo remove
	Monster generateRandomMonster(const Generate &generate)
	{
		Monster mr(generate);

		const uint32 level = generate.power;

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

		if (randomChance() < 0.1)
			mr.onDeath.push_back(generateMinion(generate));
		if (randomChance() < 0.1)
			mr.onDeath.push_back(generateSkill(Generate(generate.level, generate.powerOffset(), SlotEnum::MainHand)));

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
		// todo rewrite
		return generateRandomMonster(generate);
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

Monster generateMinion(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	CAGE_ASSERT(generate.slot == SlotEnum::None);
	// no empower
	// no rerolls
	// no loot
	// no score
	Monster mr = isHorrorFloor(generate.level) > 0.5 ? generateHorror(generate) : generateOutlaw(generate);
	mr.faction = "inherited";
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
