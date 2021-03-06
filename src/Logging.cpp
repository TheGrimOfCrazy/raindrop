#include <stdarg.h>
#include <stdio.h>
#include <fstream>

#include "Global.h"
#include "Logging.h"


#ifdef DEBUG
static void Log::DebugPrintf(GString Format, ...)
{

}
#endif

void Log::Printf(GString Format, ...)
{
	char Buffer[2048];
	va_list vl;
	va_start(vl,Format);
	vsnprintf(Buffer, 2048, Format.c_str(), vl);
	va_end(vl);
	wprintf(L"%ls", Utility::Widen(Buffer).c_str());
}

void Log::Logf(GString Format, ...)
{
	static std::fstream logf ("log.txt", std::ios::out);
	char Buffer[2048];
	va_list vl;
	va_start(vl,Format);
	vsnprintf(Buffer, 2048, Format.c_str(), vl);
	va_end(vl);
	logf << Buffer;
	logf.flush();
}
