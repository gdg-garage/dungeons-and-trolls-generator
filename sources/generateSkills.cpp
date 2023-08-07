#include "dnt.h"

#include <algorithm>

namespace
{
	Skill generateRandomSkill(const Generate &generate)
	{
		generate.valid();

		Skill sk;

		if (randomChance() < 0.3)
			sk.target = SkillTargetEnum::Character;
		else if (randomChance() < 0.5)
			sk.target = SkillTargetEnum::Position;

		if (randomChance() < 0.6)
			sk.cost[AttributeEnum::Stamina] = randomRange(2, 5);
		if (randomChance() < 0.4)
			sk.cost[AttributeEnum::Mana] = randomRange(2, 5);

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

		const auto &sae = []() -> SkillAttributes
		{
			SkillAttributes r;
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
			json += "\"data\":" + exportMonster(generateMinion(Generate(generate.level, generate.powerOffset()))) + "\n";
			json += "}";
			sk.targetFlags.push_back(std::move(json));
		}

		return sk;
	}
}

namespace
{
	// mundane ranged offensive combat
	// pierce, moves, stun
	Skill generateChainHook(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Chain Hook";
		return sk;
	}

	// mundane melee defensive combat
	// slash, aoe, knockback
	Skill generateStamp(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Stamp";
		return sk;
	}

	// mundane ranged defensive combat
	// pierce, stun, ground effect
	Skill generateBearTrap(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Bear Trap";
		return sk;
	}

	// mundane ranged offensive combat
	// poison, aoe, duration, ground effect
	Skill generatePoisonCloud(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Poison Cloud";
		return sk;
	}

	// mundane ranged offensive support
	// duration, summon
	Skill generateConstructBalista(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Construct Balista";
		return sk;
	}

	// mundane melee offensive support
	// aoe, duration, boost, stun
	Skill generateWarcry(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Warcry";
		return sk;
	}

	// mundane --- offensive support
	// duration, boost
	Skill generateFocus(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Focus";
		return sk;
	}

	// mundane --- defensive support
	//
	Skill generatePatchWounds(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Patch Wounds";
		return sk;
	}

	// mundane --- offensive support
	//
	Skill generateBerserk(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Berserk";
		return sk;
	}

	// mundane melee --- support
	// duration, curse
	Skill generateIntimidate(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Intimidate";
		return sk;
	}
}

namespace
{
	// magic melee offensive combat
	// electric, aoe
	Skill generateShockNova(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Shock Nova";
		return sk;
	}

	// magic ranged offensive combat
	// fire
	Skill generateFireball(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Fireball";
		return sk;
	}

	// magic ranged offensive combat
	// electric, moves, stun
	Skill generateLightningJump(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Lightning Jump";
		return sk;
	}

	// magic ranged offensive combat
	// fire, aoe, duration, ground effect
	Skill generateMeteor(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Meteor";
		return sk;
	}

	// magic ranged defensive combat
	// duration, curse, boost
	Skill generateManaDrain(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Mana Drain";
		return sk;
	}

	// magic ranged defensive support
	// moves
	Skill generateTeleport(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Teleport";
		return sk;
	}

	// magic ranged offensive support
	// moves
	Skill generateBodySwap(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Body Swap";
		return sk;
	}

	// magic ranged defensive support
	// duration
	Skill generateHeal(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Heal";
		return sk;
	}

	// magic --- offensive support
	//
	Skill generateBloodMagic(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Blood Magic";
		return sk;
	}

	// magic melee offensive support
	// duration, summon
	Skill generateSummonMinion(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Summon Minion";
		return sk;
	}

	// magic ranged offensive support
	// electric, duration, curse, stun
	Skill generateElectrocute(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Electrocute";
		return sk;
	}

	// magic ranged defensive support
	// piercing, fire, duration, boost
	Skill generateIceShield(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Ice Shield";
		return sk;
	}

	// magic ranged defensive support
	// slash, poison, duration, boost
	Skill generateFleshOfSteel(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Flesh Of Steel";
		return sk;
	}

	// magic ranged defensive support
	// electric, duration, boost
	Skill generateFaradayCage(const Generate &generate)
	{
		Skill sk = generateRandomSkill(generate);
		sk.name = "Faraday Cage";
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

	static constexpr Real H = 0.5;
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
	add(0, 0, 1, 0, SlotEnum::Legs, { Slash, Aoe, Knockback }, generateStamp);
	add(0, 1, 1, 0, SlotEnum::MainHand, { Pierce, Stun, GroundEffect }, generateBearTrap);
	add(0, 1, 0, 0, SlotEnum::Body, { Poison, Aoe, Duration, GroundEffect }, generatePoisonCloud);
	add(0, 1, 0, 1, SlotEnum::Body, { Duration, Summoning }, generateConstructBalista);
	add(0, 0, 0, 1, SlotEnum::Head, { Aoe, Duration, Stun }, generateWarcry);
	add(0, H, 0, 1, SlotEnum::Head, { Duration }, generateFocus);
	add(0, H, 1, 1, SlotEnum::OffHand, { Nothing }, generatePatchWounds);
	add(0, H, 0, 1, SlotEnum::Head, { Nothing }, generateBerserk);
	add(0, 0, H, 1, SlotEnum::Head, { Duration }, generateIntimidate);

	add(1, 0, 0, 0, SlotEnum::MainHand, { Electric, Aoe }, generateShockNova);
	add(1, 1, 0, 0, SlotEnum::MainHand, { Fire }, generateFireball);
	add(1, 1, 0, 0, SlotEnum::Legs, { Electric, Stun }, generateLightningJump);
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

	std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b) { return a.penalty < b.penalty; });

	// todo slightly randomize
	return candidates[0].generator(generate);
}
