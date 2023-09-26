#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <cage-core/math.h>

using namespace cage;

struct Skill;
struct Item;
struct Monster;
struct Decoration;
struct Waypoint;
struct Key;

using Variant = std::variant<Skill, Item, Monster, Decoration, Waypoint, Key>;

struct Decoration
{
	detail::StringBase<30> icon;
	std::string name;
};

struct Waypoint
{
	uint32 destinationFloor = 0;
};

struct Key
{
	std::vector<Vec2i> doors;
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

struct Generate
{
	sint32 level = 0;
	sint32 power = 0;
	SlotEnum slot = SlotEnum::None;

	Real magic = Real::Nan(); // 0 = warrior, 1 = sorcerer
	Real ranged = Real::Nan(); // 0 = melee, 1 = ranged
	Real defensive = Real::Nan(); // 0 = offensive, 1 = defensive
	Real support = Real::Nan(); // 0 = combat, 1 = support

	Generate() = default;
	explicit Generate(sint32 level, sint32 powerOffset, SlotEnum slot = SlotEnum::None);
	void randomize();
	bool valid() const;
	sint32 powerOffset() const;
};

enum class AffixEnum : uint8
{
	Prefix,
	Infix,
	Suffix,
	_Total
};

struct Affix
{
	std::string name;
	Real relevance;
};

struct Thing : private Noncopyable
{
	Thing(const Generate &generate);

	std::string name = "unnamed";
	std::array<Affix, (uint32)AffixEnum::_Total> affixes;

	Generate generate;
	Real totalRolls; // sum of weighted rolls
	Real totalWeight; // sum of weights
	Real weightedRoll() const;
	Real averageWeight() const;
	Real goldCost = 0;

	Real addPower(Real weight);
	Real addPower(Real roll, Real weight);
	Real addPower(Real weight, const std::string &affixName, AffixEnum affixPos = AffixEnum::Prefix);
	Real subtractPower(Real weight, const std::string &affixName, AffixEnum affixPos = AffixEnum::Prefix);
	Real addPower(Real roll, Real weight, const std::string &affixName, AffixEnum affixPos = AffixEnum::Prefix);
	void addOther(const Thing &other, Real weight);
	void addAffix(Real relevance, const std::string &affixName, AffixEnum affixPos = AffixEnum::Prefix);
	void updateName();
	void updateName(const std::string &basicName, Real relevance = 0.5);
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
	SlashResist,
	PierceResist,
	FireResist,
	PoisonResist,
	ElectricResist,
	// stats
	Life,
	Mana,
	Stamina,
	// other
	Constant,
};

enum class DamageTypeEnum : uint8
{
	None,
	Slash,
	Pierce,
	Fire,
	Poison,
	Electric,
};

enum class SkillTargetEnum : uint8
{
	None,
	Position,
	Character,
};

using AttributesValuesList = std::map<AttributeEnum, sint32>;
using AttributesEquationFactors = std::map<AttributeEnum, Real>;
using SkillAttributes = std::map<AttributeEnum, AttributesEquationFactors>;
using SkillFlag = StringPointer;

struct SkillFlags
{
	bool requiresAlone = false; // requires that the caster is alone (no other creature (player or monster) are visible in 5 range), furthermore the skill is canceled if you take any damage while using it
	bool requiresLineOfSight = true; // requires the target position be visible from the caster position
	bool allowTargetSelf = false; // allows skills that target a character to target oneself
	bool movement = false; // moves the caster to the target position, or the target to the caster position
	bool knockback = false; // moves the caster/target one tile away from the other
	bool stun = false; // prevents the caster/target from performing any actions for one tick, and grants immunity to stun for the following tick
	bool groundEffect = false; // creates ground effect at caster/target position, which applies the effects of the skill
	bool passive = false; // the effects of the skill are automatically applied every tick, assuming the cost can be paid; multiple passive skills are allowed

	auto operator<=>(const SkillFlags &) const = default;
};

struct SkillEffects
{
	SkillAttributes attributes;
	SkillFlags flags;
	std::vector<Variant> summons;
};

struct Skill : public Thing
{
	Skill(const Generate &generate);

	SkillTargetEnum targetType = SkillTargetEnum::None;
	AttributesValuesList cost;
	AttributesEquationFactors range, radius, duration, damageAmount;
	DamageTypeEnum damageType = DamageTypeEnum::None;
	SkillEffects caster, target;
};

struct Item : public Thing
{
	Item(const Generate &generate);

	detail::StringBase<30> icon = "item";
	SlotEnum slot = SlotEnum::None;
	AttributesValuesList requirements;
	AttributesValuesList attributes;
	std::vector<Skill> skills;
	uint32 buyPrice = 0;
	bool unidentified = false;
};

struct Monster : public Thing
{
	Monster(const Generate &generate);

	detail::StringBase<30> icon = "monster";
	detail::StringBase<30> algorithm = "default";
	detail::StringBase<30> faction = "monster";
	AttributesValuesList attributes;
	std::vector<Item> equippedItems;
	std::vector<Variant> onDeath;
	Real score;
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

void removeLastComma(std::string &json);
OccupancyEnum occupancy(TileEnum tile);
bool isLevelBoss(uint32 level);
uint32 levelToBossIndex(uint32 level);
uint32 bossIndexToLevel(uint32 index);
Real isHorrorFloor(uint32 level);
Real makeAttrFactor(uint32 power, Real roll);
uint32 makeCost(Thing &sk, Real default_);
std::string exportVariant(const Variant &variant);

template<class... T>
constexpr bool always_false = false;

constexpr float H = 0.5;
constexpr uint32 Nothing = 0;
constexpr uint32 LevelSlash = 0;
constexpr uint32 LevelPierce = 7;
constexpr uint32 LevelRanged = 7;
constexpr uint32 LevelAoe = 11;
constexpr uint32 LevelKnockback = 11;
constexpr uint32 LevelFire = 16;
constexpr uint32 LevelMagic = 16;
constexpr uint32 LevelRequirements = 22;
constexpr uint32 LevelDuration = 22;
constexpr uint32 LevelSupport = 29;
constexpr uint32 LevelPoison = 37;
constexpr uint32 LevelEthereal = 42;
constexpr uint32 LevelGroundEffect = 46;
constexpr uint32 LevelStun = 56;
constexpr uint32 LevelSummoning = 67;
constexpr uint32 LevelElectric = 79;

template<class T>
struct Candidates : private Immovable
{
	Candidates(const Generate &generate) : generate(generate) {}

	const Generate &generate;
	std::optional<T> data;
	Real bestPenalty = Real::Infinity();
	Real slotMismatchPenalty = 0.5;
	Real randomness = 0.5;
#ifdef CAGE_DEBUG
	std::vector<std::pair<Real, T>> vec;
#endif // CAGE_DEBUG

	void add(Real magic, Real ranged, Real defensive, Real support, SlotEnum preferredSlot, const std::initializer_list<uint32> &requiredLevels, T value)
	{
		uint32 minLevel = std::max(requiredLevels);
		if (generate.ranged > 0.5 || ranged > 0.5)
			minLevel = max(minLevel, LevelRanged);
		if (generate.magic > 0.5 || magic > 0.5)
			minLevel = max(minLevel, LevelMagic);
		if (generate.support > 0.5 || support > 0.5)
			minLevel = max(minLevel, LevelSupport);
		const Real a = abs(generate.magic - magic) + abs(generate.ranged - ranged) + abs(generate.defensive - defensive) + abs(generate.support - support);
		const Real s = generate.slot == preferredSlot ? 0 : slotMismatchPenalty;
		const Real l = generate.level > minLevel ? 0 : 10;
		const Real p = a + s + l + randomChance() * randomness;
		if (p < bestPenalty)
		{
			bestPenalty = p;
			data = value;
		}
#ifdef CAGE_DEBUG
		vec.push_back({ p, value });
#endif // CAGE_DEBUG
	}

	void fallback(T value)
	{
		if (bestPenalty > 10)
		{
			bestPenalty = 10;
			data = value;
		}
	}

	T pick()
	{
		CAGE_ASSERT(data.has_value());
		CAGE_ASSERT(bestPenalty <= 10);
		return *data;
	}
};
