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
	detail::StringBase<30> type;
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
	Real goldCost = 0;

	Real addPower(Real weight);
	Real addPower(Real roll, Real weight);
	Real addPower(Real weight, AffixEnum affix, const std::string &name);
	Real subtractPower(Real weight, AffixEnum affix, const std::string &name);
	Real addPower(Real roll, Real weight, AffixEnum affix, const std::string &name);
	void addPower(const Thing &other, Real weight);
	void addAffix(Real relevance, AffixEnum affix, const std::string &name);
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
	Scalar,
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
	Item,
};

using AttributesValuesList = std::map<AttributeEnum, sint32>;
using AttributesEquationFactors = std::map<AttributeEnum, Real>;
using SkillAttributes = std::map<AttributeEnum, AttributesEquationFactors>;
using SkillFlag = StringPointer;

struct SkillEffects
{
	SkillAttributes attributes;
	std::vector<SkillFlag> flags;
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
Real makeAttrFactor(Thing &sk, const Generate &generate, Real weight);
Real makeAttrFactor(Thing &sk, const Generate &generate, Real weight, const std::string &affixName, AffixEnum affixPos = AffixEnum::Prefix);
uint32 makeCost(Thing &sk, const Generate &generate, Real default_);

Skill generateSkill(const Generate &generate);
Item generateItem(const Generate &generate);
Item generateSprayCan();
Monster generateMonster(const Generate &generate);
Monster generateMinion(const Generate &generate);
Monster generateChest(const Generate &generate);
Monster generateFloorBoss(uint32 level);
Holder<PointerRange<Monster>> generateAntiHeroes();
Monster generateButcher(uint32 level);
Monster generateWitch(uint32 level);
Monster generateTemplar(uint32 level);
Monster generateZergling(uint32 level);
Monster generateHydra(uint32 level);
Monster generateSatyr(uint32 level);
Monster generateElemental(uint32 level);
Monster generateVandal();
Floor generateFloor(uint32 level, uint32 maxLevel);

std::string exportVariant(const Variant &variant);
FloorExport exportFloor(const Floor &floor);
void exportDungeon(PointerRange<const Floor> floors, const String &jsonPath, const String &htmlPath);

template<class... T>
constexpr bool always_false = false;

constexpr SkillFlag SkillAlone = "alone"; // requires that the caster is alone (no other creature (player or monster) are visible in 10 range)
constexpr SkillFlag SkillNoLineOfSight = "noLineOfSight"; // does not require the target position be visible from the caster position
constexpr SkillFlag SkillAllowSelf = "allowSelf"; // allows skills that target a character to target oneself
constexpr SkillFlag SkillMoves = "moves"; // moves the caster to the target position, or the target to the caster position
constexpr SkillFlag SkillKnockback = "knockback"; // moves the caster/target one tile away from the other
constexpr SkillFlag SkillStun = "stun"; // prevents the caster/target from performing any actions for one tick, and grants immunity to stun for the following tick
constexpr SkillFlag SkillGroundEffect = "groundEffect"; // creates ground effect at caster/target position, which applies the effects of the skill
constexpr SkillFlag SkillPassive = "passive"; // the effects of the skill are automatically applied every tick, assuming the cost can be paid; multiple passive skills are allowed

constexpr float H = 0.5;
constexpr uint32 Nothing = 0;
constexpr uint32 LevelSlash = 0;
constexpr uint32 LevelPierce = 7;
constexpr uint32 LevelRanged = 7;
constexpr uint32 LevelAoe = 11;
constexpr uint32 LevelKnockback = 11;
constexpr uint32 LevelFire = 16;
constexpr uint32 LevelMagic = 16;
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
	Real slotMismatchPenalty = 1;
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
		const Real p = a + s + l + randomChance() * 0.3;
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
