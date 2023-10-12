#include "dnt.h"

Monster monsterMinion(const Generate &generate);
Monster monsterBallista(const Generate &generate);
Monster monsterLandMine(const Generate &generate);

Skill::Skill(const Generate &generate) : Thing(generate){};

// section mundane skills
namespace
{}

Skill skillPunch(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "High"));
	sk.damageType = DamageTypeEnum::Pierce;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 8);
	sk.updateName("Punch");
	return sk;
}

Skill skillKick(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Low"));
	sk.damageType = DamageTypeEnum::Slash;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 8);
	sk.updateName("Kick");
	return sk;
}

Skill skillChainHook(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Stretching")) * 0.05;
	sk.range[AttributeEnum::Constant] = 3;
	sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(0.7, "Surgical")) * 0.5;
	sk.damageType = DamageTypeEnum::Pierce;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 35);
	sk.target.flags.movement = true;
	sk.target.flags.stun = true;
	sk.addPower(1, 1.5);
	sk.updateName("Chain Hook");
	return sk;
}

Skill skillStomp(const Generate &generate)
{
	Skill sk(generate);
	sk.radius[AttributeEnum::Constant] = interpolate(2.0, 4.0, sk.addPower(1, "Expansive"));
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Forceful")) * 0.3;
	sk.damageType = DamageTypeEnum::Slash;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 30);
	sk.target.flags.knockback = true;
	sk.addPower(1, 1);
	sk.updateName("Stomp");
	return sk;
}

Skill skillBearTrap(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Constant] = interpolate(4.0, 6.0, sk.addPower(0.5));
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(1, "Lasting"));
	sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Sprung")) * 0.6;
	sk.damageType = DamageTypeEnum::Pierce;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
	sk.target.flags.groundEffect = true;
	sk.updateName("Bear Trap");
	return sk;
}

Skill skillPoisonVial(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Constant] = 4;
	sk.radius[AttributeEnum::Constant] = interpolate(2.0, 4.0, sk.addPower(0.8, "Splashing"));
	sk.duration[AttributeEnum::Constant] = interpolate(5.0, 10.0, sk.addPower(1, "Sticky"));
	sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Potent")) * 0.2;
	sk.damageType = DamageTypeEnum::Poison;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 20);
	sk.target.flags.groundEffect = true;
	sk.updateName("Poison Vial");
	return sk;
}

Skill skillSmokeLeap(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Light")) * 0.1;
	sk.range[AttributeEnum::Constant] = 2;
	sk.duration[AttributeEnum::Constant] = interpolate(1.0, 4.0, sk.addPower(0.7, "Dense"));
	sk.damageAmount[AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.7, "Fiery")) * 0.5;
	sk.damageType = DamageTypeEnum::Fire;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 20);
	sk.caster.flags.movement = true;
	sk.caster.flags.groundEffect = true;
	sk.addPower(1, 1);
	sk.updateName("Smoke Leap");
	return sk;
}

Skill skillConstructBallista(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Constant] = 2;
	sk.duration[AttributeEnum::Constant] = 5;
	sk.duration[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Lasting")) * 0.3;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 55);
	{
		Generate g = generate;
		g.magic = 0;
		g.ranged = 1;
		g.defensive = 0;
		g.support = 0;
		Monster mr = monsterBallista(g);
		sk.addOther(mr, 1);
		sk.target.summons.push_back(std::move(mr));
	}
	sk.name = "Construct Ballista";
	return sk;
}

Skill skillWarcry(const Generate &generate)
{
	Skill sk(generate);
	sk.radius[AttributeEnum::Constant] = interpolate(2.0, 4.0, sk.addPower(1, "Loud"));
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(0.8, "Motivating"));
	sk.caster.attributes[AttributeEnum::Strength][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.8, "Strengthening")) * 0.2;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 25);
	sk.target.flags.stun = true;
	sk.addPower(1, 1.5);
	sk.updateName("Warcry");
	return sk;
}

Skill skillFocus(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(1, "Prolonged"));
	sk.caster.attributes[AttributeEnum::Intelligence][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Deep")) * 0.5;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
	sk.updateName("Focus");
	return sk;
}

Skill skillPatchWounds(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 1;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Constant] = 10;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Doctorly")) * 0.2;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Carefully")) * 0.2;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
	sk.flags.requiresOutOfCombat = true;
	sk.updateName("Patch Wounds");
	return sk;
}

Skill skillBerserk(const Generate &generate)
{
	Skill sk(generate);
	sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Raging")) * 2;
	sk.cost[AttributeEnum::Life] = makeCost(sk, 15);
	sk.updateName("Berserk");
	return sk;
}

Skill skillIntimidate(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(1, "Enduring"));
	sk.target.attributes[AttributeEnum::Intelligence][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Frightening")) * -0.5;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 10);
	sk.updateName("Intimidate");
	return sk;
}

Skill skillLiquor(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 5.0, sk.addPower(0.8, "Lasting"));
	sk.caster.attributes[AttributeEnum::Intelligence][AttributeEnum::Intelligence] = interpolate(-0.7, -0.3, sk.addPower(0.5));
	sk.caster.attributes[AttributeEnum::Dexterity][AttributeEnum::Dexterity] = interpolate(-0.7, -0.3, sk.addPower(0.5));
	sk.caster.attributes[AttributeEnum::Strength][AttributeEnum::Strength] = interpolate(0.3, 0.7, sk.addPower(1, "Strong"));
	sk.caster.attributes[AttributeEnum::Constitution][AttributeEnum::Constitution] = interpolate(0.3, 0.7, sk.addPower(1, "Sparkling"));
	sk.caster.attributes[AttributeEnum::Willpower][AttributeEnum::Willpower] = interpolate(0.3, 0.7, sk.addPower(1, "Encouraging"));
	sk.caster.attributes[AttributeEnum::SlashResist][AttributeEnum::Constant] = -10;
	sk.caster.attributes[AttributeEnum::PierceResist][AttributeEnum::Constant] = 5;
	sk.caster.attributes[AttributeEnum::FireResist][AttributeEnum::Constant] = -10;
	sk.caster.attributes[AttributeEnum::PoisonResist][AttributeEnum::Constant] = 5;
	sk.caster.flags.stun = true;
	sk.updateName("Liquor");
	return sk;
}

Skill skillCharge(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Dashing")) * 0.08;
	sk.range[AttributeEnum::Constant] = 3;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 12);
	sk.caster.flags.movement = true;
	sk.updateName("Charge");
	return sk;
}

Skill skillWaterSplash(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Constant] = 4;
	sk.radius[AttributeEnum::Constant] = interpolate(1.0, 4.0, sk.addPower(0.9, "Wide"));
	sk.duration[AttributeEnum::Constant] = interpolate(5.0, 10.0, sk.addPower(0.8, "Deep"));
	sk.target.attributes[AttributeEnum::FireResist][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Wet")) * 0.2;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
	sk.target.flags.groundEffect = true;
	sk.updateName("Water Spray");
	return sk;
}

Skill skillLandMine(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Constant] = 60;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 45);
	{
		Generate g = generate;
		g.magic = 0;
		g.ranged = 0;
		g.defensive = 0;
		g.support = 0;
		Monster mr = monsterLandMine(g);
		sk.addOther(mr, 1);
		sk.target.summons.push_back(std::move(mr));
	}
	sk.name = "Land Mine";
	return sk;
}

Skill skillSandCastle(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(0.9, "Reinforced")) * 0.1;
	sk.duration[AttributeEnum::Constant] = 3;
	sk.caster.attributes[AttributeEnum::SlashResist][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Slashproof")) * 0.2;
	sk.caster.attributes[AttributeEnum::PierceResist][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Pierceproof")) * 0.2;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 20);
	sk.caster.flags.groundEffect = true;
	sk.updateName("Sand Castle");
	return sk;
}

Skill skillTraining(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.9, "Endurance")) * 0.1;
	sk.duration[AttributeEnum::Constant] = 3;
	sk.caster.attributes[AttributeEnum::Strength][AttributeEnum::Dexterity] = makeAttrFactor(generate.power, sk.addPower(1, "Strength")) * 0.35;
	sk.caster.attributes[AttributeEnum::Dexterity][AttributeEnum::Strength] = makeAttrFactor(generate.power, sk.addPower(1, "Balance")) * 0.35;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 25);
	sk.updateName("Training");
	return sk;
}

// section magical skills
namespace
{}

Skill skillScorch(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.damageAmount[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Burning")) * 0.8;
	sk.damageType = DamageTypeEnum::Fire;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 7);
	sk.updateName("Scorch");
	return sk;
}

Skill skillShockNova(const Generate &generate)
{
	Skill sk(generate);
	sk.radius[AttributeEnum::Constant] = interpolate(3.0, 5.0, sk.addPower(1, "Vast"));
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Energetic")) * 0.3;
	sk.damageType = DamageTypeEnum::Electric;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 25);
	sk.updateName("Shock Nova");
	return sk;
}

Skill skillFireball(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.8, "Missile")) * 0.1;
	sk.range[AttributeEnum::Constant] = 5;
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Blazing"));
	sk.damageType = DamageTypeEnum::Fire;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
	sk.updateName("Fireball");
	return sk;
}

Skill skillThunderboltLeap(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.8, "Bouncing")) * 0.1;
	sk.range[AttributeEnum::Constant] = 3;
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(0.9, "Frying")) * 0.5;
	sk.damageType = DamageTypeEnum::Electric;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 40);
	sk.caster.flags.movement = true;
	sk.target.flags.stun = true;
	sk.addPower(1, 1.5);
	sk.updateName("Thunderbolt Leap");
	return sk;
}

Skill skillMeteor(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.6, "Distant")) * 0.1;
	sk.range[AttributeEnum::Constant] = 2;
	sk.radius[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(0.8, "Shower", AffixEnum::Suffix)) * 0.05;
	sk.radius[AttributeEnum::Constant] = 3;
	sk.duration[AttributeEnum::Constant] = interpolate(2.0, 5.0, sk.addPower(1, "Cosmic"));
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Vigorous")) * 0.15;
	sk.damageType = DamageTypeEnum::Fire;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 30);
	sk.target.flags.groundEffect = true;
	sk.updateName("Meteor");
	return sk;
}

Skill skillManaDrain(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.target.attributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Sucking")) * -0.35;
	sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Generous")) * 0.35;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
	sk.updateName("Mana Drain");
	return sk;
}

Skill skillTeleport(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.2;
	sk.range[AttributeEnum::Constant] = 5;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 40);
	sk.caster.flags.movement = true;
	sk.flags.requiresLineOfSight = false;
	sk.addPower(1, 1);
	sk.updateName("Teleport");
	return sk;
}

Skill skillBodySwap(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.15;
	sk.range[AttributeEnum::Constant] = 3;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 60);
	sk.caster.flags.movement = true;
	sk.target.flags.movement = true;
	sk.flags.requiresLineOfSight = false;
	sk.addPower(1);
	sk.updateName("Body Swap");
	return sk;
}

Skill skillHeal(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(1, "Distant")) * 0.05;
	sk.range[AttributeEnum::Constant] = 4;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Concentrated")) * 0.5;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Heal");
	return sk;
}

Skill skillBloodMagic(const Generate &generate)
{
	Skill sk(generate);
	sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(1, "Puissant")) * 3;
	sk.cost[AttributeEnum::Life] = makeCost(sk, 25);
	sk.updateName("Blood Magic");
	return sk;
}

Skill skillSummonMinion(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Position;
	sk.range[AttributeEnum::Constant] = 2;
	sk.duration[AttributeEnum::Constant] = interpolate(7.0, 12.0, sk.addPower(1, "Lasting"));
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 65);
	{
		Monster mr = monsterMinion(Generate(generate.level, generate.powerOffset()));
		sk.addOther(mr, 1);
		sk.name = std::string() + "Summon Minion: " + mr.name;
		sk.target.summons.push_back(std::move(mr));
	}
	return sk;
}

Skill skillElectrocute(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
	sk.range[AttributeEnum::Constant] = 4;
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(0.8, "Constricting"));
	sk.target.attributes[AttributeEnum::Strength][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Choking")) * -0.2;
	sk.target.attributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Conductive")) * -0.15;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Electrocute");
	return sk;
}

Skill skillIceShield(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
	sk.range[AttributeEnum::Constant] = 3;
	sk.duration[AttributeEnum::Constant] = interpolate(4.0, 8.0, sk.addPower(0.9, "Lasting"));
	sk.target.attributes[AttributeEnum::PierceResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Impenetrable")) * 0.2;
	sk.target.attributes[AttributeEnum::FireResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Fireproof")) * 0.2;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Ice Shield");
	return sk;
}

Skill skillFleshOfSteel(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
	sk.range[AttributeEnum::Constant] = 3;
	sk.duration[AttributeEnum::Constant] = interpolate(4.0, 8.0, sk.addPower(0.9, "Lasting"));
	sk.target.attributes[AttributeEnum::SlashResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Slipping")) * 0.2;
	sk.target.attributes[AttributeEnum::PoisonResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Resilient")) * 0.2;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Flesh Of Steel");
	return sk;
}

Skill skillFaradayCage(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Distant")) * 0.1;
	sk.range[AttributeEnum::Constant] = 3;
	sk.duration[AttributeEnum::Constant] = interpolate(4.0, 8.0, sk.addPower(0.9, "Lasting"));
	sk.target.attributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Grounded")) * 0.2;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
	sk.updateName("Faraday Cage");
	return sk;
}

Skill skillAlternateSkin(const Generate &generate)
{
	Skill sk(generate);
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 6.0, sk.addPower(0.9, "Lasting"));

	std::vector<AttributeEnum> attrs;
	attrs.reserve(5);
	attrs.push_back(AttributeEnum::SlashResist);
	attrs.push_back(AttributeEnum::PierceResist);
	if (generate.level > LevelFire)
		attrs.push_back(AttributeEnum::FireResist);
	if (generate.level > LevelPoison)
		attrs.push_back(AttributeEnum::PoisonResist);
	if (generate.level > LevelElectric)
		attrs.push_back(AttributeEnum::ElectricResist);
	const uint32 cnt = attrs.size();
	for (uint32 i = 0; i < 5; i++)
		std::swap(attrs[randomRange(0u, cnt)], attrs[randomRange(0u, cnt)]);
	const AttributeEnum a = attrs[0];
	const AttributeEnum b = attrs[1];

	const Real roll = interpolate(0.6, 1.2, sk.addPower(1, "Of Dragon", AffixEnum::Suffix));
	sk.caster.attributes[a][a] = -roll;
	sk.caster.attributes[b][b] = -roll;
	sk.caster.attributes[a][b] = roll;
	sk.caster.attributes[b][a] = roll;

	sk.cost[AttributeEnum::Mana] = makeCost(sk, 15);
	sk.updateName("Alternate Skin");
	return sk;
}

Skill skillDeathCoil(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Long")) * 0.1;
	sk.range[AttributeEnum::Constant] = 2;
	sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Life] = -1;
	sk.target.attributes[AttributeEnum::Life][AttributeEnum::Life] = -interpolate(0.8, 1.2, sk.addPower(1, "Tight"));
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 80);
	sk.addPower(1);
	sk.updateName("Death Coil");
	return sk;
}

Skill skillManaBurn(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 2;
	sk.damageAmount[AttributeEnum::Mana] = makeAttrFactor(generate.power, sk.addPower(1, "Scorching")) * 0.5;
	sk.damageType = DamageTypeEnum::Fire;
	sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Mana] = -1;
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 15);
	sk.updateName("Mana Burn");
	return sk;
}

Skill skillSoulCrucible(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 1;
	sk.duration[AttributeEnum::Constant] = interpolate(3.0, 5.0, sk.addPower(1, "Bonding"));
	sk.caster.attributes[AttributeEnum::Life][AttributeEnum::Life] = -1;
	sk.target.attributes[AttributeEnum::Strength][AttributeEnum::Strength] = interpolate(0.7, 1.0, sk.addPower(0.7, "Strong"));
	sk.target.attributes[AttributeEnum::Dexterity][AttributeEnum::Dexterity] = interpolate(0.7, 1.0, sk.addPower(0.7, "Dexterous"));
	sk.target.attributes[AttributeEnum::Constitution][AttributeEnum::Constitution] = interpolate(0.7, 1.0, sk.addPower(0.7, "Healthy"));
	sk.target.attributes[AttributeEnum::Intelligence][AttributeEnum::Intelligence] = interpolate(0.7, 1.0, sk.addPower(0.7, "Smart"));
	sk.target.attributes[AttributeEnum::Willpower][AttributeEnum::Willpower] = interpolate(0.7, 1.0, sk.addPower(0.7, "Willful"));
	sk.cost[AttributeEnum::Stamina] = makeCost(sk, 70);
	sk.updateName("Soul Crucible");
	return sk;
}

Skill skillDischarge(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.range[AttributeEnum::Constant] = 2;
	sk.damageAmount[AttributeEnum::ElectricResist] = makeAttrFactor(generate.power, sk.addPower(1, "Lightning"));
	sk.damageType = DamageTypeEnum::Electric;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Discharge");
	return sk;
}

Skill skillLavaWalk(const Generate &generate)
{
	Skill sk(generate);
	sk.radius[AttributeEnum::Willpower] = makeAttrFactor(generate.power, sk.addPower(0.7, "Wast")) * 0.1;
	sk.radius[AttributeEnum::Constant] = 2;
	sk.duration[AttributeEnum::Constant] = interpolate(4.0, 8.0, sk.addPower(0.9, "Lasting"));
	sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(generate.power, sk.addPower(1, "Burning"));
	sk.damageType = DamageTypeEnum::Fire;
	sk.caster.attributes[AttributeEnum::FireResist][AttributeEnum::Constitution] = makeAttrFactor(generate.power, sk.addPower(0.9, "Soothing")) * 0.2;
	sk.target.flags.groundEffect = true;
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 20);
	sk.updateName("Lava Walk");
	return sk;
}

Skill skillManaChannel(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.caster.attributes[AttributeEnum::Mana][AttributeEnum::Mana] = -1;
	sk.target.attributes[AttributeEnum::Mana][AttributeEnum::Mana] = interpolate(0.7, 1.0, sk.addPower(1, "Manatight"));
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 10);
	sk.updateName("Mana Channel");
	return sk;
}

Skill skillStaminaChannel(const Generate &generate)
{
	Skill sk(generate);
	sk.targetType = SkillTargetEnum::Character;
	sk.caster.attributes[AttributeEnum::Stamina][AttributeEnum::Stamina] = -1;
	sk.target.attributes[AttributeEnum::Stamina][AttributeEnum::Stamina] = interpolate(0.7, 1.0, sk.addPower(1, "Sweattight"));
	sk.cost[AttributeEnum::Mana] = makeCost(sk, 10);
	sk.updateName("Stamina Channel");
	return sk;
}

// section other
namespace
{}

Skill skillLaugh(const Generate &generate)
{
	Skill sk(generate);
	sk.updateName("Laugh");
	return sk;
}

Skill skillGeneric(const Generate &generate)
{
	CAGE_ASSERT(generate.valid());
	// may or may not have a slot

	Candidates<Skill (*)(const Generate &generate)> candidates(generate);
	candidates.slotMismatchPenalty = 0.5;
	candidates.randomness = 2;

	candidates.add(0, 1, 0, 0, SlotEnum::OffHand, { LevelPierce }, skillPunch);
	candidates.add(0, 0, 0, 0, SlotEnum::Legs, { LevelSlash }, skillKick);
	candidates.add(0, 1, 0, 0, SlotEnum::MainHand, { LevelPierce, LevelStun }, skillChainHook);
	candidates.add(0, 0, 1, 0, SlotEnum::Legs, { LevelSlash, LevelAoe, LevelKnockback }, skillStomp);
	candidates.add(0, 1, 1, 0, SlotEnum::MainHand, { LevelPierce, LevelDuration, LevelGroundEffect }, skillBearTrap);
	candidates.add(0, 1, 1, 0, SlotEnum::Legs, { LevelFire, LevelGroundEffect }, skillSmokeLeap);
	candidates.add(0, 1, 0, 0, SlotEnum::Body, { LevelPoison, LevelAoe, LevelDuration, LevelGroundEffect }, skillPoisonVial);
	candidates.add(0, 1, 0, 1, SlotEnum::Body, { LevelDuration, LevelSummoning }, skillConstructBallista);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelAoe, LevelDuration, LevelStun }, skillWarcry);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelDuration }, skillFocus);
	candidates.add(0, 0, 1, 0, SlotEnum::OffHand, { Nothing }, skillPatchWounds);
	candidates.add(0, 0, 1, 1, SlotEnum::OffHand, { Nothing }, skillPatchWounds);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { Nothing }, skillBerserk);
	candidates.add(0, 0, 0, 1, SlotEnum::Head, { LevelDuration }, skillIntimidate);
	candidates.add(0, 0, H, 1, SlotEnum::OffHand, { LevelDuration, LevelStun }, skillLiquor);
	candidates.add(0, 1, 0, 0, SlotEnum::Legs, { Nothing }, skillCharge);
	candidates.add(0, 0, 1, 1, SlotEnum::OffHand, { LevelAoe, LevelDuration, LevelGroundEffect }, skillWaterSplash);
	candidates.add(0, 0, 0, 1, SlotEnum::Body, { LevelFire, LevelAoe, LevelDuration, LevelSummoning }, skillLandMine);
	candidates.add(0, 0, 1, 1, SlotEnum::MainHand, { LevelSlash, LevelPierce, LevelDuration, LevelGroundEffect }, skillSandCastle);
	candidates.add(0, 0, 0, 1, SlotEnum::Legs, { LevelDuration }, skillTraining);

	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { LevelFire }, skillScorch);
	candidates.add(1, 0, 0, 0, SlotEnum::MainHand, { LevelElectric, LevelAoe }, skillShockNova);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { LevelFire }, skillFireball);
	candidates.add(1, 1, 0, 0, SlotEnum::Legs, { LevelElectric, LevelStun }, skillThunderboltLeap);
	candidates.add(1, 1, 0, 0, SlotEnum::MainHand, { LevelFire, LevelAoe, LevelDuration, LevelGroundEffect }, skillMeteor);
	candidates.add(1, 1, 1, 0, SlotEnum::Head, { LevelDuration }, skillManaDrain);
	candidates.add(1, 1, 1, 1, SlotEnum::Body, { Nothing }, skillTeleport);
	candidates.add(1, 1, 0, 1, SlotEnum::Body, { Nothing }, skillBodySwap);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { Nothing }, skillHeal);
	candidates.add(1, H, 0, 1, SlotEnum::Head, { Nothing }, skillBloodMagic);
	candidates.add(1, 0, 0, 1, SlotEnum::Body, { LevelDuration, LevelSummoning }, skillSummonMinion);
	candidates.add(1, 1, 0, 1, SlotEnum::MainHand, { LevelElectric, LevelDuration, LevelStun }, skillElectrocute);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelPierce, LevelFire, LevelDuration }, skillIceShield);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelSlash, LevelPoison, LevelDuration }, skillFleshOfSteel);
	candidates.add(1, 1, 1, 1, SlotEnum::OffHand, { LevelElectric, LevelDuration }, skillFaradayCage);
	candidates.add(1, 0, 1, H, SlotEnum::Body, { LevelDuration }, skillAlternateSkin);
	candidates.add(1, H, 0, 0, SlotEnum::OffHand, { LevelSupport }, skillDeathCoil);
	candidates.add(1, 0, 0, 0, SlotEnum::Head, { LevelFire }, skillManaBurn);
	candidates.add(1, 0, 0, 1, SlotEnum::Body, { LevelDuration }, skillSoulCrucible);
	candidates.add(1, 0, 0, 0, SlotEnum::OffHand, { LevelElectric }, skillDischarge);
	candidates.add(1, 0, 0, 0, SlotEnum::Legs, { LevelFire, LevelDuration, LevelGroundEffect }, skillLavaWalk);
	candidates.add(1, 0, 0, 1, SlotEnum::Head, { Nothing }, skillManaChannel);
	candidates.add(1, 0, 0, 1, SlotEnum::Head, { Nothing }, skillStaminaChannel);

	candidates.fallback(skillLaugh);
	return candidates.pick()(generate);
}
