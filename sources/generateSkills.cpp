#include "dnt.h"

Skill generateSkill(const Generate &generate)
{
	CAGE_ASSERT(valid(generate.magic) && valid(generate.ranged) && valid(generate.support));

	Skill sk;

	if (randomChance() < 0.3)
		sk.target = SkillTargetEnum::Character;
	else if (randomChance() < 0.5)
		sk.target = SkillTargetEnum::Position;

	if (randomChance() < 0.6)
		sk.cost.stamina = randomRange(2, 5);
	if (randomChance() < 0.4)
		sk.cost.mana = randomRange(2, 5);

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

	const auto &sae = []() -> SkillAttributesEffects
	{
		SkillAttributesEffects r;
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
		json += "\"data\":" + exportMonster(generateMonster(Generate(generate.level))) + "\n";
		json += "}";
		sk.targetFlags.push_back(std::move(json));
	}

	return sk;
}
