#include <cage-core/logger.h>
#include "dnt.h"

void generateDungeon();
void exportDungeon();

int main(int argc, const char *args[])
{
	try
	{
		Holder<Logger> log1 = newLogger();
		log1->format.bind<logFormatConsole>();
		log1->output.bind<logOutputStdOut>();
		generateDungeon();
		exportDungeon();
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}
