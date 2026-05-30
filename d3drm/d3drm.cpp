#include <Windows.h>
#include <math.h>
#include <cstdint>
#include "types.h"
#include "../shared/gameversion.h"
#include "../shared/file.h"
#include "injector.h"

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

	uint32_t checkSum = File::CalculateChecksumW(applicationPath);
	
	switch (checkSum)
	{
		case Shared::ST_GAMEVERSION_0_0_6:
		case Shared::ST_GAMEVERSION_0_1_6:
		case Shared::ST_GAMEVERSION_1_0_0:
		case Shared::ST_GAMEVERSION_1_1_0:
		case Shared::ST_GAMEVERSION_1_1_0_ASLR_DEP:
		case Shared::ST_GAMEVERSION_1_1_0_GOG:
		case Shared::ST_GAMEVERSION_1_1_0_GOG_ASLR_DEP:
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