#include <Windows.h>
#include <math.h>
#include <ImageHlp.h>
#pragma comment(lib, "ImageHlp.Lib")
#include "types.h"

#pragma comment(linker, "/EXPORT:D3DRMVectorModulus=_D3DRMVectorModulus@4")
extern "C" float __stdcall D3DRMVectorModulus(D3DVector3* vector)
{
	if (vector == nullptr)
		return 0.f;

	return static_cast<float>(sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z));
}

bool IsApplicationSubTitans()
{
	WCHAR applicationPath[MAX_PATH];
	GetModuleFileName(NULL, applicationPath, MAX_PATH);

	unsigned long headerSum;
	unsigned long calcSum;

	if (MapFileAndCheckSumW(applicationPath, &headerSum, &calcSum) != 0)
		return false;

	return calcSum == 0x004337AC;
}

//https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain
void LoadSubTitansModule()
{
	// LoadLibrary shouldn't be called from DllMain!
	if (!LoadLibrary(L"subtitans.dll"))
	{
		if (MessageBox(NULL, L"Failed to load subtitans.dll!\nDo you want to continue?", L"D3DRM (Custom)", MB_YESNO | MB_ICONERROR) != IDYES)
			ExitProcess(-1);
	}
}

// Abuse d3drm for injecting custom DLL
BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (!IsApplicationSubTitans())
			{
				MessageBox(NULL, L"This DLL is meant for Submarine Titans.", L"Error", MB_ICONERROR);
				ExitProcess(-1);
			}

			LoadSubTitansModule();
		}	break;
		case DLL_PROCESS_DETACH:
		default:
			break;
	}

	return TRUE;
}