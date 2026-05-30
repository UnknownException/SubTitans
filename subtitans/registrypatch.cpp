#include "subtitans.h"
#include "registrypatch.h"

/*
	The Technology Demo has multiple registry issues, especially on modern operating systems.
	This is a lazy workaround.
*/

namespace RegistryDetour {
	// Detour variables
	constexpr unsigned long DetourSize = 6;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	LSTATUS __stdcall RegQueryValueExADetour(HKEY key, LPCSTR value, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD sizePointer)
	{
		if (_stricmp(value, "CDAudioDrive") == 0 && *type == REG_SZ)
		{
			strcpy_s((char*)data, *sizePointer, "NONE");
			return ERROR_SUCCESS;
		}

		return RegQueryValueExA(key, value, reserved, type, data, sizePointer);
	}

	__declspec(naked) void Implementation()
	{
		__asm mov esi, [RegQueryValueExADetour];
		__asm jmp [JmpBackAddress];
	}
}

RegistryPatch::RegistryPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	DetourAddress = 0;
}

RegistryPatch::~RegistryPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool RegistryPatch::Validate()
{
	return DetourAddress != 0;
}

bool RegistryPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	RegistryDetour::JmpFromAddress = DetourAddress;
	RegistryDetour::JmpBackAddress = RegistryDetour::JmpFromAddress + RegistryDetour::DetourSize;
	return Detour::Create(RegistryDetour::JmpFromAddress, RegistryDetour::DetourSize, (uint32_t)RegistryDetour::Implementation);
}