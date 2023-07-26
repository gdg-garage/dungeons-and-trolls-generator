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

	String connectedWall(uint32 neighbors)
	{
		switch (neighbors)
		{
			case 0:
				return uni(u8"\u2588");
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

	String convertToAscii(const Floor &f, uint32 y)
	{
		String res;
		for (uint32 x = 0; x < f.width; x++)
		{
			switch (f.tile(x, y))
			{
				case TileEnum::Empty:
					res += " ";
					break;
				case TileEnum::Decoration:
					res += uni(u8"\u2591");
					break;
				case TileEnum::Spawn:
					res += "+";
					break;
				case TileEnum::Waypoint:
					res += "O";
					break;
				case TileEnum::Stairs:
					res += "#";
					break;
				case TileEnum::Door:
					res += "H";
					break;
				case TileEnum::Chest:
					res += "X";
					break;
				case TileEnum::Monster:
					res += "@";
					break;
				case TileEnum::Wall:
				{
					uint32 neighbors = 0;
					if (x > 0 && f.tile(x - 1, y) == TileEnum::Wall)
						neighbors += 1; // left
					if (y > 0 && f.tile(x, y - 1) == TileEnum::Wall)
						neighbors += 2; // top
					if (x + 1 < f.width && f.tile(x + 1, y) == TileEnum::Wall)
						neighbors += 4; // right
					if (y + 1 < f.height && f.tile(x, y + 1) == TileEnum::Wall)
						neighbors += 8; // bottom
					res += connectedWall(neighbors);
					break;
				}
				case TileEnum::Outside:
					res += uni(u8"\u2588");
					break;
				default:
					res += "?";
					break;
			}
		}
		return res;
	}

	uint32 countTiles(const Floor &f, TileEnum tile)
	{
		uint32 cnt = 0;
		for (auto it : f.tiles)
			cnt += it == tile;
		return cnt;
	}
}

Export exportFloor(const Floor &floor)
{
	Export result;

	result.html += "<div><pre>\n";
	for (uint32 y = 0; y < floor.height; y++)
	{
		const String line = convertToAscii(floor, y);
		result.html += line.c_str();
		result.html += "\n";
	}
	result.html += "</pre></div>\n";

	result.json += "{\n";
	result.json += (Stringizer() + "\"level\":" + floor.level + ",\n").value.c_str();
	result.json += (Stringizer() + "\"width\":" + floor.width + ",\n").value.c_str();
	result.json += (Stringizer() + "\"height\":" + floor.height + ",\n").value.c_str();
	result.json += "\"tiles\":{\n";
	for (uint32 y = 0; y < floor.height; y++)
	{
		for (uint32 x = 0; x < floor.width; x++)
		{
			if (!floor.extra(x, y).empty())
			{
				result.json += (Stringizer() + "\"" + x + "x" + y + "\":").value.c_str();
				result.json += floor.extra(x, y);
				result.json += ",\n";
			}
		}
	}
	result.json += "\"_dummy\":0\n"; // /tiles
	result.json += "}\n"; // /tiles
	result.json += "}\n"; // /root

	return result;
}

void exportDungeon(PointerRange<const Floor> floors)
{
	Holder<File> html = writeFile("dungeon.html");
	html->writeLine("<html>");
	html->writeLine("<head>");
	html->writeLine("<title>D&T dungeon</title>");
	html->writeLine("</head>");
	html->writeLine("<body>");

	Holder<File> json = writeFile("dungeon.json");
	json->writeLine("{");
	json->writeLine("\"floors\":[");

	for (const Floor &f : floors)
	{
		const Export e = exportFloor(f);
		html->writeLine(Stringizer() + "<h2>Floor " + f.level + "</h2>");
		html->write(e.html);
		if (isLevelBoss(f.level))
			html->writeLine(Stringizer() + "boss level: " + levelToBossIndex(f.level) + "<br>");
		html->writeLine(Stringizer() + "monsters: " + countTiles(f, TileEnum::Monster) + "<br>");
		html->writeLine("<hr>");
		json->write(e.json);
	}

	html->writeLine("</body>");
	html->close();

	json->writeLine("]"); // /floors
	json->writeLine("}"); // /root
	json->close();
}
