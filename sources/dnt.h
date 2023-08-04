#include <cage-core/math.h>

#include <array>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace cage;

struct Skill;
struct Item;
struct Monster;

using Variant = std::variant<std::string, Skill, Item, Monster>;

enum class AffixEnum : uint8
{
	Prefix,
	Infix,
	Suffix,
	_Total
};

struct Affix
{
	detail::StringBase<120> name;
	Real relevance;
};

struct Thing : private Noncopyable
{
	std::array<Affix, (uint32)AffixEnum::_Total> affixes;

	uint32 powersCount = 0;
	Real powerWeight; // sum of weights
	Real powerTotal; // sum of weighted rolls
	Real goldCost = 1;

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

using AttributesValuesList = std::map<AttributeEnum, sint32>;
using AttributesEquationFactors = std::map<AttributeEnum, Real>;
using SkillAttributes = std::map<AttributeEnum, AttributesEquationFactors>;

constexpr const char *Alone = "\"alone\""; // requires that the caster is alone (no other creature (player or monster) are visible in 15 range)
constexpr const char *LineOfSight = "\"lineOfSight\""; // requires the target position is visible from the caster position
constexpr const char *Moves = "\"moves\""; // moves the caster to the target position, or the target to the caster position
constexpr const char *Knockback = "\"knockback\""; // moves the caster/target one tile away from the other
constexpr const char *Stun = "\"stun\""; // prevents the caster/target from performing any actions for one tick, and grants immunity to stun for the following tick
constexpr const char *GroundEffect = "\"groundEffect\""; // creates ground effect at caster/target position, which applies the effects of the skill
constexpr const char *Passive = "\"passive\""; // the effects of the skill are automatically applied every tick, assuming the cost can be paid; multiple passive skills are allowed

struct Skill : public Thing
{
	String name = "unnamed skill";
	detail::StringBase<30> icon = "skill";
	SkillTargetEnum target = SkillTargetEnum::None;
	AttributesValuesList cost;
	AttributesEquationFactors range, radius, duration, damageAmount;
	DamageTypeEnum damageType = DamageTypeEnum::None;
	SkillAttributes casterAttributes, targetAttributes;
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
	String name = "unnamed item";
	detail::StringBase<30> icon = "item";
	SlotEnum slot = SlotEnum::None;
	AttributesValuesList requirements;
	AttributesValuesList attributes;
	std::vector<Skill> skills;
	std::vector<std::string> flags;
	uint32 buyPrice = 0;
};

struct Monster : public Thing
{
	String name = "unnamed monster";
	detail::StringBase<30> icon = "monster";
	detail::StringBase<30> algorithm = "default";
	detail::StringBase<30> faction = "monster";
	AttributesValuesList attributes;
	std::vector<Item> equippedItems;
	std::vector<Variant> onDeath;
	uint32 score = 0;
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
	// other
	Placeholder, // used temporarily in some algorithms
};

enum class OccupancyEnum : uint8
{
	Free, // empty tiles, walkable decorations
	Prop, // doors, chests, monsters
	Block, // walls, non-walkable decorations
};

using TileExtra = std::vector<Variant>;

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

struct Generate
{
	uint32 level = 0;
	uint32 power = 0;
	SlotEnum slot = SlotEnum::None;

	Real magic = Real::Nan(); // 0 = warrior, 1 = sorcerer
	Real ranged = Real::Nan(); // 0 = melee, 1 = ranged
	Real defensive = Real::Nan(); // 0 = offensive, 1 = defensive
	Real support = Real::Nan(); // 0 = combat, 1 = support

	Generate() = default;
	explicit Generate(uint32 level, sint32 powerOffset, SlotEnum slot = SlotEnum::None);
	void randomize();
	bool valid() const;
	sint32 powerOffset() const;
};

void removeLastComma(std::string &json);
OccupancyEnum occupancy(TileEnum tile);
bool isLevelBoss(uint32 level);
uint32 levelToBossIndex(uint32 level);
uint32 bossIndexToLevel(uint32 index);

Skill generateSkill(const Generate &generate);
Item generateItem(const Generate &generate);
Monster generateMonster(const Generate &generate);
Monster generateChest(const Generate &generate);
Floor generateFloor(uint32 level, uint32 maxLevel);

std::string exportVariant(const Variant &variant);
std::string exportSkill(const Skill &skill);
std::string exportItem(const Item &item);
std::string exportMonster(const Monster &monster);
FloorExport exportFloor(const Floor &floor);
void exportDungeon(PointerRange<const Floor> floors);
void exportExamples(uint32 maxLevel);

template<class... T>
constexpr bool always_false = false;
