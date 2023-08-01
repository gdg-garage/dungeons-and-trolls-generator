#include <cage-core/math.h>

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using namespace cage;

enum class AffixEnum : uint8
{
	Prefix,
	Infix,
	Suffix,
	_Total
};

struct Affix
{
	String name;
	Real relevance;
};

struct Thing : private Noncopyable
{
	std::array<Affix, (uint32)AffixEnum::_Total> affixes;

	uint32 powersCount = 0;
	Real powerWeight; // sum of weights
	Real powerTotal; // sum of weighted rolls
	Real goldCost;

	Real addPower(Real weight);
	Real addPower(Real roll, Real weight);
	Real addPower(Real weight, AffixEnum affix, const String &name);
	Real addPower(Real roll, Real weight, AffixEnum affix, const String &name);
	void addPower(const Thing &other, Real weight);
	void addAffix(Real relevance, AffixEnum affix, const String &name);
	String makeName(const String &basicName, Real relevance = 0.5) const;
};

enum class AttributeEnum : uint8
{
	// primary
	Strength,
	Dexterity,
	Intelligence,
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

struct Skill : public Thing
{
	String name = "<unnamed skill>";
	SkillTargetEnum target = SkillTargetEnum::None;
	SkillCost cost;
	AttributesValueMapping range, radius, duration, damageAmount;
	DamageTypeEnum damageType = DamageTypeEnum::None;
	SkillAttributesEffects casterAttributes, targetAttributes;
	std::vector<std::string> casterFlags, targetFlags;
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

struct Item : public Thing
{
	String name = "<unnamed item>";
	SlotEnum slot = SlotEnum::None;
	AttributesValueMapping requirements;
	AttributesValueMapping attributes;
	std::vector<Skill> skills;
	std::vector<std::string> flags;
};

struct Monster;

using OnDeathEffect = std::variant<std::unique_ptr<Skill>, std::unique_ptr<Item>, std::unique_ptr<Monster>>;

struct Monster : public Thing
{
	String name = "<unnamed monster>";
	AttributesValueMapping attributes;
	std::vector<Item> equippedItems;
	std::vector<OnDeathEffect> onDeath;
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
OccupancyEnum occupancy(TileEnum tile);
bool isLevelBoss(uint32 level);
uint32 levelToBossIndex(uint32 level);
uint32 bossIndexToLevel(uint32 index);

Skill generateMagicSpell(uint32 level, SlotEnum slot);
Item generateEquippedItem(uint32 level, SlotEnum slot);
Item generateDroppedItem(uint32 level);
Monster generateMonster(uint32 level, sint32 difficultyOffset);
Monster generateSummonedMinion(uint32 level);
Monster generateChest(uint32 level);
Floor generateFloor(uint32 level);

std::string exportSkill(const Skill &skill);
std::string exportItem(const Item &item);
std::string exportMonster(const Monster &monster);
FloorExport exportFloor(const Floor &floor);
void exportDungeon(PointerRange<const Floor> floors);

template<class... T>
constexpr bool always_false = false;
