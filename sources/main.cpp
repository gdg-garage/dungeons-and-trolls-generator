#include "dnt.h"

#include <cage-core/ini.h>
#include <cage-core/logger.h>
#include <cage-core/string.h>
#include <cage-core/tasks.h>

Floor generateFloor(uint32 level, uint32 maxLevel);
void exportDungeon(PointerRange<const Floor> floors, const String &jsonPath, const String &htmlPath);

int main(int argc, const char *args[])
{
	Holder<Logger> log1 = newLogger();
	log1->format.bind<logFormatConsole>();
	log1->output.bind<logOutputStdErr>();
	try
	{
		Holder<Ini> cmd = newIni();
		cmd->parseCmd(argc, args);
		const uint32 s = cmd->cmdUint32('s', "start", 0);
		const uint32 e = cmd->cmdUint32('e', "end", s);
		const uint32 m = cmd->cmdUint32('m', "max", e);
		const String j = cmd->cmdString('j', "json", "dungeon.json");
		const String h = cmd->cmdString('h', "html", "dungeon.html");
		if (cmd->cmdBool('?', "help", false))
		{
			cmd->logHelp();
			return 0;
		}
		cmd->checkUnusedWithHelp();

		if (e < s || m < e || m == 0)
			CAGE_THROW_ERROR(Exception, "invalid input range parameters");

		std::vector<Floor> floors;
		floors.reserve(m);
		if (e - s + 1 > 2)
		{
			floors.resize(e - s + 1);
			struct Ctx
			{
				Floor *first = nullptr;
				uint32 s = 0;
				uint32 m = 0;
			} ctxVal, *ctx = &ctxVal;
			ctx->first = floors.data();
			ctx->s = s;
			ctx->m = m;
			tasksRunBlocking<Floor>("generate floors", Delegate<void(Floor &)>([ctx](Floor &f) -> void { f = generateFloor(ctx->s + (&f - ctx->first), ctx->m); }), floors);
		}
		else
		{
			for (uint32 l = s; l <= e; l++)
				floors.push_back(generateFloor(l, m));
		}
		exportDungeon(floors, j, h);
		return 0;
	}
	catch (...)
	{
		detail::logCurrentCaughtException();
	}
	return 1;
}
