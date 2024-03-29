#include <cstdio>

#include "dnt.h"

#include <cage-core/files.h>

namespace
{
	String uni(PointerRange<const char8_t> s)
	{
		String r;
		r.rawLength() = s.size() - 1;
		for (uint32 i = 0; i < r.length(); i++)
			r.rawData()[i] = s[i];
		return r;
	}

	String tileAscii(const Floor &f, uint32 x, uint32 y)
	{
		switch (f.tile(x, y))
		{
			case TileEnum::Empty:
				return " ";
			case TileEnum::Decoration:
				return uni(u8"\u2591");
			case TileEnum::Spawn:
				return "S";
			case TileEnum::Waypoint:
				return "O";
			case TileEnum::Stairs:
				return "#";
			case TileEnum::Door:
				return "H";
			case TileEnum::Chest:
				return "$";
			case TileEnum::Monster:
				return "@";
			case TileEnum::Wall:
				return uni(u8"\u2592");
			case TileEnum::Outside:
				return "~";
			default:
				return "?";
		}
	}

	const char *tileName(TileEnum tile)
	{
		switch (tile)
		{
			case TileEnum::Empty:
				return "empty";
			case TileEnum::Decoration:
				return "decoration";
			case TileEnum::Spawn:
				return "spawn";
			case TileEnum::Waypoint:
				return "waypoint";
			case TileEnum::Stairs:
				return "stairs";
			case TileEnum::Door:
				return "door";
			case TileEnum::Chest:
				return "chest";
			case TileEnum::Monster:
				return "monster";
			case TileEnum::Wall:
			case TileEnum::Outside: // treat outside same as walls
				return "wall";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown tile enum");
		}
	}

	std::string tileJson(Vec2i position, TileEnum type, const TileExtra &extra)
	{
		if (type == TileEnum::Empty)
			return {};

		std::string json;
		json += "{\n";
		json += (Stringizer() + "\"x\":" + position[0] + ",\n").value.c_str();
		json += (Stringizer() + "\"y\":" + position[1] + ",\n").value.c_str();
		json += (Stringizer() + "\"type\":\"" + tileName(type) + "\",\n").value.c_str();

		if (!extra.empty())
		{
			json += "\"data\":[\n";
			for (const auto &it : extra)
				json += exportVariant(it) + ",\n";
			removeLastComma(json);
			json += "],\n"; // data
		}

		removeLastComma(json);
		json += "}"; // /root
		return json;
	}

	void replaceAll(std::string &str, const std::string &from, const std::string &to)
	{
		size_t start = 0;
		while (true)
		{
			start = str.find(from, start);
			if (start == std::string::npos)
				break;
			str = str.replace(start, from.length(), to);
			start += to.length();
		}
	}

	uint32 countTiles(const Floor &f, TileEnum tile)
	{
		uint32 cnt = 0;
		for (auto it : f.tiles)
			cnt += it == tile;
		return cnt;
	}

	std::string tileHtml(const Floor &f, uint32 x, uint32 y, std::string json)
	{
		if (json.empty())
			return tileAscii(f, x, y).c_str();

		//replaceAll(json, "<", "&lt;");
		//replaceAll(json, ">", "&gt;");
		replaceAll(json, "\"", "&quot;");

		std::string r;
		r += "<span data-json=\"" + json + "\">";
		r += tileAscii(f, x, y).c_str();
		r += "</span>";
		return r;
	}

	// returns total and average score
	std::pair<Real, Real> totalScore(const Floor &f)
	{
		Real s = 0;
		uint32 c = 0;
		for (uint32 y = 0; y < f.height; y++)
		{
			for (uint32 x = 0; x < f.width; x++)
			{
				if (f.tile(x, y) != TileEnum::Monster)
					continue;
				for (const auto &v : f.extra(x, y))
				{
					std::visit(
						[&](const auto &arg)
						{
							using T = std::decay_t<decltype(arg)>;
							if constexpr (std::is_same_v<T, Monster>)
							{
								s += arg.score;
								c++;
							}
						},
						v);
				}
			}
		}
		return { s, s / c };
	}

	// returns total and average cost
	std::pair<Real, Real> totalItemCost(const Floor &f)
	{
		Real s = 0;
		uint32 c = 0;
		for (const TileExtra &t : f.extras)
		{
			for (const auto &v : t)
			{
				std::visit(
					[&](const auto &arg)
					{
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, Monster>)
						{
							for (const Item &i : arg.equippedItems)
							{
								s += i.goldCost;
								c++;
							}
						}
					},
					v);
			}
		}
		return { s, s / c };
	}

	struct Useless
	{
		uint32 attacks = 0;
		uint32 expensive = 0;
	};

	Useless countUselessMonsters(const Floor &f)
	{
		Useless res;
		for (const TileExtra &t : f.extras)
		{
			for (const auto &v : t)
			{
				std::visit(
					[&](const auto &arg)
					{
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, Monster>)
						{
							uint32 totalAttacks = 0;
							uint32 uselessAttacks = 0;
							uint32 expensiveAttacks = 0;
							const AttributesValuesList totalAttributes = monsterTotalAttributes(arg);
							for (const Item &it : arg.equippedItems)
							{
								for (const Skill &sk : it.skills)
								{
									if (sk.damageType != DamageTypeEnum::None)
									{
										totalAttacks++;
										if (attributesSum(sk.damageAmount, totalAttributes) < 1.0)
											uselessAttacks++;
										if (!attributesCompare(sk.cost, totalAttributes))
											expensiveAttacks++;
									}
								}
							}
							if (totalAttacks > 0 && uselessAttacks == totalAttacks)
								res.attacks++;
							if (totalAttacks > 0 && expensiveAttacks == totalAttacks)
								res.expensive++;
						}
					},
					v);
			}
		}
		return res;
	}

	void flush(Holder<File> &f, const String &p)
	{
		if (p.empty())
			return;
		f->seek(0);
		if (p == "-")
		{
			const auto buff = f->readAll();
			fwrite(buff.data(), buff.size(), 1, stdout);
		}
		else
		{
			Holder<File> o = writeFile(p);
			o->write(f->readAll());
			o->close();
		}
	}
}

FloorExport exportFloor(const Floor &floor)
{
	FloorExport result;
	result.html += "<div class=\"floor\">\n";
	result.json += "{\n";
	result.json += (Stringizer() + "\"level\":" + floor.level + ",\n").value.c_str();
	result.json += (Stringizer() + "\"width\":" + floor.width + ",\n").value.c_str();
	result.json += (Stringizer() + "\"height\":" + floor.height + ",\n").value.c_str();
	result.json += (Stringizer() + "\"horror\":" + isHorrorFloor(floor.level) + ",\n").value.c_str();
	result.json += "\"tiles\":[\n";
	for (uint32 y = 0; y < floor.height; y++)
	{
		for (uint32 x = 0; x < floor.width; x++)
		{
			const std::string j = tileJson(Vec2i(x, y), floor.tile(x, y), floor.extra(x, y));
			result.html += tileHtml(floor, x, y, j);
			if (!j.empty())
			{
				result.json += j;
				result.json += ",\n";
			}
		}
		result.html += "\n";
	}
	removeLastComma(result.json);
	result.json += "]\n"; // /tiles
	result.json += "}\n"; // /root
	result.html += "</div>\n";
	return result;
}

void exportDungeon(PointerRange<const Floor> floors, const String &jsonPath, const String &htmlPath)
{
	Holder<File> html = newFileBuffer();
	html->writeLine("<!DOCTYPE html>");
	html->writeLine("<html>");
	html->writeLine("<head>");
	html->writeLine("<title>D&T dungeon</title>");
	html->write(R"html(
<style>
.floor {
	font-family: monospace;
	white-space: pre;
	cursor: default;
	transform: translateX(50%) scaleX(2);
}
.floor span {
	cursor: crosshair;
}
</style>
)html");
	html->writeLine("</head>");
	html->writeLine("<body>");

	Holder<File> json = newFileBuffer();
	json->writeLine("{");
	json->writeLine("\"floors\":[");

	bool firstlevel = true;
	for (const Floor &f : floors)
	{
		const FloorExport e = exportFloor(f);
		html->writeLine(Stringizer() + "<h2>Floor " + f.level + "</h2>");
		html->write(e.html);
		if (isLevelBoss(f.level))
			html->writeLine(Stringizer() + "boss level: " + levelToBossIndex(f.level) + "<br>");
		html->writeLine(Stringizer() + "horror: " + isHorrorFloor(f.level) + "<br>");
		const auto score = totalScore(f);
		const auto itemsCost = totalItemCost(f);
		html->writeLine(Stringizer() + "monsters: " + countTiles(f, TileEnum::Monster) + ", total score: " + score.first + ", average score: " + score.second + ", average equipped item value: " + itemsCost.second + "<br>");
#ifdef CAGE_DEBUG
		const auto useless = countUselessMonsters(f);
		html->writeLine(Stringizer() + "useless attacks: " + useless.attacks + ", expensive attacks: " + useless.expensive + "<br>");
#endif // CAGE_DEBUG
		html->writeLine("<hr>");

		if (firstlevel)
			firstlevel = false;
		else
			json->writeLine(",\n");
		json->write(e.json);
	}

	html->write(R"html(
<dialog id="dialog">
<pre>
<code id="code">
</code>
</pre>
</dialog>
)html");
	html->writeLine("</body>");
	html->write(R"html(
<script>
document.body.addEventListener("click", function(event) {
	let d = event.target.getAttribute("data-json");
	if (!d)
		return;
	document.getElementById("code").innerHTML = JSON.stringify(JSON.parse(d), null, 10);
	document.getElementById("dialog").showModal();
});
</script>
)html");
	html->close();

	json->writeLine("]"); // /floors
	json->writeLine("}"); // /root
	json->close();

	flush(json, jsonPath);
	flush(html, htmlPath);
}
