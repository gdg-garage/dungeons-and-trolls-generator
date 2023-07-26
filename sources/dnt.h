#include <cage-core/math.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using namespace cage;

struct ThingPower
{
	Real relative; // usually in range 0 .. 1 -> defines how good were random rolls from various ranges
	Real absolute; // estimated value of this thing in gold
};

enum class AttributeEnum : uint8
{
	// primary
	Strength,
	Dexterity,
	Inteligence,
	Willpower,
	Constitution,
	// secondary
	SlashArmor,
	PiercingArmor,
	FireResist,
	PoisonResist,
	ElectricResist,
	// stats
	Life,
	Mana,
	Stamina,
	// other
	Scalar,
};

using AttributesValueMapping = std::map<AttributeEnum, sint16>;

enum class DamageTypeEnum : uint8
{
	None,
	Slash,
	Piercing,
	Fire,
	Poison,
	Electric,
};

enum class SkillTargetEnum : uint8
{
	None,
	Position,
	Character,
	Item,
};

struct SkillCost
{
	uint16 life = 0;
	uint16 mana = 0;
	uint16 stamina = 0;
};

using SkillAttributesEffects = std::map<AttributeEnum, AttributesValueMapping>; // contains values in percentages -> value 100 multiplies by 1

struct Skill : private Noncopyable
{
	String name = "<unnamed skill>";
	SkillTargetEnum target = SkillTargetEnum::None;
	SkillCost cost;
	AttributesValueMapping range, radius, duration, damageAmount;
	DamageTypeEnum damageType = DamageTypeEnum::None;
	SkillAttributesEffects casterAttributes, targetAttributes;
	bool movesCaster = false; // moves the caster to the target position
	bool movesTarget = false; // moves targeted entity to position of the caster
	bool knockback = false; // moves targeted entity one tile away from the caster
	bool stun = false; // prevents the target from any actions for one tick, and grants stun immunity for the following tick
	bool requiresLineOfSight = true; // the target position must be visible from the caster position
	bool requiresCasterIsAlone = false; // no other characters may be visible (line of sight check) anywhere from the caster position
	bool createsGroundEffect = false;
	ThingPower power;
};

enum class SlotEnum : uint8
{
	None,
	MainHand,
	OffHand,
	Head,
	Body,
	Legs,
	Neck,
};

struct Item : private Noncopyable
{
	String name = "<unnamed item>";
	SlotEnum slot = SlotEnum::None;
	AttributesValueMapping requirements;
	AttributesValueMapping attributes;
	std::vector<Skill> skills;
	ThingPower power;
};

struct Monster;

using OnDeathEffect = std::variant<std::unique_ptr<Skill>, std::unique_ptr<Item>, std::unique_ptr<Monster>>;

struct Monster : private Noncopyable
{
	String name = "<unnamed monster>";
	AttributesValueMapping attributes;
	std::vector<Item> equippedItems;
	std::vector<OnDeathEffect> onDeath;
	ThingPower power;
};

enum class TileEnum : uint8
{
	// free
	Empty,
	Decoration,
	// props
	Spawn, // the position where all players spawn when entering this level (in any way)
	Waypoint, // teleports players to any other floor with a waypoint
	Stairs, // teleports players to next floor
	Door,
	Chest,
	Monster,
	// blocking
	Wall,
	Outside,
};

enum class OccupancyEnum : uint8
{
	Free, // empty tiles, walkable decorations
	Prop, // doors, chests, monsters
	Block, // walls, non-walkable decorations
};

using TileExtra = std::variant<std::monostate, std::string, std::unique_ptr<Monster>>;

struct Floor : private Noncopyable
{
	std::vector<TileEnum> tiles;
	std::vector<TileExtra> extras;
	uint32 width = 0, height = 0;
	uint32 level = 0;

	bool inside(Vec2i pos) const { return pos[0] >= 0 && pos[1] >= 0 && pos[0] < width && pos[1] < height; }

	TileEnum &tile(Vec2i pos)
	{
		CAGE_ASSERT(inside(pos));
		return tiles[pos[1] * width + pos[0]];
	}

	TileEnum tile(Vec2i pos) const
	{
		CAGE_ASSERT(inside(pos));
		return tiles[pos[1] * width + pos[0]];
	}

	TileEnum &tile(uint32 x, uint32 y) { return tile(Vec2i(x, y)); }

	TileEnum tile(uint32 x, uint32 y) const { return tile(Vec2i(x, y)); }

	TileExtra &extra(Vec2i pos)
	{
		CAGE_ASSERT(inside(pos));
		return extras[pos[1] * width + pos[0]];
	}

	const TileExtra &extra(Vec2i pos) const
	{
		CAGE_ASSERT(inside(pos));
		return extras[pos[1] * width + pos[0]];
	}

	TileExtra &extra(uint32 x, uint32 y) { return extra(Vec2i(x, y)); }

	const TileExtra &extra(uint32 x, uint32 y) const { return extra(Vec2i(x, y)); }
};

struct FloorExport
{
	std::string html;
	std::string json;
};

void removeLastComma(std::string &json);

const char *attributeName(AttributeEnum attribute);
std::string attributesValueMappingJson(const AttributesValueMapping &attributesValues);
const char *damageTypeName(DamageTypeEnum damageType);
const char *skillTargetName(SkillTargetEnum skillTarget);
std::string skillCostJson(const SkillCost &cost);
std::string skillAttributesEffectsJson(const SkillAttributesEffects &effects);
const char *slotName(SlotEnum slot);

Skill generateSkill(uint32 level, SlotEnum slot);
std::string skillJson(const Skill &skill);
Item generateItem(uint32 level, SlotEnum slot);
std::string itemJson(const Item &item);
Monster generateMonster(uint32 level, uint32 difficulty);
std::string monsterJson(const Monster &monster);

const char *tileName(TileEnum tile);
OccupancyEnum occupancy(TileEnum tile);
bool isLevelBoss(uint32 level);
uint32 levelToBossIndex(uint32 level);
uint32 bossIndexToLevel(uint32 index);
Floor generateFloor(uint32 level);
FloorExport exportFloor(const Floor &floor);
void exportDungeon(PointerRange<const Floor> floors);
