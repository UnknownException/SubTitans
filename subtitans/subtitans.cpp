#include "subtitans.h"
#include "steampatcher.h"
#include "steampatchedpatcher.h"
#include "gogpatcher.h"

static Logger* g_Logger = nullptr;

Logger* GetLogger()
{
	if (!g_Logger)
	{
		g_Logger = new Logger("subtitans.log");
		g_Logger->Clear();
		g_Logger->Informational("Starting SubTitans\nBuild %s\n", __TIMESTAMP__);
	}

	return g_Logger;
}

static Patcher* g_GamePatcher = nullptr;
static UINT g_TimerResolution = 0;

#pragma comment(linker, "/EXPORT:InitializeLibrary=_InitializeLibrary@4")
extern "C" void __stdcall InitializeLibrary(unsigned long gameVersion)
{
	switch (gameVersion)
	{
		case Shared::ST_GAMEVERSION_RETAIL_UNPATCHED:
			g_GamePatcher = new SteamPatcher();
			GetLogger()->Informational("Version: Retail (1.0)\n");
			GetLogger()->Warning("DEPRECATED!!! Consider updating to 1.1\n");
			break;
		case Shared::ST_GAMEVERSION_RETAIL_PATCHED:
			g_GamePatcher = new SteamPatchedPatcher();
			GetLogger()->Informational("Version: Retail (1.1)\n");
			break;			
		case Shared::ST_GAMEVERSION_GOG_MODIFIED:
			g_GamePatcher = new GOGPatcher();
			GetLogger()->Informational("Version: GOG (1.1)\n");
			break;
		default:
			GetLogger()->Critical("Incompatible application\n");
			MessageBox(NULL, L"Incompatible application", L"subtitans.dll", MB_ICONERROR);
			ExitProcess(-1);
			break;
	}

	GetLogger()->Informational("Initializing patch\n");
	if (!g_GamePatcher->Initialize())
	{
		GetLogger()->Warning("Game restart required due to GamePatcher::Initialize returning false\n");
		MessageBox(NULL, L"Please restart the game.", L"Information", MB_ICONINFORMATION);
		ExitProcess(0);
	}

	GetLogger()->Informational("Configuring game patches\n");
	g_GamePatcher->Configure();

	GetLogger()->Informational("Applying patches\n");
	if (!g_GamePatcher->Apply())
	{
		GetLogger()->Critical("Failed to apply patches\n");
		ExitProcess(-1);
	}

	TIMECAPS timeCaps;
	if (timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		GetLogger()->Error("Failed to get timer device resolution\n");
	else
		g_TimerResolution = min(max(timeCaps.wPeriodMin, 1), timeCaps.wPeriodMax);

	if (g_TimerResolution != 0)
		timeBeginPeriod(g_TimerResolution);
}

#pragma comment(linker, "/EXPORT:ReleaseLibrary=_ReleaseLibrary@0")
extern "C" void __stdcall ReleaseLibrary()
{
	GetLogger()->Informational("Shutting down\n");

	if (g_TimerResolution != 0)
		timeEndPeriod(g_TimerResolution);

	if (g_GamePatcher)
		delete g_GamePatcher;

	if (g_Logger)
		delete g_Logger;
}

BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	return TRUE;
}