#include "dnt.h"

namespace
{
	Skill generateChainHook(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Stretching") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 0.7, "Surgical") * 0.5;
		sk.damageType = DamageTypeEnum::Piercing;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 35);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(SkillMoves);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(SkillStun);
		sk.updateName("Chain Hook");
		return sk;
	}

	Skill generateStomp(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, AffixEnum::Prefix, "Expansive"));
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Forceful") * 0.3;
		sk.damageType = DamageTypeEnum::Slash;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 30);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(SkillKnockback);
		sk.updateName("Stomp");
		return sk;
	}

	Skill generateBearTrap(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = interpolate(4.0, 6.0, sk.addPower(0.5));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Durable"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Sprung"));
		sk.damageType = DamageTypeEnum::Piercing;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 15);
		sk.targetFlags.push_back(SkillGroundEffect);
		sk.updateName("Bear Trap");
		return sk;
	}

	Skill generatePoisonVial(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 0.6, "Throwing") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(0.8, AffixEnum::Prefix, "Splashing"));
		sk.duration[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Sticky"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Potent"));
		sk.damageType = DamageTypeEnum::Poison;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 20);
		sk.targetFlags.push_back(SkillGroundEffect);
		sk.updateName("Poison Vial");
		return sk;
	}

	Skill generateSmokeLeap(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Light") * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(1.0, 4.0, sk.addPower(0.7, AffixEnum::Prefix, "Dense"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Fiery"));
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 20);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillMoves);
		sk.casterFlags.push_back(SkillGroundEffect);
		sk.updateName("Smoke Leap");
		return sk;
	}

	Skill generateConstructBallista(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(10.0, 15.0, sk.addPower(1));
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 55);
		{
			Monster mr(generate);
			mr.icon = "ballista";
			mr.algorithm = "ballista";
			mr.faction = "inherited";
			// todo
			mr.updateName("Ballista");

			std::string json;
			json += "{\n";
			json += "\"class\":\"summon\",\n";
			json += "\"data\":" + exportMonster(mr) + "\n";
			json += "}";

			sk.addPower(mr, 1);
			sk.targetFlags.push_back(std::move(json));
		}
		sk.name = "Construct Ballista";
		return sk;
	}

	Skill generateWarcry(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, AffixEnum::Prefix, "Loud"));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.8, AffixEnum::Prefix, "Motivating"));
		sk.casterAttributes[AttributeEnum::Strength][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 0.8, "Strengthening") * 0.1;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 25);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(SkillStun);
		sk.updateName("Warcry");
		return sk;
	}

	Skill generateFocus(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Prolonged"));
		sk.casterAttributes[AttributeEnum::Intelligence][AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Deep") * 0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.updateName("Focus");
		return sk;
	}

	Skill generatePatchWounds(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Doctorly") * 0.2;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 1, "Carefully") * 0.2;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.updateName("Patch Wounds");
		return sk;
	}

	Skill generateBerserk(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.targetAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = makeAttrFactor(sk, generate, 1, "Raging") * 25;
		sk.cost[AttributeEnum::Life] = makeCost(sk, generate, 15);
		sk.updateName("Berserk");
		return sk;
	}

	Skill generateIntimidate(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Enduring"));
		sk.targetAttributes[AttributeEnum::Intelligence][AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Frightening") * -0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.updateName("Intimidate");
		return sk;
	}
}

namespace
{
	Skill generateShockNova(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(3.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Vast"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Energetic") * 0.3;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 25);
		sk.updateName("Shock Nova");
		return sk;
	}

	Skill generateFireball(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.8, "Missile") * 0.1;
		sk.range[AttributeEnum::Scalar] = 5;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Blazing") * 1;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.updateName("Fireball");
		return sk;
	}

	Skill generateThunderboltLeap(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.8, "Bouncing") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 0.9, "Frying") * 0.5;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 40);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillMoves);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(SkillStun);
		sk.updateName("Thunderbolt Leap");
		return sk;
	}

	Skill generateMeteor(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.6, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.radius[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 0.8, "Shower", AffixEnum::Suffix) * 0.1;
		sk.radius[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Cosmic"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Vigorous") * 0.25;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 30);
		sk.targetFlags.push_back(SkillGroundEffect);
		sk.updateName("Meteor");
		return sk;
	}

	Skill generateManaDrain(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.targetAttributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Sucking") * -0.5;
		sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Generous") * 0.5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.updateName("Mana Drain");
		return sk;
	}

	Skill generateTeleport(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Distant") * 0.2;
		sk.range[AttributeEnum::Scalar] = 5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 40);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillMoves);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillNoLineOfSight);
		sk.updateName("Teleport");
		return sk;
	}

	Skill generateBodySwap(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Distant") * 0.2;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 60);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillMoves);
		sk.targetFlags.push_back(SkillMoves);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(SkillNoLineOfSight);
		sk.updateName("Body Swap");
		return sk;
	}

	Skill generateHeal(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Concentrated") * 0.5;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.updateName("Heal");
		return sk;
	}

	Skill generateBloodMagic(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.targetAttributes[AttributeEnum::Mana][AttributeEnum::Scalar] = makeAttrFactor(sk, generate, 1, "Puissant") * 35;
		sk.cost[AttributeEnum::Life] = makeCost(sk, generate, 20);
		sk.updateName("Blood Magic");
		return sk;
	}

	Skill generateSummonMinion(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(10.0, 15.0, sk.addPower(1));
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 65);
		{
			Monster mr = generateMinion(Generate(generate.level, generate.powerOffset()));

			std::string json;
			json += "{\n";
			json += "\"class\":\"summon\",\n";
			json += "\"data\":" + exportMonster(mr) + "\n";
			json += "}";

			sk.addPower(mr, 1);
			sk.targetFlags.push_back(std::move(json));

			sk.name = std::string() + "Summon Minion: " + mr.name;
		}
		return sk;
	}

	Skill generateElectrocute(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.duration[AttributeEnum::Scalar] = interpolate(3.0, 6.0, sk.addPower(0.8, AffixEnum::Prefix, "Constricting"));
		sk.targetAttributes[AttributeEnum::Strength][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Choking") * -0.2;
		sk.targetAttributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Conductive") * -0.15;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.updateName("Electrocute");
		return sk;
	}

	Skill generateIceShield(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::PiercingArmor][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Impenetrable") * 0.2;
		sk.targetAttributes[AttributeEnum::FireResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Fireproof") * 0.2;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.updateName("Ice Shield");
		return sk;
	}

	Skill generateFleshOfSteel(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::SlashArmor][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Slipping") * 0.2;
		sk.targetAttributes[AttributeEnum::PoisonResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Resilient") * 0.2;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.updateName("Flesh Of Steel");
		return sk;
	}

	Skill generateFaradayCage(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Grounded") * 0.2;
		sk.casterFlags.push_back(SkillAllowSelf);
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.updateName("Faraday Cage");
		return sk;
	}
}

namespace
{
	Skill generateLaugh(const Generate &generate)
	{
		Skill sk(generate);
		sk.target = SkillTargetEnum::None;
		sk.updateName("Laugh");
		return sk;
	}
}

Skill generateSkill(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	// may or may not have a slot

	Candidates<Skill (*)(const Generate &generate)> candidates(generate);

	candidates.add(0, 1, 0, 0, SlotEnum::MainHand, { LevelPierce, LevelStun }, generateChainHook);
	candidates.add(0, 0, 1, 0, SlotEnum::Legs, { LevelSlash, LevelAoe, LevelKnockback }, generateStomp);
	candidates.add(0, 1, 1, 0, SlotEnum::MainHand, { LevelPierce, LevelDuration, LevelGroundEffect }, generateBearTrap);
	candidates.add(0, 1, 1, 0, SlotEnum::Legs, { LevelFire, LevelGroundEffect }, generateSmokeLeap);
	candidates.add(0, 1, 0, 0, SlotEnum::Body, { LevelPoison, LevelAoe, LevelDuration, LevelGroundEffect }, generatePoisonVial);
	candidates.add(0, 1, 0, 1, SlotEnum::Body, { LevelDuration, LevelSummoning }, generateConstructBallista);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelAoe, LevelDuration, LevelStun }, generateWarcry);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelDuration }, generateFocus);
	candidates.add(0, 0, 1, 1, SlotEnum::OffHand, { Nothing }, generatePatchWounds);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { Nothing }, generateBerserk);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelDuration }, generateIntimidate);

	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { LevelElectric, LevelAoe }, generateShockNova);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { LevelFire }, generateFireball);
	candidates.add(1, 1, 0, 0, SlotEnum::Legs, { LevelElectric, LevelStun }, generateThunderboltLeap);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { LevelFire, LevelAoe, LevelDuration, LevelGroundEffect }, generateMeteor);
	candidates.add(1, 1, 1, 0, SlotEnum::Head, { LevelDuration }, generateManaDrain);
	candidates.add(1, 1, 1, 1, SlotEnum::Body, { Nothing }, generateTeleport);
	candidates.add(1, 1, 0, 1, SlotEnum::Body, { Nothing }, generateBodySwap);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelDuration }, generateHeal);
	candidates.add(1, H, 0, 1, SlotEnum::Head, { Nothing }, generateBloodMagic);
	candidates.add(1, 0, 0, 1, SlotEnum::Body, { LevelDuration, LevelSummoning }, generateSummonMinion);
	candidates.add(1, 1, 0, 1, SlotEnum::MainHand, { LevelElectric, LevelDuration, LevelStun }, generateElectrocute);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelPierce, LevelFire, LevelDuration }, generateIceShield);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelSlash, LevelPoison, LevelDuration }, generateFleshOfSteel);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelElectric, LevelDuration }, generateFaradayCage);

	candidates.fallback(generateLaugh);
	return candidates.pick()(generate);
}
