
#include <windows.h>

#include "Debug.h"

bool DebugUtils::consoleOpened = false;

void DebugUtils::openConsole() {
#ifdef USE_CONSOLE
	if (consoleOpened) return;
	consoleOpened = true;

	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
}

void DebugUtils::closeConsole() {
#ifdef USE_CONSOLE
	if (!consoleOpened) return;
	consoleOpened = false;

	FreeConsole();
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
#endif
}

void DebugUtils::startTimer() {
	ticks = (double)GetTickCount();
}

void DebugUtils::endTimer() {
	auto delta = (double)GetTickCount() - ticks;
	LOG("Process time:" << delta << "ms");
}

double DebugUtils::ticks;

