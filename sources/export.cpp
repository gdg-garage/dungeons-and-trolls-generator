#include <cage-core/files.h>
#include "dnt.h"

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
		case 0: return uni(u8"\u2588");
		case 1: return uni(u8"\u2500");
		case 2: return uni(u8"\u2502");
		case 3: return uni(u8"\u2518");
		case 4: return uni(u8"\u2500");
		case 5: return uni(u8"\u2500");
		case 6: return uni(u8"\u2514");
		case 7: return uni(u8"\u2534");
		case 8: return uni(u8"\u2502");
		case 9: return uni(u8"\u2510");
		case 10: return uni(u8"\u2502");
		case 11: return uni(u8"\u2524");
		case 12: return uni(u8"\u250C");
		case 13: return uni(u8"\u252C");
		case 14: return uni(u8"\u251C");
		case 15: return uni(u8"\u253C");
		default: CAGE_THROW_CRITICAL(Exception, "invalid neighbors count for connecting walls");
		}
	}

	String convertToAscii(const Floor &f, uint32 y)
	{
		String res;
		for (uint32 x = 0; x < f.width; x++)
		{
			switch (f.tile(x, y))
			{
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
			} break;
			case TileEnum::Outside: res += uni(u8"\u2588"); break;
			case TileEnum::Waypoint: res += "F"; break;
			case TileEnum::Stairs: res += "#"; break;
			case TileEnum::Door: res += "D"; break;
			case TileEnum::Curtain: res += uni(u8"\u2592"); break;
			case TileEnum::Chest: res += "C"; break;
			case TileEnum::Item: res += "i"; break;
			case TileEnum::Key: res += "k"; break;
			case TileEnum::Monster: res += "@"; break;
			case TileEnum::Empty: res += " "; break;
			case TileEnum::Rug: res += uni(u8"\u2591"); break;
			default: res += "?"; break;
			}
		}
		return res;
	}
}

void exportDungeon()
{
	Holder<File> html = writeFile("dungeon.html");
	html->writeLine("<html>");
	html->writeLine("<head>");
	html->writeLine("<title>D&T dungeon</title>");
	html->writeLine("</head>");
	html->writeLine("<body>");

	Holder<File> json = writeFile("dungeon.json");
	json->writeLine("{");
	json->writeLine("\"floors\":{");

	uint32 level = 0;
	for (const Floor &f : dungeon.floors)
	{
		html->writeLine(Stringizer() + "<h2>Floor " + level + "</h2>");
		html->writeLine("<div><pre>");
		json->writeLine(Stringizer() + "\"" + level + "\":{");
		json->writeLine("\"map\":[");

		for (uint32 y = 0; y < f.height; y++)
		{
			const String line = convertToAscii(f, y);
			html->writeLine(line);
			json->writeLine(Stringizer() + "\"" + line + "\"" + (y + 1 < f.height ? "," : ""));
		}

		json->writeLine("]"); // /map
		json->writeLine(Stringizer() + "}" + (level + 1 < dungeon.floors.size() ? "," : "")); // /level
		html->writeLine("</pre></div>");

		level++;
	}

	json->writeLine("}"); // /floors
	json->writeLine("}"); // /root
	json->close();

	html->writeLine("</body>");
	html->close();
}
