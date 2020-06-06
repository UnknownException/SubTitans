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

#pragma comment(linker, "/EXPORT:InitializeLibrary=_InitializeLibrary@4")
extern "C" void __stdcall InitializeLibrary(unsigned long gameVersion)
{
	switch (gameVersion)
	{
		case Shared::ST_GAMEVERSION_STEAM:
			g_GamePatcher = new SteamPatcher();
			GetLogger()->Informational("Version: Steam (1.0)\n");
			break;
//		case Shared::ST_GAMEVERSION_STEAM_PATCHED:
//			g_GamePatcher = new SteamPatchedPatcher();
//			g_Logger->Informational("Version: Steam (1.1)\n");
//			break;			
		case Shared::ST_GAMEVERSION_GOG:
			g_GamePatcher = new GOGPatcher();
			GetLogger()->Informational("Version: GOG (1.1)\n");
			break;
		default:
			GetLogger()->Critical("Non compatible application\n");
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
}

#pragma comment(linker, "/EXPORT:ReleaseLibrary=_ReleaseLibrary@0")
extern "C" void __stdcall ReleaseLibrary()
{
	GetLogger()->Informational("Shutting down\n");

	if (g_GamePatcher)
		delete g_GamePatcher;
}

BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	return TRUE;
}