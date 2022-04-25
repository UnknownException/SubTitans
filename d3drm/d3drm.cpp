#include <Windows.h>
#include <math.h>
#include <cstdint>
#include "types.h"
#include "../shared/gameversion.h"
#include "../shared/file.h"
#include "injector.h"

#ifdef _DEBUG
	#pragma comment(lib, "../Debug/shared.lib")
#else
	#pragma comment(lib, "../Release/shared.lib")
#endif

#pragma comment(linker, "/EXPORT:D3DRMVectorModulus=_D3DRMVectorModulus@4")
extern "C" float __stdcall D3DRMVectorModulus(D3DVector3* vector)
{
	if (vector == nullptr)
		return 0.f;

	return static_cast<float>(sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z));
}

uint32_t GetSubTitansVersion()
{
	WCHAR applicationPath[MAX_PATH];
	GetModuleFileName(NULL, applicationPath, MAX_PATH);

	uint32_t checkSum = File::CalculateChecksum(applicationPath);
	
	switch (checkSum)
	{
		case Shared::ST_GAMEVERSION_RETAIL_UNPATCHED:
		case Shared::ST_GAMEVERSION_RETAIL_PATCHED:
		case Shared::ST_GAMEVERSION_GOG_MODIFIED:
		case Shared::ST_GAMEVERSION_DEMO:
			return checkSum;
		default:
			return 0;
	}
}

// Use d3drm for injecting custom DLL
BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	// Warning: Only use functions available in Kernel32
	if (reason == DLL_PROCESS_ATTACH)
	{
		uint32_t gameVersion = GetSubTitansVersion();
		if (gameVersion == 0)
			return FALSE;

		// Allow uninstalling patch by removing SubTitans.dll
		if (!File::Exists(L"SubTitans.dll"))
			return TRUE;

		return Injector::Apply(gameVersion);
	}

	return TRUE;
}