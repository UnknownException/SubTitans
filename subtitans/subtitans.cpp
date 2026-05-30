#include "subtitans.h"
#include "steampatcher.h"
#include "steampatchedpatcher.h"
#include "gogpatcher.h"
#include "demopatcher.h"
#include "techdemopatcher.h"
#include "pescalpel.h"

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

static void StackTrace()
{
	uint32_t* ebpRegister = nullptr;
	__asm mov [ebpRegister], ebp;

	ULONG_PTR stackHighLimit = 0;
	ULONG_PTR stackLowLimit = 0;

	GetCurrentThreadStackLimits(&stackLowLimit, &stackHighLimit);

	uint32_t returnAddress = 0;
	int depth = 0;

	std::vector<std::pair<std::string, std::string>> modulePaths;

	GetLogger()->Informational("Stack trace:\n");

	while (ebpRegister != nullptr)
	{
		if ((uint32_t)ebpRegister >= (uint32_t)stackHighLimit || (uint32_t)ebpRegister < (uint32_t)stackLowLimit)
		{
			GetLogger()->Informational("Address out of range!\n");
			break;
		}

		if ((uint32_t)ebpRegister % 4 != 0)
		{
			GetLogger()->Informational("Invalid address\n");
			break;
		}

		returnAddress = *(ebpRegister + 1); // ebp + 0x04
		if (returnAddress == 0)
			break;

		HMODULE hMod = nullptr;
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)returnAddress, &hMod))
		{
			char modulePath[_MAX_PATH];
			char name[_MAX_FNAME];
			char ext[_MAX_EXT];

			if (GetModuleFileNameA(hMod, modulePath, _MAX_PATH) != NULL
				&& _splitpath_s(modulePath, nullptr, 0, nullptr, 0, name, _MAX_FNAME, ext, _MAX_EXT) == 0)
			{
				modulePaths.push_back(std::make_pair(std::string(name) + ext, modulePath));
				GetLogger()->Informational("[%i] Absolute 0x%04X Relative 0x%04X (%s%s)\n", depth++, returnAddress, returnAddress - (uint32_t)hMod, name, ext);
			}
			else
			{
				GetLogger()->Informational("[%i] Absolute 0x%04X Relative 0x%04X\n", depth++, returnAddress, returnAddress - (uint32_t)hMod);
			}
		}
		else
		{
			GetLogger()->Informational("[%i] Address 0x%04X\n", depth++, returnAddress);
		}

		ebpRegister = (uint32_t*)(*ebpRegister);
		if (ebpRegister == nullptr)
			break;
	}

	GetLogger()->Informational("End of stack trace\n");

	for (const auto& modulePath : modulePaths)
	{
		uint32_t checksum = File::CalculateChecksumA(modulePath.second.c_str());
		GetLogger()->Informational("%s CRC32 %04X\n", modulePath.first.c_str(), checksum);
	}
}

void Exit()
{
	__try {
		StackTrace();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		GetLogger()->Critical("Stack trace crashed!");
	}

	MessageBox(NULL, L"SubTitans has been stopped to prevent undefined behavior.\nPlease check subtitans.log and submit a ticket at Github.", L"Application has been terminated", MB_ICONERROR);
	ExitProcess(-1);
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

	bool VideoRequested = false;

	HWND GameWindow = nullptr;
	HWND RenderWindow = nullptr;

	HANDLE RenderEvent = nullptr;
	HANDLE VerticalBlankEvent = nullptr;

	IRenderer* Backend = nullptr;

	bool RetroShader = false;
	std::atomic<bool> ImGuiEnabled = false;

	_MouseInformation MouseInformation;
	_KeyboardInformation KeyboardInformation;

	static void Init()
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

static const char* GetApplicationLanguage()
{
	const std::wstring STStringDll = L"ST_String.dll";
	if (File::Exists(STStringDll.c_str()))
	{
		uint32_t checksum = File::CalculateChecksumW(STStringDll.c_str());
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

static Patcher* CreateVersionSpecificGamePatcher(unsigned long gameVersion)
{
	Patcher* patcher = nullptr;

	GetLogger()->Informational("Detected Version: ");
	switch (gameVersion)
	{
		case Shared::ST_GAMEVERSION_0_0_6:
			patcher = new TechDemoPatcher();
			GetLogger()->Informational("Technology Demo - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_0_1_6:
			patcher = new DemoPatcher();
			GetLogger()->Informational("Demo - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_1_0_0:
			patcher = new SteamPatcher();
			GetLogger()->Informational("Retail 1.0 - %s\n", GetApplicationLanguage());
			GetLogger()->Warning("DEPRECATED!!! Consider updating to 1.1\n");
			MessageBox(NULL, L"This version of Submarine Titans is outdated, please update to v1.1.", L"SubTitans Patch", MB_ICONWARNING);
			break;
		case Shared::ST_GAMEVERSION_1_1_0:
			patcher = new SteamPatchedPatcher(Shared::IMAGE_BASE);
			GetLogger()->Informational("Retail 1.1 - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_1_1_0_GOG:
			patcher = new GOGPatcher(Shared::IMAGE_BASE);
			GetLogger()->Informational("GOG 1.1 - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_1_1_0_ASLR_DEP:
			patcher = new SteamPatchedPatcher((uint32_t)GetModuleHandle(NULL));
			GetLogger()->Informational("Retail 1.1 with ASLR and DEP enabled (experimental) - %s\n", GetApplicationLanguage());
			break;
		case Shared::ST_GAMEVERSION_1_1_0_GOG_ASLR_DEP:
			patcher = new GOGPatcher((uint32_t)GetModuleHandle(NULL));
			GetLogger()->Informational("GOG 1.1 with ASLR and DEP enabled (experimental) - %s\n", GetApplicationLanguage());
			break;
		default:
			GetLogger()->Critical("Incompatible application\n");
			MessageBox(NULL, L"Incompatible application", L"SubTitans Patch", MB_ICONERROR);
			break;
	}

	return patcher;
}

static void LogOperatingSystem()
{
	GetLogger()->Informational("Operating System: ");

	if (IsWindowsXPOrGreater() && !IsWindowsVistaOrGreater())
		GetLogger()->Informational("Windows XP (unsupported)");
	else if (IsWindowsVistaOrGreater() && !IsWindows7OrGreater())
		GetLogger()->Informational("Windows Vista (unsupported)");
	else if (IsWindows7OrGreater() && !IsWindows8OrGreater())
		GetLogger()->Informational("Windows 7 (unsupported)");
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

static void LogHardwareInformation()
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

static uint32_t GetTimerResolution()
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

void RestartRequired()
{
	GetLogger()->Warning("Game restart required because settings have been modified.\n");
	MessageBox(NULL, L"The game configuration has been changed, please restart the game.", L"SubTitans Patch", MB_ICONINFORMATION);
	ExitProcess(0);
}

static bool HandleASLRAndDEPConfiguration(uint32_t gameVersion)
{
	bool aslrDepEnabled = GetConfiguration()->GetBoolean(L"SECURITY", L"EnableASLRAndDEP", false);
	if (aslrDepEnabled && (gameVersion == Shared::ST_GAMEVERSION_1_1_0 || gameVersion == Shared::ST_GAMEVERSION_1_1_0_GOG))
	{
		int result = MessageBoxW(nullptr, L"Do you want to enable ASLR and DEP?\n\n"
			"This operation modifies the executable, which may cause unexpected side effects.\n"
			"Enabling this for Submarine Titans is experimental and hasn't been thoroughly tested.\n"
			, L"SubTitans ASLR/DEP", MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES)
		{
			GetLogger()->Informational("ALSR and DEP enable requested, PE surgery required.\n");
			if (!PE::Scalpel::EnableDynamicBaseAndNXCompat())
				Exit();
			
			return true;
		}
	}
	else if (!aslrDepEnabled && (gameVersion == Shared::ST_GAMEVERSION_1_1_0_ASLR_DEP || gameVersion == Shared::ST_GAMEVERSION_1_1_0_GOG_ASLR_DEP))
	{
		GetLogger()->Informational("ALSR and DEP disable requested, PE surgery required.\n");
		if (!PE::Scalpel::DisableDynamicBaseAndNXCompat())
			Exit();

		return true;
	}

	GetLogger()->Informational("ASLR and DEP: %s\n", aslrDepEnabled ? "enabled" : "disabled");

	return false;
}

static void TerminateOnHeapCorruption(uint32_t gameVersion)
{
	// This is automatically turned on for 'modern' executables with a subsystem of 6 and higher.
	// The game is ancient and has a subsystem version 4.0 defined in the PE header, so manual enabling it is.
	// https://devblogs.microsoft.com/oldnewthing/20131227-00/?p=2243

	BOOL result = false;
	switch (gameVersion)
	{
		case Shared::ST_GAMEVERSION_1_1_0:
		case Shared::ST_GAMEVERSION_1_1_0_GOG:
		case Shared::ST_GAMEVERSION_1_1_0_ASLR_DEP:
		case Shared::ST_GAMEVERSION_1_1_0_GOG_ASLR_DEP:
			result = HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
			break;
	}

	GetLogger()->Informational("Terminate on heap corruption: %s\n", result ? "enabled" : "disabled");
}

static Patcher* g_GamePatcher = nullptr;
static UINT g_TimerResolution = 0;

#pragma comment(linker, "/EXPORT:InitializeLibrary=_InitializeLibrary@4")
extern "C" void __stdcall InitializeLibrary(unsigned long gameVersion)
{
	Global::Init();

	GetLogger()->Informational("Compilation Date: %s\n", __TIMESTAMP__);

	if (HandleASLRAndDEPConfiguration(gameVersion))
		return RestartRequired();

	TerminateOnHeapCorruption(gameVersion);

	g_GamePatcher = CreateVersionSpecificGamePatcher(gameVersion);
	if (!g_GamePatcher)
		return Exit();

	LogOperatingSystem();
	LogHardwareInformation();

	uint32_t timerResolution = GetTimerResolution();

	GetLogger()->Informational("\nStarting SubTitans\n");
	if (!g_GamePatcher->Initialize())
		return RestartRequired();

	GetLogger()->Informational("Configuring patches\n");
	g_GamePatcher->Configure();

	GetLogger()->Informational("Applying patches\n");
	if (!g_GamePatcher->Apply())
	{
		GetLogger()->Critical("Failed to apply patches\n");
		return Exit();
	}

	// Set timer resolution
	g_TimerResolution = timerResolution;
	if(g_TimerResolution != 0)
		timeBeginPeriod(g_TimerResolution);

	GetLogger()->Informational("Initialized SubTitans\n");
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