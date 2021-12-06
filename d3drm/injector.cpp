#include <Windows.h>
#include "../shared/gameversion.h"
#include "injector.h"

namespace Injector {
	static unsigned long GameVersion = 0;

	static HMODULE SubTitansLibrary = nullptr;
	typedef void(__stdcall* SubTitansLibrary_InitializeLibrary)(unsigned long);
	typedef void(__stdcall* SubTitansLibrary_ReleaseLibrary)();

	void LoadModule()
	{
		SubTitansLibrary = LoadLibrary(L"subtitans.dll");
		if (!SubTitansLibrary)
		{
			MessageBox(NULL, L"Failed to load subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
			ExitProcess(-1);
		}

		SubTitansLibrary_InitializeLibrary initializeLibrary = (SubTitansLibrary_InitializeLibrary)GetProcAddress(SubTitansLibrary, "InitializeLibrary");
		if (!initializeLibrary)
		{
			MessageBox(NULL, L"Failed to retrieve InitializeLibrary from subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
			ExitProcess(-1);
		}

		initializeLibrary(GameVersion);
	}

	constexpr unsigned long LoadModule_DetourSize = 5;
	static unsigned long LoadModule_JmpFrom = 0;
	static unsigned long LoadModule_JmpBack = 0;
	static unsigned long StartApplicationAddress = 0;
	__declspec(naked) void LoadModule_Detour()
	{
		__asm pushad;
		__asm pushfd;

		LoadModule();

		__asm popfd;
		__asm popad;

		__asm call [StartApplicationAddress];

		__asm jmp [LoadModule_JmpBack];
	}

	void UnloadModule()
	{
		if (!SubTitansLibrary)
		{
			MessageBox(NULL, L"Failed to unload subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
			ExitProcess(-1);
		}

		SubTitansLibrary_ReleaseLibrary releaseLibrary = (SubTitansLibrary_ReleaseLibrary)GetProcAddress(SubTitansLibrary, "ReleaseLibrary");
		if (!releaseLibrary)
		{
			MessageBox(NULL, L"Failed to retrieve ReleaseLibrary from subtitans.dll!", L"D3DRM (Custom)", MB_ICONERROR);
			ExitProcess(-1);
		}

		releaseLibrary();
		FreeLibrary(SubTitansLibrary);
	}

	constexpr unsigned long UnloadModule_DetourSize = 6;
	static unsigned long UnloadModule_JmpFrom = 0;
	static unsigned long UnloadModule_JmpBack = 0;
	__declspec(naked) void UnloadModule_Detour()
	{
		__asm pushad;
		__asm pushfd;

		UnloadModule();

		__asm popfd;
		__asm popad;

		__asm mov dword ptr ss:[ebp - 0x60], eax;
		__asm mov eax, dword ptr ss:[ebp - 0x60];

		__asm jmp [UnloadModule_JmpBack];
	}

	// Only Kernel32 dependent
	bool ApplyDetour(unsigned long origin, unsigned long length, unsigned long destination)
	{
		HANDLE currentProcess = GetCurrentProcess();
		unsigned long* originPointer = (unsigned long*)origin;

		unsigned long previousProtection = 0;
		if (VirtualProtectEx(currentProcess, originPointer, length, PAGE_EXECUTE_READWRITE, &previousProtection) == FALSE)
			return false;

		// Hard length limit of 6...
		unsigned char bytesToCopy[] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0x90 };
		if (sizeof(bytesToCopy) < length)
			return false;

		unsigned long jumpDistance = destination - origin - 5;

		for (int i = 0; i < 4; ++i)
		{
			unsigned char* destByte = (unsigned char*)&jumpDistance;
			bytesToCopy[i + 1] = *(destByte + i);
		}

		SIZE_T writtenBytes = 0;
		if (WriteProcessMemory(currentProcess, originPointer, bytesToCopy, length, &writtenBytes) == FALSE)
			return false;

		FlushInstructionCache(currentProcess, originPointer, length);

		if (VirtualProtectEx(currentProcess, originPointer, length, previousProtection, &previousProtection) == FALSE)
			return false;

		return true;
	}

	bool Apply(unsigned long gameVersion)
	{
		GameVersion = gameVersion;
		switch (GameVersion)
		{
			case Shared::ST_GAMEVERSION_RETAIL_UNPATCHED:
			{
				LoadModule_JmpFrom = 0x00734B6E;
				LoadModule_JmpBack = LoadModule_JmpFrom + LoadModule_DetourSize;

				UnloadModule_JmpFrom = 0x00734B73;
				UnloadModule_JmpBack = UnloadModule_JmpFrom + UnloadModule_DetourSize;

				StartApplicationAddress = 0x00401FEB;
			} break;
			case Shared::ST_GAMEVERSION_RETAIL_PATCHED:
			case Shared::ST_GAMEVERSION_GOG_MODIFIED:
			{
				LoadModule_JmpFrom = 0x007337FE;
				LoadModule_JmpBack = LoadModule_JmpFrom + LoadModule_DetourSize;

				UnloadModule_JmpFrom = 0x00733803;
				UnloadModule_JmpBack = UnloadModule_JmpFrom + UnloadModule_DetourSize;

				StartApplicationAddress = 0x00401FF5;
			} break;
			default:
				return false;
		}

		if (!ApplyDetour(LoadModule_JmpFrom, LoadModule_DetourSize, (unsigned long)LoadModule_Detour))
			return false;

		if (!ApplyDetour(UnloadModule_JmpFrom, UnloadModule_DetourSize, (unsigned long)UnloadModule_Detour))
			return false;

		return true;
	}
}