#include "dnt.h"

void fixup(Skill &sk)
{
	if (sk.targetType == SkillTargetEnum::None)
		sk.flags.requiresLineOfSight = false;
}

void fixup(Item &item)
{
	for (Skill &sk : item.skills)
		fixup(sk);
}

void fixup(Monster &mr)
{
	for (Item &it : mr.equippedItems)
		fixup(it);
	for (Variant &it : mr.onDeath)
		fixup(it);
}

void fixup(Decoration &decoration) {}

void fixup(Waypoint &waypoint) {}

void fixup(Key &key) {}

void fixup(Variant &variant)
{
	std::visit([](auto &arg) { fixup(arg); }, variant);
}
