#include "dnt.h"

Skill generateSkill(uint32 level, SlotEnum slot)
{
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
		sk.range[AttributeEnum::Inteligence] = randomRange(20, 60);

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

	sk.movesCaster = randomChance() < 0.3;
	sk.knockback = randomChance() < 0.1;
	sk.stun = randomChance() < 0.05;
	sk.createsGroundEffect = randomChance() < 0.1;

	return sk;
}

std::string skillJson(const Skill &skill)
{
	std::string json;
	json += "{\n";
	json += std::string() + "\"name\":\"" + skill.name.c_str() + "\",\n";
	json += std::string() + "\"target\":\"" + skillTargetName(skill.target) + "\",\n";
	json += std::string() + "\"cost\":" + skillCostJson(skill.cost) + ",\n";
	json += std::string() + "\"range\":" + attributesValueMappingJson(skill.range) + ",\n";
	json += std::string() + "\"radius\":" + attributesValueMappingJson(skill.radius) + ",\n";
	json += std::string() + "\"duration\":" + attributesValueMappingJson(skill.duration) + ",\n";
	json += std::string() + "\"damageAmount\":" + attributesValueMappingJson(skill.damageAmount) + ",\n";
	json += std::string() + "\"damageType\":\"" + damageTypeName(skill.damageType) + "\",\n";
	json += std::string() + "\"casterAttributes\":" + skillAttributesEffectsJson(skill.casterAttributes) + ",\n";
	json += std::string() + "\"targetAttributes\":" + skillAttributesEffectsJson(skill.targetAttributes) + ",\n";

	// boolean2json, not blowjob ;)
	const auto &bj = [&](const char *name, bool value) { json += std::string() + "\"" + name + "\":" + (value ? "true" : "false") + ",\n"; };
	bj("movesCaster", skill.movesCaster);
	bj("movesTarget", skill.movesTarget);
	bj("knockback", skill.knockback);
	bj("stun", skill.stun);
	bj("requiresLineOfSight", skill.requiresLineOfSight);
	bj("requiresCasterIsAlone", skill.requiresCasterIsAlone);
	bj("createsGroundEffect", skill.createsGroundEffect);

	removeLastComma(json);
	json += "}"; // /root
	return json;
}
