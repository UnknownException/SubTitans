#include <Windows.h>
#include <math.h>
#include <ImageHlp.h>
#pragma comment(lib, "ImageHlp.Lib")
#include "types.h"
#include "../shared/gameversion.h"
#include "injector.h"

#pragma comment(linker, "/EXPORT:D3DRMVectorModulus=_D3DRMVectorModulus@4")
extern "C" float __stdcall D3DRMVectorModulus(D3DVector3* vector)
{
	if (vector == nullptr)
		return 0.f;

	return static_cast<float>(sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z));
}

unsigned long GetSubTitansVersion()
{
	WCHAR applicationPath[MAX_PATH];
	GetModuleFileName(NULL, applicationPath, MAX_PATH);

	unsigned long headerSum;
	unsigned long checkSum;

	// Non Kernel32
	if (MapFileAndCheckSumW(applicationPath, &headerSum, &checkSum) != 0)
		return false;
	
	switch (checkSum)
	{
		case Shared::ST_GAMEVERSION_STEAM:
		case Shared::ST_GAMEVERSION_GOG:
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
		unsigned long gameVersion = GetSubTitansVersion();
		switch (gameVersion)
		{
			case Shared::ST_GAMEVERSION_STEAM:
			case Shared::ST_GAMEVERSION_GOG:
			{
				return Injector::Apply(gameVersion) ? TRUE : FALSE;
			}
			default:
				return FALSE;
		}
	}

	return TRUE;
}