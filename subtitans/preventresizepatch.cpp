#include "subtitans.h"
#include "preventresizepatch.h"

namespace PreventResize {
	// Detour variables
	constexpr unsigned long DetourSize = 17;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	// Function specific variables
	static unsigned long PreviousBitsPerPixel = 0;
	static unsigned long PreviousWidth = 0;
	static unsigned long PreviousHeight = 0;

	__declspec(naked) void Implementation()
	{
		__asm push eax;

		__asm cmp [PreviousBitsPerPixel], edx;
		__asm jne ASM_PR_SETDISPLAYMODE;
		__asm mov eax, dword ptr ds:[ebp + 0x14];
		__asm cmp [PreviousHeight], eax;
		__asm jne ASM_PR_SETDISPLAYMODE;
		__asm mov eax, dword ptr ds:[ebp + 0x10];
		__asm cmp [PreviousWidth], eax;
		__asm jne ASM_PR_SETDISPLAYMODE;

		// Skip DisplayMode
		__asm pop eax;
		__asm mov eax, 0; // DD_OK

		__asm jmp[JmpBackAddress];

	ASM_PR_SETDISPLAYMODE:
		// Store new settings
		__asm mov [PreviousBitsPerPixel], edx;
		__asm mov eax, dword ptr ds:[ebp + 0x14];
		__asm mov [PreviousHeight], eax;
		__asm mov eax, dword ptr ds:[ebp + 0x10];
		__asm mov[PreviousWidth], eax;

		// Restore eax
		__asm pop eax;

		// SetDisplayMode
		__asm push edi;
		__asm push edi;
		__asm push edx;
		__asm mov edx, dword ptr ds:[ebp + 0x14];
		__asm push edx;
		__asm mov edx, dword ptr ds:[ebp + 0x10];
		__asm push edx;
		__asm push eax;
		__asm mov ecx, dword ptr ds:[eax];
		__asm call dword ptr ds:[ecx + 0x54];

		__asm jmp [JmpBackAddress];
	}
}

PreventResizePatch::PreventResizePatch()
{
	GetLogger()->Informational("Initializing %s\n", __func__);

	SetDisplayModeDetourAddress = 0;
	RestoreDisplayModeAddress = 0;
}

PreventResizePatch::~PreventResizePatch()
{
}

bool PreventResizePatch::Validate()
{
	return SetDisplayModeDetourAddress != 0 && RestoreDisplayModeAddress != 0;
}

bool PreventResizePatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	PreventResize::JmpFromAddress = SetDisplayModeDetourAddress;
	PreventResize::JmpBackAddress = PreventResize::JmpFromAddress + PreventResize::DetourSize;

	if (!Detour::Create(PreventResize::JmpFromAddress, PreventResize::DetourSize, (unsigned long)PreventResize::Implementation))
		return false;

	unsigned char nopArray[] = { 0x90, 0x90, 0x90 };
	if (!MemoryWriter::Write(RestoreDisplayModeAddress, nopArray, sizeof(nopArray)))
		return false;

	return true;
}

const wchar_t* PreventResizePatch::ErrorMessage()
{
	return L"Failed to apply Prevent Resize patch";
}