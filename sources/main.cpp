#include "dnt.h"
#include <cage-core/logger.h>
#include <cage-core/string.h>

int main(int argc, const char *args[])
{
	Holder<Logger> log1 = newLogger();
	log1->format.bind<logFormatConsole>();
	log1->output.bind<logOutputStdOut>();
	try
	{
		if (argc <= 1)
		{
			std::vector<Floor> floors;
			for (uint32 l = 0; l < 100; l++)
				floors.push_back(generateFloor(l));
			exportDungeon(floors);
		}
		else
		{
			const uint32 l = toUint32(String(args[1]));
			Floor f = generateFloor(l);
			exportDungeon(PointerRange(&f, &f + 1));
		}
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}
