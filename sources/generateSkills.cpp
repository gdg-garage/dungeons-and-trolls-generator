#include "dnt.h"

Skill::Skill(const Generate &generate) : Thing(generate){};

Skill skillStomp(const Generate &generate)
{
	Skill sk(generate);
	sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, "Expansive"));
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Forceful")) * 0.3;
	sk.damageType = DamageTypeEnum::Slash;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 30);
	sk.target.flags.knockback = true;
	sk.addPower(1, 1);
	sk.updateName("Stomp");
	return sk;
}

namespace
{
	Skill generatePunch(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "High")) * 0.7;
		sk.damageType = DamageTypeEnum::Pierce;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 6);
		sk.updateName("Punch");
		return sk;
	}

	Skill generateKick(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Low")) * 0.8;
		sk.damageType = DamageTypeEnum::Slash;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 8);
		sk.updateName("Kick");
		return sk;
	}

	Skill generateChainHook(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Stretching")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(0.7, "Surgical")) * 0.5;
		sk.damageType = DamageTypeEnum::Pierce;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 35);
		sk.target.flags.movement = true;
		sk.target.flags.stun = true;
		sk.addPower(1, 1.5);
		sk.updateName("Chain Hook");
		return sk;
	}

	Skill generateStomp(const Generate &generate)
	{
		return skillStomp(generate);
	}

	Skill generateBearTrap(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = interpolate(4.0, 6.0, sk.addPower(0.5));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, "Lasting"));
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Sprung")) * 0.6;
		sk.damageType = DamageTypeEnum::Pierce;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
		sk.target.flags.groundEffect = true;
		sk.updateName("Bear Trap");
		return sk;
	}

	Skill generatePoisonVial(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(0.8, "Splashing"));
		sk.duration[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, "Sticky"));
		sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Potent")) * 0.2;
		sk.damageType = DamageTypeEnum::Poison;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 20);
		sk.target.flags.groundEffect = true;
		sk.updateName("Poison Vial");
		return sk;
	}

	Skill generateSmokeLeap(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Light")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(1.0, 4.0, sk.addPower(0.7, "Dense"));
		sk.damageAmount[AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.7, "Fiery")) * 0.5;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 20);
		sk.caster.flags.movement = true;
		sk.caster.flags.groundEffect = true;
		sk.addPower(1, 1);
		sk.updateName("Smoke Leap");
		return sk;
	}

	Skill generateConstructBallista(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Lasting"));
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 55);
		{
			Generate g = generate;
			g.magic = 0;
			g.ranged = 1;
			g.defensive = 0;
			g.support = 0;
			Monster mr(g);
			mr.icon = "totem";
			mr.algorithm = "ballista";
			mr.faction = "inherited";
			Item it(g);
			it.slot = SlotEnum::MainHand;
			it.name = "Bow";
			it.icon = "bow";
			{
				Skill sk(g);
				sk.name = "Attack";
				sk.targetType = SkillTargetEnum::Character;
				sk.range[AttributeEnum::Scalar] = randomRange(4, 8);
				sk.damageAmount[AttributeEnum::Scalar] = generate.power * 0.1 + 5;
				sk.damageType = DamageTypeEnum::Pierce;
				it.addOther(sk, 1);
				it.skills.push_back(std::move(sk));
			}
			mr.addOther(it, 1);
			mr.equippedItems.push_back(std::move(it));
			mr.updateName("Ballista");
			sk.addOther(mr, 1);
			sk.target.summons.push_back(std::move(mr));
		}
		sk.name = "Construct Ballista";
		return sk;
	}

	Skill generateWarcry(const Generate &generate)
	{
		Skill sk(generate);
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, "Loud"));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.8, "Motivating"));
		sk.caster.attributes[AttributeEnum::Strength][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.8, "Strengthening")) * 0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 25);
		sk.target.flags.stun = true;
		sk.addPower(1, 1.5);
		sk.updateName("Warcry");
		return sk;
	}

	Skill generateFocus(const Generate &generate)
	{
		Skill sk(generate);
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, "Prolonged"));
		sk.caster.attributes[AttributeEnum::Intelligence][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Deep")) * 0.5;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
		sk.updateName("Focus");
		return sk;
	}

	Skill generatePatchWounds(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.target.attributes[AttributeEnum::Life][AttributeEnum::Scalar] = 5;
		sk.target.attributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Doctorly")) * 0.2;
		sk.target.attributes[AttributeEnum::Life][AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Carefully")) * 0.2;
		sk.caster.flags.allowTargetSelf = true;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
		sk.updateName("Patch Wounds");
		return sk;
	}

	Skill generateBerserk(const Generate &generate)
	{
		Skill sk(generate);
		sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Raging")) * 2;
		sk.cost[AttributeEnum::Life] = makeCost(sk, 15);
		sk.updateName("Berserk");
		return sk;
	}

	Skill generateIntimidate(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, "Enduring"));
		sk.target.attributes[AttributeEnum::Intelligence][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Frightening")) * -0.5;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
		sk.updateName("Intimidate");
		return sk;
	}
}

Skill skillFireball(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.8, "Missile")) * 0.1;
	sk.range[AttributeEnum::Scalar] = 5;
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Blazing"));
	sk.damageType = DamageTypeEnum::Fire;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
	sk.updateName("Fireball");
	return sk;
}

Skill skillHeal(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.1;
	sk.range[AttributeEnum::Scalar] = 4;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Concentrated")) * 0.5;
	sk.caster.flags.allowTargetSelf = true;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Heal");
	return sk;
}

namespace
{
	Skill generateScorch(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.damageAmount[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Burning")) * 0.6;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 7);
		sk.updateName("Scorch");
		return sk;
	}

	Skill generateShockNova(const Generate &generate)
	{
		Skill sk(generate);
		sk.radius[AttributeEnum::Scalar] = interpolate(3.0, 5.0, sk.addPower(1, "Vast"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Energetic")) * 0.3;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 25);
		sk.updateName("Shock Nova");
		return sk;
	}

	Skill generateFireball(const Generate &generate)
	{
		return skillFireball(generate);
	}

	Skill generateThunderboltLeap(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.8, "Bouncing")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(0.9, "Frying")) * 0.5;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 40);
		sk.caster.flags.movement = true;
		sk.target.flags.stun = true;
		sk.addPower(1, 1.5);
		sk.updateName("Thunderbolt Leap");
		return sk;
	}

	Skill generateMeteor(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.6, "Distant")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.radius[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(0.8, "Shower", AffixEnum::Suffix)) * 0.05;
		sk.radius[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, "Cosmic"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Vigorous")) * 0.15;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 30);
		sk.target.flags.groundEffect = true;
		sk.updateName("Meteor");
		return sk;
	}

	Skill generateManaDrain(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.target.attributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Sucking")) * -0.35;
		sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Generous")) * 0.35;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
		sk.updateName("Mana Drain");
		return sk;
	}

	Skill generateTeleport(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.2;
		sk.range[AttributeEnum::Scalar] = 5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 40);
		sk.caster.flags.movement = true;
		sk.caster.flags.requiresLineOfSight = false;
		sk.addPower(1, 1);
		sk.updateName("Teleport");
		return sk;
	}

	Skill generateBodySwap(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.15;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 60);
		sk.caster.flags.movement = true;
		sk.target.flags.movement = true;
		sk.caster.flags.requiresLineOfSight = false;
		sk.addPower(1, 1);
		sk.updateName("Body Swap");
		return sk;
	}

	Skill generateHeal(const Generate &generate)
	{
		return skillHeal(generate);
	}

	Skill generateBloodMagic(const Generate &generate)
	{
		Skill sk(generate);
		sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Puissant")) * 3;
		sk.cost[AttributeEnum::Life] = makeCost(sk, 25);
		sk.updateName("Blood Magic");
		return sk;
	}

	Skill generateSummonMinion(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(10.0, 15.0, sk.addPower(1, "Lasting"));
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 65);
		{
			Monster mr = generateMinion(Generate(generate.level, generate.powerOffset()));
			sk.addOther(mr, 1);
			sk.name = std::string() + "Summon Minion: " + mr.name;
			sk.target.summons.push_back(std::move(mr));
		}
		return sk;
	}

	Skill generateElectrocute(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.duration[AttributeEnum::Scalar] = interpolate(3.0, 6.0, sk.addPower(0.8, "Constricting"));
		sk.target.attributes[AttributeEnum::Strength][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Choking")) * -0.2;
		sk.target.attributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Conductive")) * -0.15;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
		sk.updateName("Electrocute");
		return sk;
	}

	Skill generateIceShield(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, "Lasting"));
		sk.target.attributes[AttributeEnum::PierceResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Impenetrable")) * 0.2;
		sk.target.attributes[AttributeEnum::FireResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Fireproof")) * 0.2;
		sk.caster.flags.allowTargetSelf = true;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
		sk.updateName("Ice Shield");
		return sk;
	}

	Skill generateFleshOfSteel(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, "Lasting"));
		sk.target.attributes[AttributeEnum::SlashResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Slipping")) * 0.2;
		sk.target.attributes[AttributeEnum::PoisonResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Resilient")) * 0.2;
		sk.caster.flags.allowTargetSelf = true;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
		sk.updateName("Flesh Of Steel");
		return sk;
	}

	Skill generateFaradayCage(const Generate &generate)
	{
		Skill sk(generate);
		sk.targetType = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, "Lasting"));
		sk.target.attributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Grounded")) * 0.2;
		sk.caster.flags.allowTargetSelf = true;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
		sk.updateName("Faraday Cage");
		return sk;
	}
}

namespace
{
	Skill generateLaugh(const Generate &generate)
	{
		Skill sk(generate);
		sk.updateName("Laugh");
		return sk;
	}
}

Skill generateSkill(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	// may or may not have a slot

	Candidates<Skill (*)(const Generate &generate)> candidates(generate);

	candidates.add(0, 0, 0, 0, SlotEnum::MainHand, { LevelPierce }, generatePunch);
	candidates.add(0, 0, 0, 0, SlotEnum::Legs, { LevelSlash }, generateKick);
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

	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { LevelFire }, generateScorch);
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
