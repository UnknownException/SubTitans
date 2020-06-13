#include "subtitans.h"
#include "highdpipatch.h"

namespace HighDPI{
	namespace RetrieveCursorFromWinMessage{
		// Detour variables
		constexpr unsigned long DetourSize = 18;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = 0;

		// Function specific variables
		static unsigned long WinMessage = 0;
		static unsigned long XYPosition = 0;
		static unsigned long XPosition = 0;
		static unsigned long YPosition = 0;

		__declspec(naked) void Implementation()
		{
			__asm lea eax, [ebp - 0x24];
			__asm mov [WinMessage], eax;
			__asm push PM_REMOVE;
			__asm push WM_NULL;
			__asm push WM_NULL;
			__asm push NULL;
			__asm push [WinMessage];
			__asm call [PeekMessageA];
			__asm cmp eax, 0;
			__asm je PFHD_RCFWM_RETURN;
			__asm mov eax, [WinMessage];
			__asm cmp dword ptr ds:[eax + 0x04], WM_MOUSEMOVE;
			__asm jne PFHD_RCFWM_RETURN_TRUE;
			__asm mov eax, dword ptr ds:[eax + 0x0C];
			__asm mov [XYPosition], eax;
			XPosition = GET_X_LPARAM(XYPosition);
			YPosition = GET_Y_LPARAM(XYPosition);
		PFHD_RCFWM_RETURN_TRUE:
			__asm mov eax, 1;
		PFHD_RCFWM_RETURN:
			__asm jmp [JmpBackAddress];
		}
	}

	namespace IgnoreDInputMovement{
		constexpr unsigned long DetourSize = 7;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = 0;

		__declspec(naked) void Implementation()
		{
			__asm mov edi, [RetrieveCursorFromWinMessage::XPosition];
			__asm mov edx, [RetrieveCursorFromWinMessage::YPosition];
			__asm jmp [JmpBackAddress];
		}
	}

	namespace OverrideWindowSize{
		// Detour variables
		constexpr unsigned long DetourSize = 5;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = 0;

		// Function specific variables
		static unsigned long Width = 0;
		static unsigned long Height = 0;

		__declspec(naked) void Implementation()
		{
			__asm mov edx, dword ptr ss:[ebp + 0x10]; // Get width
			__asm cmp edx, 0x500; // keep under 1280x* as is
			__asm jge ASM_PFHD_OVERRIDE;
			__asm cmp edx, 0x400;
			__asm je ASM_PFHD_FIX1024;
			__asm mov edx, dword ptr ss:[ebp + 0x14]; // Get height
			__asm push edx;
			__asm mov edx, dword ptr ss:[ebp + 0x10]; // Get width
			__asm push edx;
			__asm jmp [JmpBackAddress];

		ASM_PFHD_OVERRIDE:
			__asm push [Height];
			__asm push [Width];
			__asm jmp [JmpBackAddress];

		ASM_PFHD_FIX1024:
			__asm pushad;
			__asm pushfd;
			{
				GetLogger()->Critical("1024x768 is broken when using a high dpi resolution. Please reset your resolution through STConfig.exe!\n");
				MessageBox(NULL, L"1024x768 is broken. Please reset your resolution through STConfig.exe!", L"High DPI", MB_ICONERROR);
				ExitProcess(-1);
			}
			__asm popfd;
			__asm popad;
			__asm mov edx, dword ptr ss:[ebp + 0x14] ; // Get height
			__asm push edx;
			__asm mov edx, dword ptr ss:[ebp + 0x10] ; // Get width
			__asm push edx;
			__asm jmp [JmpBackAddress];
		}
	}
}

HighDPIPatch::HighDPIPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	RetrieveCursorFromWindowsMessageDetourAddress = 0;
	IgnoreDInputMovementDetourAddress = 0;
	OverrideWindowSizeDetourAddress = 0;
	MouseExclusiveFlagAddress = 0;

	CheckIfValidResolutionAddress = 0;
}

HighDPIPatch::~HighDPIPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool HighDPIPatch::Validate()
{
	if (!RetrieveCursorFromWindowsMessageDetourAddress ||
		!IgnoreDInputMovementDetourAddress ||
		!OverrideWindowSizeDetourAddress ||
		!MouseExclusiveFlagAddress ||
		!CheckIfValidResolutionAddress)
		return false;

	return true;
}

bool HighDPIPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (screenWidth < 1280 || screenHeight < 720) // Don't patch; user should use 800x600/1024x768
		return true;

	DEVMODE deviceMode;
	ZeroMemory(&deviceMode, sizeof(deviceMode));
	deviceMode.dmSize = sizeof(DEVMODE);
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &deviceMode))
		return false;

	// Check if running in a high DPI mode
	if (screenHeight == deviceMode.dmPelsHeight && screenWidth == deviceMode.dmPelsWidth)
		return true;

	GetLogger()->Informational("Enabling High DPI patch\n");

#ifdef _DEBUG
	MessageBox(NULL, L"Rescaling", L"High DPI", MB_ICONINFORMATION);
#endif

	HighDPI::OverrideWindowSize::Width = deviceMode.dmPelsWidth;
	HighDPI::OverrideWindowSize::Height = deviceMode.dmPelsHeight;

	HighDPI::RetrieveCursorFromWinMessage::JmpFromAddress = RetrieveCursorFromWindowsMessageDetourAddress;
	HighDPI::RetrieveCursorFromWinMessage::JmpBackAddress = HighDPI::RetrieveCursorFromWinMessage::JmpFromAddress + HighDPI::RetrieveCursorFromWinMessage::DetourSize;
	if (!Detour::Create(HighDPI::RetrieveCursorFromWinMessage::JmpFromAddress, HighDPI::RetrieveCursorFromWinMessage::DetourSize, (unsigned long)HighDPI::RetrieveCursorFromWinMessage::Implementation))
		return false;

	HighDPI::IgnoreDInputMovement::JmpFromAddress = IgnoreDInputMovementDetourAddress;
	HighDPI::IgnoreDInputMovement::JmpBackAddress = HighDPI::IgnoreDInputMovement::JmpFromAddress + HighDPI::IgnoreDInputMovement::DetourSize;
	if (!Detour::Create(HighDPI::IgnoreDInputMovement::JmpFromAddress, HighDPI::IgnoreDInputMovement::DetourSize, (unsigned long)HighDPI::IgnoreDInputMovement::Implementation))
		return false;

	HighDPI::OverrideWindowSize::JmpFromAddress = OverrideWindowSizeDetourAddress;
	HighDPI::OverrideWindowSize::JmpBackAddress = HighDPI::OverrideWindowSize::JmpFromAddress + HighDPI::OverrideWindowSize::DetourSize;
	if (!Detour::Create(HighDPI::OverrideWindowSize::JmpFromAddress, HighDPI::OverrideWindowSize::DetourSize, (unsigned long)HighDPI::OverrideWindowSize::Implementation))
		return false;

	// Only affects 1.1; v1.0 is already 6
	// 5 = EXCLUSIVE | FOREGROUND
	// 6 = NONEXCLUSIVE | FOREGROUND
	unsigned char mouseFlags = 0x06; 
	if(!MemoryWriter::Write(MouseExclusiveFlagAddress, &mouseFlags, 1))
		return false;

	// Nasty disable valid resolution check
	const SIZE_T nopArrayLength = 45;
	unsigned char* nopArray = new unsigned char[nopArrayLength];
	memset(nopArray, 0x90, nopArrayLength);
	bool result = MemoryWriter::Write(CheckIfValidResolutionAddress, nopArray, nopArrayLength);
	delete[] nopArray;

	return result;
}

const wchar_t* HighDPIPatch::ErrorMessage()
{
	return L"Failed to apply High DPI patch";
}