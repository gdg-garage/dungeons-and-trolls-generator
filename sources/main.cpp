#include "dnt.h"
#include <cage-core/logger.h>
#include <cage-core/string.h>

namespace
{
	void generateSingle(uint32 level, uint32 maxLevel)
	{
		const Floor f = generateFloor(level, maxLevel);
		exportDungeon(PointerRange(&f, &f + 1));
	}

	void generateAll()
	{
		constexpr uint32 maxLevel = 20;
		std::vector<Floor> floors;
		floors.reserve(maxLevel);
		for (uint32 l = 0; l <= maxLevel; l++)
			floors.push_back(generateFloor(l, maxLevel));
		exportDungeon(floors);
		exportExamples(maxLevel);
	}
}

int main(int argc, const char *args[])
{
	Holder<Logger> log1 = newLogger();
	log1->format.bind<logFormatConsole>();
	log1->output.bind<logOutputStdOut>();
	try
	{
		if (argc == 3)
		{
			const uint32 l = toUint32(String(args[1]));
			const uint32 t = toUint32(String(args[2]));
			generateSingle(l, t);
		}
		else if (argc == 1)
		{
			generateAll();
		}
		else
		{
			CAGE_THROW_ERROR(Exception, "invalid number of cmd parameters");
		}
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}
