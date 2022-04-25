#include "subtitans.h"
#include <VersionHelpers.h>
#include "steampatcher.h"
#include "steampatchedpatcher.h"
#include "gogpatcher.h"
#include "demopatcher.h"

static Logger* g_Logger = nullptr;
Logger* GetLogger()
{
	if (!g_Logger)
	{
		g_Logger = new Logger("subtitans.log");
		g_Logger->Clear();
	}

	return g_Logger;
}

static Configuration* g_Configuration = nullptr;
Configuration* GetConfiguration()
{
	if (!g_Configuration)
	{
		g_Configuration = new Configuration(L"subtitans.ini");
		GetLogger()->Informational("Initialized configuration\n");
	}

	return g_Configuration;
}

namespace Global
{
	int32_t InternalWidth = 0;
	int32_t InternalHeight = 0;
	int32_t MonitorWidth = 0;
	int32_t MonitorHeight = 0;
	int32_t RenderWidth = 0;
	int32_t RenderHeight = 0;

	int32_t BitsPerPixel = 0;

	bool VideoWorkaround = false;

	HWND GameWindow = nullptr;

	HANDLE RenderEvent = nullptr;
	HANDLE VerticalBlankEvent = nullptr;

	IRenderer* Backend = nullptr;

	bool RetroShader = false;
	std::atomic<bool> ImGuiEnabled = false;

	_MouseInformation MouseInformation;
	_KeyboardInformation KeyboardInformation;

	void Init()
	{
		InternalWidth = 800;
		InternalHeight = 600;
		MonitorWidth = GetSystemMetrics(SM_CXSCREEN);
		MonitorHeight = GetSystemMetrics(SM_CYSCREEN);
		RenderWidth = MonitorWidth;
		RenderHeight = MonitorHeight;

		BitsPerPixel = 8;

		RenderEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		VerticalBlankEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		memset(&MouseInformation, 0, sizeof(_MouseInformation));
		memset(&KeyboardInformation, 0, sizeof(_KeyboardInformation));
	}
}

const char* GetApplicationLanguage()
{
	const std::wstring STStringDll = L"ST_String.dll";
	if (File::Exists(STStringDll.c_str()))
	{
		uint32_t checksum = File::CalculateChecksum(STStringDll.c_str());
		switch (checksum)
		{
			case Shared::ST_LANGUAGE_ENGLISH_UNPATCHED:
			case Shared::ST_LANGUAGE_ENGLISH_PATCHED:
			case Shared::ST_LANGUAGE_ENGLISH_DEMO:
				return "English";
			default:
				return "Unknown language";
		}
	}
	else
	{
		return "ST_String.dll not found";
	}
}

Patcher* CreateVersionSpecificGamePatcher(unsigned long gameVersion)
{
	Patcher* patcher = nullptr;

	GetLogger()->Informational("Detected Version: ");
	switch (gameVersion)
	{
		case Shared::ST_GAMEVERSION_RETAIL_UNPATCHED:
			patcher = new SteamPatcher();
			GetLogger()->Informational("Retail 1.0 - %s\n", GetApplicationLanguage());
			GetLogger()->Warning("DEPRECATED!!! Consider updating to 1.1\n");
			MessageBox(NULL, L"This version of Submarine Titans is outdated, please update to v1.1.", L"SubTitans Patch", MB_ICONWARNING);
			break;
		case Shared::ST_GAMEVERSION_RETAIL_PATCHED:
			patcher = new SteamPatchedPatcher();
			GetLogger()->Informational("Retail 1.1 - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_GOG_MODIFIED:
			patcher = new GOGPatcher();
			GetLogger()->Informational("GOG 1.1 - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_DEMO:
			patcher = new DemoPatcher();
			GetLogger()->Informational("Demo - %s\n", GetApplicationLanguage());
			break;
		default:
			GetLogger()->Informational("Unknown\n");
			GetLogger()->Critical("Incompatible application\n");
			MessageBox(NULL, L"Incompatible application", L"SubTitans Patch", MB_ICONERROR);
			break;
	}

	return patcher;
}

void LogOperatingSystem()
{
	GetLogger()->Informational("Operating System: ");

	if (IsWindowsXPOrGreater() && !IsWindowsVistaOrGreater())
		GetLogger()->Informational("Windows XP (unsupported)");
	else if (IsWindowsVistaOrGreater() && !IsWindows7OrGreater())
		GetLogger()->Informational("Windows Vista (unsupported)");
	else if (IsWindows7OrGreater() && !IsWindows8OrGreater())
		GetLogger()->Informational("Windows 7 (supported)");
	// BUG: ST.exe has no manifest targeting Windows 8.1 or 10 so it will always report as Windows 8
	else if (IsWindows8OrGreater())
		GetLogger()->Informational("Windows 8 or newer (supported)");
	else
		GetLogger()->Informational("Unknown (unsupported)");

	HMODULE hNTDLL = GetModuleHandle(L"ntdll.dll");
	if (hNTDLL && GetProcAddress(hNTDLL, "wine_get_version") != NULL)
		GetLogger()->Informational(" - Running under Wine (supported)");

	GetLogger()->Informational("\n");
}

void LogHardwareInformation()
{
	// Architecture
	GetLogger()->Informational("CPU Architecture: ");

	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);
	switch (systemInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64:
			GetLogger()->Informational("x64\n");
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			GetLogger()->Informational("x32\n");
			break;
		default:
			GetLogger()->Informational("Unknown (%i)\n", systemInfo.wProcessorArchitecture);
			break;
	}

	// Core count
	uint32_t coreCount = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	GetLogger()->Informational("Total Processors: %i Cores\n", coreCount);

	// Memory
	MEMORYSTATUSEX memoryStatus;
	memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&memoryStatus) == 0)
	{
		GetLogger()->Warning("Failed to retrieve memory status\n");
	}
	else
	{
		GetLogger()->Informational("Installed Memory: %I64dMiB\n", memoryStatus.ullTotalPhys / 1024 / 1024);
		GetLogger()->Informational("Available Memory: %I64dMiB\n", memoryStatus.ullAvailPhys / 1024 / 1024);
		GetLogger()->Informational("Committed Memory: %I64dMiB (%i%%)\n", (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) / 1024 / 1024, memoryStatus.dwMemoryLoad);
	}
}

uint32_t GetTimerResolution()
{
	uint32_t resolution = 0;

	TIMECAPS timeCaps;
	if (timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)) == TIMERR_NOERROR)
		resolution = min(max(timeCaps.wPeriodMin, 1), timeCaps.wPeriodMax);
	else
		GetLogger()->Error("Failed to get timer resolution\n");

	GetLogger()->Informational("Timer Resolution: %ims\n", resolution);

	return resolution;
}

static Patcher* g_GamePatcher = nullptr;
static UINT g_TimerResolution = 0;

#pragma comment(linker, "/EXPORT:InitializeLibrary=_InitializeLibrary@4")
extern "C" void __stdcall InitializeLibrary(unsigned long gameVersion)
{
	Global::Init();

	GetLogger()->Informational("Compilation Date: %s\n", __TIMESTAMP__);

	g_GamePatcher = CreateVersionSpecificGamePatcher(gameVersion);
	if(!g_GamePatcher)
		ExitProcess(-1);

	LogOperatingSystem();
	LogHardwareInformation();

	uint32_t timerResolution = GetTimerResolution();

	GetLogger()->Informational("\nStarting SubTitans\n");
	if (!g_GamePatcher->Initialize())
	{
		GetLogger()->Warning("Game restart required because GamePatcher::Initialize returned false\n");
		MessageBox(NULL, L"Please restart the game.", L"SubTitans Patch", MB_ICONINFORMATION);
		ExitProcess(0);
	}

	GetLogger()->Informational("Configuring patches\n");
	g_GamePatcher->Configure();

	GetLogger()->Informational("Applying patches\n");
	if (!g_GamePatcher->Apply())
	{
		GetLogger()->Critical("Failed to apply patches\n");
		ExitProcess(-1);
	}

	// Set timer resolution
	g_TimerResolution = timerResolution;
	if(g_TimerResolution != 0)
		timeBeginPeriod(g_TimerResolution);
}

#pragma comment(linker, "/EXPORT:ReleaseLibrary=_ReleaseLibrary@0")
extern "C" void __stdcall ReleaseLibrary()
{
	GetLogger()->Informational("Shutting down\n");
	ClipCursor(nullptr); // Free the cursor

	if (g_TimerResolution != 0)
		timeEndPeriod(g_TimerResolution);

	if (g_GamePatcher)
		delete g_GamePatcher;

	if (g_Configuration)
		delete g_Configuration;

	if (g_Logger)
		delete g_Logger;
}

BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	return TRUE;
}