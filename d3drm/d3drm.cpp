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

	// Non Kernel32
	if (MapFileAndCheckSumW(applicationPath, &headerSum, &calcSum) != 0)
		return false;

	return calcSum == 0x004337AC;
}

static HMODULE g_SubTitansLibrary = nullptr;
typedef void(__stdcall *g_SubTitansLibrary_InitializeLibrary)();
typedef void(__stdcall *g_SubTitansLibrary_ReleaseLibrary)();

unsigned long constexpr LoadSubTitansModule_DetourSize = 5;
unsigned long constexpr LoadSubTitansModule_JmpFrom = 0x00734B6E;
unsigned long constexpr LoadSubTitansModule_JmpBack = LoadSubTitansModule_JmpFrom + LoadSubTitansModule_DetourSize;
void LoadSubTitansModule_Implementation()
{
	g_SubTitansLibrary = LoadLibrary(L"subtitans.dll");
	if (!g_SubTitansLibrary)
	{
		MessageBox(NULL, L"Failed to load subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
		ExitProcess(-1);
	}

	g_SubTitansLibrary_InitializeLibrary initializeLibrary = (g_SubTitansLibrary_InitializeLibrary)GetProcAddress(g_SubTitansLibrary, "InitializeLibrary");
	if (!initializeLibrary)
	{
		MessageBox(NULL, L"Failed to retrieve InitializeLibrary from subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
		ExitProcess(-1);
	}

	initializeLibrary();
}

__declspec(naked) void LoadSubTitansModule_Detour()
{
	__asm pushad;
	__asm pushfd;

		LoadSubTitansModule_Implementation();

	__asm popfd;
	__asm popad;

	__asm mov eax, 0x00401FEB
	__asm call eax; // WinMain?

	__asm jmp LoadSubTitansModule_JmpBack;
}

unsigned long constexpr UnloadSubTitansModule_DetourSize = 6;
unsigned long constexpr UnloadSubTitansModule_JmpFrom = 0x00734B73;
unsigned long constexpr UnloadSubTitansModule_JmpBack = UnloadSubTitansModule_JmpFrom + UnloadSubTitansModule_DetourSize;
void UnloadSubTitansModule_Implementation()
{
	if (!g_SubTitansLibrary)
	{
		MessageBox(NULL, L"Failed to unload subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
		ExitProcess(-1);
	}

	g_SubTitansLibrary_ReleaseLibrary releaseLibrary = (g_SubTitansLibrary_ReleaseLibrary)GetProcAddress(g_SubTitansLibrary, "ReleaseLibrary");
	if (!releaseLibrary)
	{
		MessageBox(NULL, L"Failed to retrieve ReleaseLibrary from subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
		ExitProcess(-1);
	}

	releaseLibrary();
	FreeLibrary(g_SubTitansLibrary);
}

__declspec(naked) void UnloadSubTitansModule_Detour()
{
	__asm pushad;
	__asm pushfd;

		UnloadSubTitansModule_Implementation();

	__asm popfd;
	__asm popad;

	__asm mov dword ptr ss:[ebp - 0x60], eax;
	__asm mov eax, dword ptr ss:[ebp - 0x60];

	__asm jmp UnloadSubTitansModule_JmpBack;
}

BOOLEAN ApplyDetour(unsigned long origin, SIZE_T length, unsigned long destination)
{
	HANDLE currentProcess = GetCurrentProcess();
	unsigned long* addressPointer = (unsigned long*)origin;

	unsigned long previousProtection = 0;
	if (VirtualProtectEx(currentProcess, addressPointer, length, PAGE_EXECUTE_READWRITE, &previousProtection) == FALSE)
		return FALSE;

	// Hard length limit of 6...
	unsigned char bytesToCopy[] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0x90};
	if (sizeof(bytesToCopy) < length)
		return FALSE;

	unsigned long jumpDistance = destination - origin - 5;

	for (int i = 0; i < 4; ++i)
	{
		unsigned char* destByte = (unsigned char*)&jumpDistance;
		bytesToCopy[i+1] = *(destByte + i);
	}

	SIZE_T writtenBytes = 0;
	if (WriteProcessMemory(currentProcess, addressPointer, bytesToCopy, length, &writtenBytes) == FALSE)
		return FALSE;

	FlushInstructionCache(currentProcess, addressPointer, length);

	if (VirtualProtectEx(currentProcess, addressPointer, length, previousProtection, &previousProtection) == FALSE)
		return FALSE;

	return TRUE;
}

// Use d3drm for injecting custom DLL
BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	// Warning: Only use functions available in Kernel32
	if (reason == DLL_PROCESS_ATTACH)
	{
		if (!IsApplicationSubTitans())
			return FALSE;

		if (!ApplyDetour(LoadSubTitansModule_JmpFrom, LoadSubTitansModule_DetourSize, (unsigned long)LoadSubTitansModule_Detour))
			return FALSE;

		if (!ApplyDetour(UnloadSubTitansModule_JmpFrom, UnloadSubTitansModule_DetourSize, (unsigned long)UnloadSubTitansModule_Detour))
			return FALSE;
	}

	return TRUE;
}