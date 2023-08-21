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

	String connectedWall(const Floor &f, uint32 x, uint32 y)
	{
		uint32 neighbors = 0;
		if (x == 0 || f.tile(x - 1, y) == TileEnum::Wall || f.tile(x - 1, y) == TileEnum::Outside)
			neighbors += 1; // left
		if (y == 0 || f.tile(x, y - 1) == TileEnum::Wall || f.tile(x, y - 1) == TileEnum::Outside)
			neighbors += 2; // top
		if (x + 1 == f.width || f.tile(x + 1, y) == TileEnum::Wall || f.tile(x + 1, y) == TileEnum::Outside)
			neighbors += 4; // right
		if (y + 1 == f.height || f.tile(x, y + 1) == TileEnum::Wall || f.tile(x, y + 1) == TileEnum::Outside)
			neighbors += 8; // bottom

		switch (neighbors)
		{
			case 0:
				return "X";
			case 1:
				return uni(u8"\u2500");
			case 2:
				return uni(u8"\u2502");
			case 3:
				return uni(u8"\u2518");
			case 4:
				return uni(u8"\u2500");
			case 5:
				return uni(u8"\u2500");
			case 6:
				return uni(u8"\u2514");
			case 7:
				return uni(u8"\u2534");
			case 8:
				return uni(u8"\u2502");
			case 9:
				return uni(u8"\u2510");
			case 10:
				return uni(u8"\u2502");
			case 11:
				return uni(u8"\u2524");
			case 12:
				return uni(u8"\u250C");
			case 13:
				return uni(u8"\u252C");
			case 14:
				return uni(u8"\u251C");
			case 15:
				return uni(u8"\u253C");
			default:
				CAGE_THROW_CRITICAL(Exception, "invalid neighbors count for connecting walls");
		}
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
				return connectedWall(f, x, y);
			case TileEnum::Outside:
				return "~";
				return uni(u8"\u2593");
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
				return "wall";
			case TileEnum::Outside:
				return "outside";
			default:
				CAGE_THROW_CRITICAL(Exception, "unknown tile enum");
		}
	}

	std::string tileJson(Vec2i position, TileEnum type, const TileExtra &extra)
	{
		if (type == TileEnum::Empty || type == TileEnum::Outside)
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
}

FloorExport exportFloor(const Floor &floor)
{
	FloorExport result;
	result.html += "<div class=\"floor\">\n";
	result.json += "{\n";
	result.json += (Stringizer() + "\"level\":" + floor.level + ",\n").value.c_str();
	result.json += (Stringizer() + "\"width\":" + floor.width + ",\n").value.c_str();
	result.json += (Stringizer() + "\"height\":" + floor.height + ",\n").value.c_str();
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
	Holder<File> html = htmlPath.empty() ? newFileBuffer() : writeFile(htmlPath);
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

	Holder<File> json = jsonPath.empty() ? newFileBuffer() : writeFile(jsonPath);
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
		html->writeLine(Stringizer() + "monsters: " + countTiles(f, TileEnum::Monster) + "<br>");
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
}
