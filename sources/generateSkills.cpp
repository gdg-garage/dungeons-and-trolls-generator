#include "dnt.h"

#include <algorithm>

namespace
{
	Real makeAttrFactor(uint32 power, Real roll)
	{
		return interpolate(0.1, 1.0, roll) + power * 0.05;
	}

	Real makeAttrFactor(Thing &sk, const Generate &generate, Real weight)
	{
		return makeAttrFactor(generate.power, sk.addPower(weight));
	}

	Real makeAttrFactor(Thing &sk, const Generate &generate, Real weight, const String &affixName, AffixEnum affixPos = AffixEnum::Prefix)
	{
		return makeAttrFactor(generate.power, sk.addPower(weight, affixPos, affixName));
	}

	uint32 makeCost(Thing &sk, const Generate &generate, Real default_)
	{
		const Real roll = sk.addPower(0.7, AffixEnum::Prefix, "Thrifty");
		const Real p = interpolate(default_ * 1.10, default_ * 0.95 * 100 / (100 + generate.power), roll);
		return numeric_cast<uint32>(max(p, 1));
	}
}

namespace
{
	Skill generateChainHook(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Stretching") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 0.7, "Surgical") * 0.5;
		sk.damageType = DamageTypeEnum::Piercing;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 35);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(Moves);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(Stun);
		sk.name = sk.makeName("Chain Hook");
		return sk;
	}

	Skill generateStomp(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, AffixEnum::Prefix, "Expansive"));
		sk.damageAmount[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Forceful") * 0.3;
		sk.damageType = DamageTypeEnum::Slash;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 30);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(Knockback);
		sk.name = sk.makeName("Stomp");
		return sk;
	}

	Skill generateBearTrap(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = interpolate(4.0, 6.0, sk.addPower(0.5));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Durable"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Sprung"));
		sk.damageType = DamageTypeEnum::Piercing;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 15);
		sk.targetFlags.push_back(GroundEffect);
		sk.name = sk.makeName("Bear Trap");
		return sk;
	}

	Skill generatePoisonVial(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 0.6, "Throwing") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(0.8, AffixEnum::Prefix, "Splashing"));
		sk.duration[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Sticky"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Potent"));
		sk.damageType = DamageTypeEnum::Poison;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 20);
		sk.targetFlags.push_back(GroundEffect);
		sk.name = sk.makeName("Poison Vial");
		return sk;
	}

	Skill generateSmokeLeap(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Light") * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(1.0, 4.0, sk.addPower(0.7, AffixEnum::Prefix, "Dense"));
		sk.damageAmount[AttributeEnum::Scalar] = interpolate(5.0, 10.0, sk.addPower(1, AffixEnum::Prefix, "Fiery"));
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 20);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(Moves);
		sk.casterFlags.push_back(GroundEffect);
		sk.name = sk.makeName("Smoke Leap");
		return sk;
	}

	Skill generateConstructBallista(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.duration[AttributeEnum::Scalar] = interpolate(10.0, 15.0, sk.addPower(1));
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 55);
		{
			Monster mr;
			mr.icon = "ballista";
			mr.algorithm = "ballista";
			mr.faction = "inherited";
			// todo
			mr.name = mr.makeName("Ballista");

			std::string json;
			json += "{\n";
			json += "\"class\":\"summon\",\n";
			json += "\"data\":" + exportMonster(mr) + "\n";
			json += "}";

			sk.addPower(mr, 1);
			sk.targetFlags.push_back(std::move(json));
		}
		sk.name = sk.makeName("Construct Ballista");
		return sk;
	}

	Skill generateWarcry(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(2.0, 4.0, sk.addPower(1, AffixEnum::Prefix, "Loud"));
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.8, AffixEnum::Prefix, "Motivating"));
		sk.casterAttributes[AttributeEnum::Strength][AttributeEnum::Constitution] = makeAttrFactor(sk, generate, 0.8, "Strengthening") * 0.1;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 25);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(Stun);
		sk.name = sk.makeName("Warcry");
		return sk;
	}

	Skill generateFocus(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Prolonged"));
		sk.casterAttributes[AttributeEnum::Intelligence][AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Deep") * 0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.name = sk.makeName("Focus");
		return sk;
	}

	Skill generatePatchWounds(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Doctorly") * 0.2;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Dexterity] = makeAttrFactor(sk, generate, 1, "Carefully") * 0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.name = sk.makeName("Patch Wounds");
		return sk;
	}

	Skill generateBerserk(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.targetAttributes[AttributeEnum::Stamina][AttributeEnum::Scalar] = makeAttrFactor(sk, generate, 1, "Raging") * 25;
		sk.cost[AttributeEnum::Life] = makeCost(sk, generate, 15);
		sk.name = sk.makeName("Berserk");
		return sk;
	}

	Skill generateIntimidate(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Enduring"));
		sk.targetAttributes[AttributeEnum::Intelligence][AttributeEnum::Strength] = makeAttrFactor(sk, generate, 1, "Frightening") * -0.2;
		sk.cost[AttributeEnum::Stamina] = makeCost(sk, generate, 10);
		sk.name = sk.makeName("Intimidate");
		return sk;
	}
}

namespace
{
	Skill generateShockNova(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.radius[AttributeEnum::Scalar] = interpolate(3.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Vast"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Energetic") * 0.3;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 25);
		sk.name = sk.makeName("Shock Nova");
		return sk;
	}

	Skill generateFireball(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.8, "Missile") * 0.1;
		sk.range[AttributeEnum::Scalar] = 5;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Blazing") * 1;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.name = sk.makeName("Fireball");
		return sk;
	}

	Skill generateThunderboltLeap(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.8, "Bouncing") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 0.9, "Frying") * 0.5;
		sk.damageType = DamageTypeEnum::Electric;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 40);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(Moves);
		sk.addPower(1, 1);
		sk.targetFlags.push_back(Stun);
		sk.name = sk.makeName("Thunderbolt Leap");
		return sk;
	}

	Skill generateMeteor(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.6, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 2;
		sk.radius[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 0.8, "Shower", AffixEnum::Suffix) * 0.1;
		sk.radius[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(1, AffixEnum::Prefix, "Cosmic"));
		sk.damageAmount[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Vigorous") * 0.25;
		sk.damageType = DamageTypeEnum::Fire;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 30);
		sk.targetFlags.push_back(GroundEffect);
		sk.name = sk.makeName("Meteor");
		return sk;
	}

	Skill generateManaDrain(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.targetAttributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Sucking") * -0.5;
		sk.casterAttributes[AttributeEnum::Mana][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Generous") * 0.5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.name = sk.makeName("Mana Drain");
		return sk;
	}

	Skill generateTeleport(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Position;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Distant") * 0.2;
		sk.range[AttributeEnum::Scalar] = 5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 40);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(Moves);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(NoLineOfSight);
		sk.name = sk.makeName("Teleport");
		return sk;
	}

	Skill generateBodySwap(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Distant") * 0.2;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 60);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(Moves);
		sk.targetFlags.push_back(Moves);
		sk.addPower(1, 1);
		sk.casterFlags.push_back(NoLineOfSight);
		sk.name = sk.makeName("Body Swap");
		return sk;
	}

	Skill generateHeal(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 1, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.targetAttributes[AttributeEnum::Life][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Concentrated") * 0.5;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.name = sk.makeName("Heal");
		return sk;
	}

	Skill generateBloodMagic(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::None;
		sk.targetAttributes[AttributeEnum::Mana][AttributeEnum::Scalar] = makeAttrFactor(sk, generate, 1, "Puissant") * 35;
		sk.cost[AttributeEnum::Life] = makeCost(sk, generate, 20);
		sk.name = sk.makeName("Blood Magic");
		return sk;
	}

	Skill generateSummonMinion(const Generate &generate)
	{
		Skill sk;
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

			sk.name = Stringizer() + "Summon Minion: " + mr.name;
		}
		return sk;
	}

	Skill generateElectrocute(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 4;
		sk.duration[AttributeEnum::Scalar] = interpolate(3.0, 6.0, sk.addPower(0.8, AffixEnum::Prefix, "Constricting"));
		sk.targetAttributes[AttributeEnum::Strength][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Choking") * -0.2;
		sk.targetAttributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Conductive") * -0.15;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.name = sk.makeName("Electrocute");
		return sk;
	}

	Skill generateIceShield(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::PiercingArmor][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Impenetrable") * 0.2;
		sk.targetAttributes[AttributeEnum::FireResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Fireproof") * 0.2;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.name = sk.makeName("Ice Shield");
		return sk;
	}

	Skill generateFleshOfSteel(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::SlashArmor][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Slipping") * 0.2;
		sk.targetAttributes[AttributeEnum::PoisonResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Resilient") * 0.2;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 20);
		sk.name = sk.makeName("Flesh Of Steel");
		return sk;
	}

	Skill generateFaradayCage(const Generate &generate)
	{
		Skill sk;
		sk.target = SkillTargetEnum::Character;
		sk.range[AttributeEnum::Willpower] = makeAttrFactor(sk, generate, 0.7, "Distant") * 0.1;
		sk.range[AttributeEnum::Scalar] = 3;
		sk.duration[AttributeEnum::Scalar] = interpolate(2.0, 5.0, sk.addPower(0.9, AffixEnum::Prefix, "Durable"));
		sk.targetAttributes[AttributeEnum::ElectricResist][AttributeEnum::Intelligence] = makeAttrFactor(sk, generate, 1, "Grounded") * 0.2;
		sk.cost[AttributeEnum::Mana] = makeCost(sk, generate, 15);
		sk.name = sk.makeName("Faraday Cage");
		return sk;
	}
}

Skill generateSkill(const Generate &generate)
{
	struct Candidate
	{
		Real penalty;
		Skill (*generator)(const Generate &generate);
	};
	std::vector<Candidate> candidates;
	candidates.reserve(100);
	const auto &add = [&](Real magic, Real ranged, Real defensive, Real support, SlotEnum preferredSlot, const std::initializer_list<uint32> &requiredLevels, Skill (*generator)(const Generate &generate))
	{
		uint32 minLevel = std::max(requiredLevels);
		if (generate.ranged > 0.5)
			minLevel = max(minLevel, 9u);
		if (generate.magic > 0.5)
			minLevel = max(minLevel, 16u);
		if (generate.support > 0.5)
			minLevel = max(minLevel, 29u);
		const Real p = abs(generate.magic - magic) + abs(generate.ranged - ranged) + abs(generate.defensive - defensive) + abs(generate.support - support) + (generate.slot != preferredSlot) + (generate.level > minLevel ? 0 : 10) + randomChance() * 0.2;
		candidates.push_back({ p, generator });
	};

	static constexpr float H = 0.5;
	static constexpr uint32 Nothing = 0;
	static constexpr uint32 Slash = 0;
	static constexpr uint32 Pierce = 7;
	static constexpr uint32 Aoe = 11;
	static constexpr uint32 Knockback = 11;
	static constexpr uint32 Fire = 16;
	static constexpr uint32 Duration = 22;
	static constexpr uint32 Poison = 37;
	static constexpr uint32 GroundEffect = 46;
	static constexpr uint32 Stun = 56;
	static constexpr uint32 Summoning = 67;
	static constexpr uint32 Electric = 79;

	add(0, 1, 0, 0, SlotEnum::MainHand, { Pierce, Stun }, generateChainHook);
	add(0, 0, 1, 0, SlotEnum::Legs, { Slash, Aoe, Knockback }, generateStomp);
	add(0, 1, 1, 0, SlotEnum::MainHand, { Pierce, Duration, GroundEffect }, generateBearTrap);
	add(0, 1, 1, 0, SlotEnum::Legs, { Fire, GroundEffect }, generateSmokeLeap);
	add(0, 1, 0, 0, SlotEnum::Body, { Poison, Aoe, Duration, GroundEffect }, generatePoisonVial);
	add(0, 1, 0, 1, SlotEnum::Body, { Duration, Summoning }, generateConstructBallista);
	add(0, 0, 0, 1, SlotEnum::Head, { Aoe, Duration, Stun }, generateWarcry);
	add(0, H, 0, 1, SlotEnum::Head, { Duration }, generateFocus);
	add(0, H, 1, 1, SlotEnum::OffHand, { Nothing }, generatePatchWounds);
	add(0, H, 0, 1, SlotEnum::Head, { Nothing }, generateBerserk);
	add(0, 0, H, 1, SlotEnum::Head, { Duration }, generateIntimidate);

	add(1, 0, 0, 0, SlotEnum::MainHand, { Electric, Aoe }, generateShockNova);
	add(1, 1, 0, 0, SlotEnum::MainHand, { Fire }, generateFireball);
	add(1, 1, 0, 0, SlotEnum::Legs, { Electric, Stun }, generateThunderboltLeap);
	add(1, 1, 0, 0, SlotEnum::MainHand, { Fire, Aoe, Duration, GroundEffect }, generateMeteor);
	add(1, 1, 1, 0, SlotEnum::Head, { Duration }, generateManaDrain);
	add(1, 1, 1, 1, SlotEnum::Body, { Nothing }, generateTeleport);
	add(1, 1, 0, 1, SlotEnum::Body, { Nothing }, generateBodySwap);
	add(1, 1, 1, 1, SlotEnum::OffHand, { Duration }, generateHeal);
	add(1, H, 0, 1, SlotEnum::Head, { Nothing }, generateBloodMagic);
	add(1, 0, 0, 1, SlotEnum::Body, { Duration, Summoning }, generateSummonMinion);
	add(1, 1, 0, 1, SlotEnum::MainHand, { Electric, Duration, Stun }, generateElectrocute);
	add(1, 1, 1, 1, SlotEnum::OffHand, { Pierce, Fire, Duration }, generateIceShield);
	add(1, 1, 1, 1, SlotEnum::OffHand, { Slash, Poison, Duration }, generateFleshOfSteel);
	add(1, 1, 1, 1, SlotEnum::OffHand, { Electric, Duration }, generateFaradayCage);

	CAGE_ASSERT(!candidates.empty());
	std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b) { return a.penalty < b.penalty; });
	return candidates[0].generator(generate);
}
