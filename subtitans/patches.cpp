#include <Windows.h>
#include "patches.h"
#include "memorywriter.h"
#include "detour.h"

bool WindowedMode()
{
	// Overwrites FULLSCREEN | NOWINDOWCHANGES | ALLOWMODEX
	unsigned char normalFlag[] = { 0x08 };
	if (!MemoryWriter::Write(0x006BAC5F, normalFlag, 1))
		return false;

	// No clue about the original intention but it has to be forced to 0x08
	unsigned char movNormalFlagIntoEDX[] = { 0xBA, 0x08, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
	if (!MemoryWriter::Write(0x006BAEA6, movNormalFlagIntoEDX, sizeof(movNormalFlagIntoEDX)))
		return false;

	return true;
}

#include <windowsx.h>
// Detour variables
unsigned long constexpr PatchForHighDPI_RetrieveCursorFromWinMessage_DetourSize = 18;
unsigned long constexpr PatchForHighDPI_RetrieveCursorFromWinMessage_JmpFrom = 0x006E62B9;
unsigned long constexpr PatchForHighDPI_RetrieveCursorFromWinMessage_JmpBack = PatchForHighDPI_RetrieveCursorFromWinMessage_JmpFrom + PatchForHighDPI_RetrieveCursorFromWinMessage_DetourSize;
// Function specific variables
static unsigned long PatchForHighDPI_RetrieveCursorFromWinMessage_WinMessage = 0;
static unsigned long PatchForHighDPI_RetrieveCursorFromWinMessage_XY = 0;
static unsigned long PatchForHighDPI_RetrieveCursorFromWinMessage_X = 0;
static unsigned long PatchForHighDPI_RetrieveCursorFromWinMessage_Y = 0;
__declspec(naked) void PatchForHighDPI_RetrieveCursorFromWinMessage_Implementation()
{
	__asm lea eax, [ebp - 0x24];
	__asm mov PatchForHighDPI_RetrieveCursorFromWinMessage_WinMessage, eax;
	__asm push PM_REMOVE;
	__asm push WM_NULL;
	__asm push WM_NULL;
	__asm push NULL;
	__asm push PatchForHighDPI_RetrieveCursorFromWinMessage_WinMessage;
	__asm call PeekMessageA;
	__asm cmp eax, 0;
	__asm je PFHD_RCFWM_RETURN;
	__asm mov eax, PatchForHighDPI_RetrieveCursorFromWinMessage_WinMessage;
	__asm cmp dword ptr ds:[eax + 0x04], WM_MOUSEMOVE;
	__asm jne PFHD_RCFWM_RETURN;
	__asm mov eax, dword ptr ds:[eax + 0x0C];
	__asm mov PatchForHighDPI_RetrieveCursorFromWinMessage_XY, eax;
	PatchForHighDPI_RetrieveCursorFromWinMessage_X = GET_X_LPARAM(PatchForHighDPI_RetrieveCursorFromWinMessage_XY);
	PatchForHighDPI_RetrieveCursorFromWinMessage_Y = GET_Y_LPARAM(PatchForHighDPI_RetrieveCursorFromWinMessage_XY);
	__asm mov eax, 1;
PFHD_RCFWM_RETURN:
	__asm jmp PatchForHighDPI_RetrieveCursorFromWinMessage_JmpBack;
}

unsigned long constexpr PatchForHighDPI_IgnoreDInputMovement_DetourSize = 7;
unsigned long constexpr PatchForHighDPI_IgnoreDInputMovement_JmpFrom = 0x0071C9C0;
unsigned long constexpr PatchForHighDPI_IgnoreDInputMovement_JmpBack = PatchForHighDPI_IgnoreDInputMovement_JmpFrom + PatchForHighDPI_IgnoreDInputMovement_DetourSize;
__declspec(naked) void PatchForHighDPI_IgnoreDInputMovement_Implementation()
{
	__asm mov edi, PatchForHighDPI_RetrieveCursorFromWinMessage_X;
	__asm mov edx, PatchForHighDPI_RetrieveCursorFromWinMessage_Y;
	__asm jmp PatchForHighDPI_IgnoreDInputMovement_JmpBack;
}

// Detour variables
unsigned long constexpr PatchForHighDPI_OverrideWindowSize_DetourSize = 5;
unsigned long constexpr PatchForHighDPI_OverrideWindowSize_JmpFrom = 0x006BAEEE;
unsigned long constexpr PatchForHighDPI_OverrideWindowSize_JmpBack = PatchForHighDPI_OverrideWindowSize_JmpFrom + PatchForHighDPI_OverrideWindowSize_DetourSize;
// Function specific variables
static unsigned long PatchForHighDPI_OverrideWindowSize_Width = 0;
static unsigned long PatchForHighDPI_OverrideWindowSize_Height = 0;
__declspec(naked) void PatchForHighDPI_OverrideWindowSize_Implementation()
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
	__asm jmp PatchForHighDPI_OverrideWindowSize_JmpBack;

ASM_PFHD_OVERRIDE:
	__asm push PatchForHighDPI_OverrideWindowSize_Height;
	__asm push PatchForHighDPI_OverrideWindowSize_Width;
	__asm jmp PatchForHighDPI_OverrideWindowSize_JmpBack;

ASM_PFHD_FIX1024:
	__asm pushad;
	__asm pushfd;
	{
		MessageBox(NULL, L"1024x768 is broken. Please reset your resolution through STConfig.exe!", L"High DPI", MB_ICONERROR);
		ExitProcess(-1);
	}
	__asm popfd;
	__asm popad;
	__asm mov edx, dword ptr ss:[ebp + 0x14] ; // Get height
	__asm push edx;
	__asm mov edx, dword ptr ss:[ebp + 0x10] ; // Get width
	__asm push edx;
	__asm jmp PatchForHighDPI_OverrideWindowSize_JmpBack;
}

bool PatchForHighDPI(int screenWidth, int screenHeight)
{
	DEVMODE deviceMode;
	ZeroMemory(&deviceMode, sizeof(deviceMode));
	deviceMode.dmSize = sizeof(DEVMODE);
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &deviceMode))
		return false;

	if (screenHeight == deviceMode.dmPelsHeight && screenWidth == deviceMode.dmPelsWidth)
		return true;

#ifdef _DEBUG
	MessageBox(NULL, L"Rescaling", L"High DPI", MB_ICONINFORMATION);
#endif

	PatchForHighDPI_OverrideWindowSize_Width = deviceMode.dmPelsWidth;
	PatchForHighDPI_OverrideWindowSize_Height = deviceMode.dmPelsHeight;

	if (!Detour::Create(PatchForHighDPI_RetrieveCursorFromWinMessage_JmpFrom, PatchForHighDPI_RetrieveCursorFromWinMessage_DetourSize, (unsigned long)PatchForHighDPI_RetrieveCursorFromWinMessage_Implementation))
		return false;

	if (!Detour::Create(PatchForHighDPI_IgnoreDInputMovement_JmpFrom, PatchForHighDPI_IgnoreDInputMovement_DetourSize, (unsigned long)PatchForHighDPI_IgnoreDInputMovement_Implementation))
		return false;

	if (!Detour::Create(PatchForHighDPI_OverrideWindowSize_JmpFrom, PatchForHighDPI_OverrideWindowSize_DetourSize, (unsigned long)PatchForHighDPI_OverrideWindowSize_Implementation))
		return false;

	// Nasty disable valid resolution check
	const SIZE_T nopArrayLength = 45;
	unsigned char* nopArray = new unsigned char[nopArrayLength];
	memset(nopArray, 0x90, nopArrayLength);
	MemoryWriter::Write(0x0056F088, nopArray, nopArrayLength);
	delete[] nopArray;

	return true;
}

// Detour variables
unsigned long constexpr NativeResolution_RepositionBottomMenu_DetourSize = 26;
unsigned long constexpr NativeResolution_RepositionBottomMenu_JmpFrom = 0x004F8084;
unsigned long constexpr NativeResolution_RepositionBottomMenu_JmpBack = NativeResolution_RepositionBottomMenu_JmpFrom + NativeResolution_RepositionBottomMenu_DetourSize;
// Function specific variables
static unsigned long NativeResolution_RepositionBottomMenu_MarginLeft = 0;
__declspec(naked) void NativeResolution_RepositionBottomMenu_Implementation()
{
	__asm mov ecx, 0x0A 
	__asm mov dword ptr ds:[esi + 0x60], edx

	__asm mov edx, dword ptr ds:[esi + 0x8C]; 
	__asm cmp edx, 0xF0;
	__asm jne ASM_NR_RBM_LOOP;
	__asm add ecx, 0x01; // + 1 for Left TV panel repositioning (orig 0a)

ASM_NR_RBM_LOOP:
	__asm mov edx, dword ptr ds:[esi + 0x8C];
	__asm cmp edx, 0xF0; // Only do this for 1280x1024 resolution modification
	__asm jne ASM_NR_RBM_NOTNATIVERES;
	__asm cmp ecx, 1 // Check if last (0x0B: Left TV panel) <-- Don't add default value if match
	__asm jne ASM_NR_RBM_DONTOVERRIDEDEFAULT
	__asm mov edx, 0x00

ASM_NR_RBM_DONTOVERRIDEDEFAULT:
	__asm add edx, NativeResolution_RepositionBottomMenu_MarginLeft; // Widescreen reposition value

ASM_NR_RBM_NOTNATIVERES:
	// Restore
	__asm mov ebx, dword ptr ds:[eax];
	__asm add ebx, edx;
	__asm mov dword ptr ds:[eax], ebx;
	__asm add eax, 4;
	__asm dec ecx;
	__asm jnz ASM_NR_RBM_LOOP;

	__asm jmp NativeResolution_RepositionBottomMenu_JmpBack;
}

// Detour variables
unsigned long constexpr NativeResolution_RenameSetting_DetourSize = 6;
unsigned long constexpr NativeResolution_RenameSetting_JmpFrom = 0x005309D0;
unsigned long constexpr NativeResolution_RenameSetting_JmpBack = NativeResolution_RenameSetting_JmpFrom + NativeResolution_RenameSetting_DetourSize;
// Function specific variables
static char* NativeResolution_RenameSetting_CurrentStringPtr = 0;
static char NativeResolution_RenameSetting_TargetString[] = { '1', '2', '8', '0', 'x', '1', '0', '2', '4', 0x00 };
static char NativeResolution_RenameSetting_NewString[] = { 'N', 'A', 'T', 'I', 'V', 'E', ' ', 'R', 'E', 'S', 'O', 'L', 'U', 'T', 'I', 'O', 'N', 0x00 };
__declspec(naked) void NativeResolution_RenameSetting_Implementation()
{
	__asm mov NativeResolution_RenameSetting_CurrentStringPtr, eax;

	__asm pushad;
	__asm pushfd;
		if (strcmp(NativeResolution_RenameSetting_TargetString, NativeResolution_RenameSetting_CurrentStringPtr) == 0)
			NativeResolution_RenameSetting_CurrentStringPtr = NativeResolution_RenameSetting_NewString;
	__asm popfd;
	__asm popad;

	__asm push NativeResolution_RenameSetting_CurrentStringPtr;
	__asm mov eax, 0x00712E20;
	__asm call eax;
	__asm jmp NativeResolution_RenameSetting_JmpBack;
}

// Detour variables
unsigned long constexpr NativeResolution_RedesignFrame_DetourSize = 18;
unsigned long constexpr NativeResolution_RedesignFrame_JmpFrom = 0x00544872;
unsigned long constexpr NativeResolution_RedesignFrame_JmpBack = NativeResolution_RedesignFrame_JmpFrom + NativeResolution_RedesignFrame_DetourSize;
// Function specific variables
static unsigned char* NativeResolution_RedesignFrame_ImagePtr;
static unsigned char NativeResolution_RedesignFrame_LastTeamId = 0; // Required for check if frame has to be remade
const int NativeResolution_RedesignFrame_HeaderAndColorTableSize = 40 /* 0x28 */ + 1024;
static unsigned char* NativeResolution_RedesignFrame_FrameBuffer = nullptr;
static int NativeResolution_RedesignFrame_Width = 0;
static int NativeResolution_RedesignFrame_Height = 0;

void NativeResolution_RedesignFrame_FillRect(unsigned int sourceX, unsigned int sourceY, unsigned int targetX, unsigned int targetY, unsigned int width, unsigned int height)
{
	int sourceWidth = 0;
	memcpy(&sourceWidth, NativeResolution_RedesignFrame_ImagePtr + 0x04, sizeof(int)); // Always 1280

	int sourceHeight = 0;
	memcpy(&sourceHeight, NativeResolution_RedesignFrame_ImagePtr + 0x08, sizeof(int)); // Always 1024

	int targetWidth = 0;
	memcpy(&targetWidth, NativeResolution_RedesignFrame_FrameBuffer + 0x04, sizeof(int));

	int targetHeight = 0;
	memcpy(&targetHeight, NativeResolution_RedesignFrame_FrameBuffer + 0x08, sizeof(int));

	// Lower height is ok (720 is lowest)
	if (sourceWidth != 1280 || sourceHeight != 1024 || targetWidth < sourceWidth)
		return;

	sourceY = sourceHeight - height - sourceY;
	targetY = targetHeight - height - targetY;

	unsigned long sourceBoundary = (unsigned long)NativeResolution_RedesignFrame_ImagePtr + NativeResolution_RedesignFrame_HeaderAndColorTableSize + sourceWidth * sourceHeight;
	unsigned long targetBoundary = (unsigned long)NativeResolution_RedesignFrame_FrameBuffer + NativeResolution_RedesignFrame_HeaderAndColorTableSize + targetWidth * targetHeight;

	for (unsigned int line = 0; line < height; ++line)
	{
		bool outOfBounds = false;

		unsigned char* sourcePtr = NativeResolution_RedesignFrame_ImagePtr + NativeResolution_RedesignFrame_HeaderAndColorTableSize; // Set pointer past color table
		sourcePtr += (line + sourceY) * sourceWidth + sourceX;

		unsigned long sourceReadEndPosition = (unsigned long)sourcePtr + width;
		if (sourceReadEndPosition > sourceBoundary)
			outOfBounds = true;

		unsigned char* targetPtr = NativeResolution_RedesignFrame_FrameBuffer + NativeResolution_RedesignFrame_HeaderAndColorTableSize; // Set pointer past color table
		targetPtr += (line + targetY) * targetWidth + targetX;

		unsigned long targetReadEndPosition = (unsigned long)targetPtr + width;
		if (targetReadEndPosition > targetBoundary)
			outOfBounds = true;

		if (!outOfBounds)
			memcpy(targetPtr, sourcePtr, width);
	}
}

void NativeResolution_RedesignFrame_Render()
{
	int imageSize = NativeResolution_RedesignFrame_Width * NativeResolution_RedesignFrame_Height;
	memset(NativeResolution_RedesignFrame_FrameBuffer, 0x00, NativeResolution_RedesignFrame_HeaderAndColorTableSize + imageSize);

	memcpy(NativeResolution_RedesignFrame_FrameBuffer, NativeResolution_RedesignFrame_ImagePtr, NativeResolution_RedesignFrame_HeaderAndColorTableSize);
	memcpy(NativeResolution_RedesignFrame_FrameBuffer + 0x04, &NativeResolution_RedesignFrame_Width, sizeof(int));
	memcpy(NativeResolution_RedesignFrame_FrameBuffer + 0x08, &NativeResolution_RedesignFrame_Height, sizeof(int));
	memcpy(NativeResolution_RedesignFrame_FrameBuffer + 0x14, &imageSize, sizeof(int));

	// Top bar
	const int topLeftWidth = 426;
	const int topMidWidth = 448;
	const int topRightWidth = 406;
	const int topHeight = 30;

	NativeResolution_RedesignFrame_FillRect(0, 0, 0, 0, topLeftWidth, topHeight); // Left
	const int topMidRequiredWidth = NativeResolution_RedesignFrame_Width - topRightWidth - topLeftWidth;
	for (int i = 0; i < topMidRequiredWidth;)
	{
		int drawWidth = topMidWidth;
		if (i + drawWidth > topMidRequiredWidth)
			drawWidth -= i + drawWidth - topMidRequiredWidth;

		NativeResolution_RedesignFrame_FillRect(topLeftWidth, 0, topLeftWidth + i, 0, drawWidth, topHeight); // Middle

		i += drawWidth;
	}
	NativeResolution_RedesignFrame_FillRect(1280 - topRightWidth, 0, NativeResolution_RedesignFrame_Width - topRightWidth, 0, topRightWidth, topHeight); // Right
	
	// Left bar
	const int leftWidth = 12;
	const int leftHeight = 772; // Not precise, doesn't matter; looks good
	const int leftBottomHeight = 42;

	const int leftRequiredHeight = NativeResolution_RedesignFrame_Height - topHeight - leftBottomHeight;
	for (int i = 0; i < leftRequiredHeight;)
	{
		int drawHeight = leftHeight;
		if (i + drawHeight > leftRequiredHeight)
			drawHeight -= i + drawHeight - leftRequiredHeight;

		NativeResolution_RedesignFrame_FillRect(0, topHeight, 0, topHeight + i, leftWidth, drawHeight); // Top

		i += drawHeight;
	}
	NativeResolution_RedesignFrame_FillRect(0, 1024 - leftBottomHeight, 0, NativeResolution_RedesignFrame_Height - leftBottomHeight, leftWidth, leftBottomHeight); // Bottom

	// Bottom bar
	const int bottomHeight = 32;
	const int bottomMidWidth = 692;
	const int bottomRightWidth = 133;

	const int bottomRequiredWidth = NativeResolution_RedesignFrame_Width - leftWidth - bottomRightWidth;
	for (int i = 0; i < bottomRequiredWidth;)
	{
		int drawWidth = bottomMidWidth;
		if (i + drawWidth > bottomRequiredWidth)
			drawWidth -= i + drawWidth - bottomRequiredWidth;

		NativeResolution_RedesignFrame_FillRect(306, 1024 - bottomHeight, leftWidth + i, NativeResolution_RedesignFrame_Height - bottomHeight, drawWidth, bottomHeight); // Left

		i += drawWidth;
	}
	NativeResolution_RedesignFrame_FillRect(1280 - bottomRightWidth, 1024 - bottomHeight, NativeResolution_RedesignFrame_Width - bottomRightWidth, NativeResolution_RedesignFrame_Height - bottomHeight, bottomRightWidth, bottomHeight); // Right

	// Right bar
	const int rightWidth = 28;
	const int rightTopHeight = 336;
	const int rightMidHeight = 422;
	const int rightBottomHeight = 204;

	NativeResolution_RedesignFrame_FillRect(1280 - rightWidth, topHeight, NativeResolution_RedesignFrame_Width - rightWidth, topHeight, rightWidth, rightTopHeight); // Top
	const int rightMidRequiredHeight = NativeResolution_RedesignFrame_Height - rightTopHeight - rightBottomHeight - topHeight - bottomHeight;
	for (int i = 0; i < rightMidRequiredHeight;)
	{
		int drawHeight = rightMidHeight;
		if (i + drawHeight > rightMidRequiredHeight)
			drawHeight -= i + drawHeight - rightMidRequiredHeight;

		NativeResolution_RedesignFrame_FillRect(1280 - rightWidth, topHeight + rightTopHeight, NativeResolution_RedesignFrame_Width - rightWidth, topHeight + rightTopHeight + i, rightWidth, drawHeight); // Mid

		i += drawHeight;
	}
	NativeResolution_RedesignFrame_FillRect(1280 - rightWidth, 1024-bottomHeight-rightBottomHeight, NativeResolution_RedesignFrame_Width - rightWidth, NativeResolution_RedesignFrame_Height - bottomHeight - rightBottomHeight, rightWidth, rightBottomHeight); // Bottom

}

__declspec(naked) void NativeResolution_RedesignFrame_Implementation()
{
	__asm mov dword ptr ss:[ebp - 0x08], eax;

	// Check image properties
	__asm cmp dword ptr ds:[eax], 0x28; // Header size (bmp (v4))
	__asm jne ASM_NR_RF_DEFAULT;
	__asm cmp dword ptr ds:[eax + 0x04], 0x500; // Width
	__asm jne ASM_NR_RF_DEFAULT;
	__asm cmp dword ptr ds:[eax + 0x08], 0x400; // Height
	__asm jne ASM_NR_RF_DEFAULT;
	__asm cmp word ptr ds:[eax + 0x0C], 0x01; // Layers
	__asm jne ASM_NR_RF_DEFAULT;
	__asm cmp word ptr ds:[eax + 0x0E], 0x08; // Bits
	__asm jne ASM_NR_RF_DEFAULT;
	__asm cmp dword ptr ds:[eax + 0x20], 0x100 // Color Table items 
	__asm jne ASM_NR_RF_DEFAULT;

	// Get original image
	__asm mov NativeResolution_RedesignFrame_ImagePtr, eax; 

	// Uninitialized teamid?
	__asm cmp byte ptr ds:[0x0080911E], 0x00; 
	__asm je ASM_NR_RF_DEFAULT;

	// Check if teams have changes
	__asm xor eax, eax;
	__asm mov al, NativeResolution_RedesignFrame_LastTeamId;
	__asm cmp byte ptr ds:[0x0080911E], al;
	__asm je ASM_NR_RF_RENDERREDESIGN;

	// Store team id
	__asm mov al, byte ptr ds:[0x0080911E];
	__asm mov NativeResolution_RedesignFrame_LastTeamId, al;

	// Create new frame
	__asm pushad;
	__asm pushfd;
		NativeResolution_RedesignFrame_Render();
	__asm popfd;
	__asm popad;

ASM_NR_RF_RENDERREDESIGN:
	__asm push NativeResolution_RedesignFrame_FrameBuffer;
	__asm push 1;
	__asm push 0;
	__asm push 0;
	__asm mov eax, 0x0040372E;
	__asm call eax;
	__asm add esp, 0x10;

	__asm jmp NativeResolution_RedesignFrame_JmpBack;

ASM_NR_RF_DEFAULT:
	__asm push eax;
	__asm push 1;
	__asm push 0;
	__asm push 0
	__asm mov eax, 0x0040372E;
	__asm call eax;
	__asm add esp, 0x10;

	__asm jmp NativeResolution_RedesignFrame_JmpBack;
}

// Overwrites 1280x1024
bool NativeResolution()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (screenWidth < 1280 || screenHeight < 720) // Don't patch; user should use 800x600/1024x768
		return true;

	if (!PatchForHighDPI(screenWidth, screenHeight))
		return false;

	if (screenWidth == 1366 && screenHeight == 768)
	{
		MessageBox(NULL, L"1366x768 causes rendering issues; trying 1280x768", L"Warning", MB_ICONWARNING);
		screenWidth = 1280;
	}

	unsigned char buffer[4];
	
	// Create window Width (store @ 807100): Default 800
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6C5, buffer, sizeof(int)))
		return false;

	// Create window Height (store @ 807104): Default 600
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6CF, buffer, sizeof(int)))
		return false;

	// GUI Rescaler?
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x004F3254, buffer, sizeof(int)))
		return false;

	// Queuescreen bottom @ ingame patch (0: 800, 2:1024, 6: 1280)
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x004F9CCB, buffer, sizeof(int)))
		return false;

	// Initial screen width/height
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0053202B, buffer, sizeof(int)))
		return false;
	
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x00532032, buffer, sizeof(int)))
		return false;

	// Screen resize compare value
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056EFCA, buffer, sizeof(int)))
		return false;

	// Screen resize values (width, height)
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056F035, buffer, sizeof(int)))
		return false;

	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056F03F, buffer, sizeof(int)))
		return false;

	// Gamefield preset values (width, height)
	int fieldWidth = screenWidth - 40;
	memcpy(buffer, &fieldWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056CA0C, buffer, sizeof(int)))
		return false;

	int fieldHeight = screenHeight - 62;
	memcpy(buffer, &fieldHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056CA16, buffer, sizeof(int)))
		return false;

	// The movie resolution patch does NOT scale the movie
	// Perhaps try to fix this someday?

	// Movie resolution patch (Width)
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x00571D55, buffer, sizeof(int)))
		return false;

	// Movie resolution patch (Height)
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x00571D5C, buffer, sizeof(int)))
		return false;

	NativeResolution_RepositionBottomMenu_MarginLeft = (screenWidth - 1280) / 2;

	NativeResolution_RedesignFrame_Width = screenWidth;
	NativeResolution_RedesignFrame_Height = screenHeight;
	NativeResolution_RedesignFrame_FrameBuffer = new unsigned char[NativeResolution_RedesignFrame_HeaderAndColorTableSize + screenWidth * screenHeight];

	if (!Detour::Create(NativeResolution_RepositionBottomMenu_JmpFrom, NativeResolution_RepositionBottomMenu_DetourSize, (unsigned long)NativeResolution_RepositionBottomMenu_Implementation))
		return false;

	if (!Detour::Create(NativeResolution_RenameSetting_JmpFrom, NativeResolution_RenameSetting_DetourSize, (unsigned long)NativeResolution_RenameSetting_Implementation))
		return false;

	if (!Detour::Create(NativeResolution_RedesignFrame_JmpFrom, NativeResolution_RedesignFrame_DetourSize, (unsigned long)NativeResolution_RedesignFrame_Implementation))
		return false;

	return true;
}

#pragma comment(lib, "winmm.lib")
#include <cmath>

// Detour variables
unsigned long constexpr SleepWell_Implementation_DetourSize = 10;
unsigned long constexpr SleepWell_Implementation_JmpFrom = 0x006E6253;
unsigned long constexpr SleepWell_Implementation_JmpBack = SleepWell_Implementation_JmpFrom + SleepWell_Implementation_DetourSize;

// Function specific variables
static unsigned long SleepWell_Implementation_CurrentTime = timeGetTime();
static unsigned long SleepWell_Implementation_PreviousTime = timeGetTime();
static unsigned long SleepWell_Implementation_Difference = 0;
unsigned long constexpr SleepWell_Implementation_FrameLimit = 1000 / 25; // TODO: Decouple logics & rendering (clock is correct @ 25fps) <-- Seems to cause issues with some menu effects
__declspec(naked) void SleepWell_Implementation()
{
	__asm pushad;
	__asm pushfd;

	SleepWell_Implementation_CurrentTime = timeGetTime();
	SleepWell_Implementation_Difference = SleepWell_Implementation_CurrentTime - SleepWell_Implementation_PreviousTime;

	// Reset if way behind rendering schedule (Try to prevent negative effect of sleep inaccuracy)
	if (std::abs((long)SleepWell_Implementation_CurrentTime - (long)SleepWell_Implementation_PreviousTime) > 1000)
	{
#ifdef _DEBUG
//		MessageBox(NULL, L"Rendering is behind schedule", L"FPS Limiter", MB_ICONINFORMATION);
#endif
		SleepWell_Implementation_PreviousTime = SleepWell_Implementation_CurrentTime - SleepWell_Implementation_FrameLimit;
	}

	// Sleep (inaccurate) to prevent rendering too many frames
	if (SleepWell_Implementation_Difference < SleepWell_Implementation_FrameLimit)
	{
		Sleep(SleepWell_Implementation_FrameLimit - SleepWell_Implementation_Difference);
	}

	// Try to prevent negative effect of sleep inaccuracy
	SleepWell_Implementation_PreviousTime += SleepWell_Implementation_FrameLimit;

	__asm popfd;
	__asm popad;
	__asm jmp SleepWell_Implementation_JmpBack;
}

bool SleepWell()
{
	if (!Detour::Create(SleepWell_Implementation_JmpFrom, SleepWell_Implementation_DetourSize, (unsigned long)SleepWell_Implementation))
		return false;
	
	return true;
}

// This old optimization tanks the FPS
// Disabling it by overwriting the limit (100) with -1 (Jump Greater will ALWAYS execute)
bool DisableDrawStacking()
{
	unsigned char minusOne[] = { 0xFF };
	if (!MemoryWriter::Write(0x006B7293, minusOne, 1))
		return false;

	return true;
}

bool Patches::Apply()
{
	if (!WindowedMode())
	{
		MessageBox(NULL, L"Failed to apply Windowed Mode patch", L"Patching error", MB_ICONERROR);
		return false;
	}

	if (!NativeResolution())
	{
		MessageBox(NULL, L"Failed to apply Native Resolution patch", L"Patching error", MB_ICONERROR);
		return false;
	}

	// TODO Menu effects (Sleep based timing?) & Decoupling logics/rendering
	if (!SleepWell())
	{
		MessageBox(NULL, L"Failed to apply Sleep Well patch", L"Patching error", MB_ICONERROR);
		return false;
	}

	if (!DisableDrawStacking())
	{
		MessageBox(NULL, L"Failed to apply Disable Draw Stacking patch", L"Patching error", MB_ICONERROR);
		return false;
	}

	return true;
}

void Patches::Release()
{
	if (NativeResolution_RedesignFrame_FrameBuffer != nullptr)
		delete[] NativeResolution_RedesignFrame_FrameBuffer;

#ifdef _DEBUG
	MessageBox(NULL, L"Unloading SubTitans.dll", L"Informational", MB_ICONINFORMATION);
#endif
}