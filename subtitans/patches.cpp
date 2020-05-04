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

bool NativeResolution()
{
	int screenWidth = 1920;
	int screenHeight = 1200;

	unsigned char buffer[4];
	
	// Create window Width (store @ 807100): Default 800
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6C5, buffer, sizeof(int)))
		return false;

	// Create window Height (store @ 807104): Default 600
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6CF, buffer, sizeof(int)))
		return false;

	// Unknown width 1
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x004F3254, buffer, sizeof(int)))
		return false;

	// Unknown width 2
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x004F9CCB, buffer, sizeof(int)))
		return false;

	// Unknown width + height 3
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0053202B, buffer, sizeof(int)))
		return false;
	
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x00532032, buffer, sizeof(int)))
		return false;

	// Unknown width 4
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056EFCA, buffer, sizeof(int)))
		return false;

	// Unknown width + height 5
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056F035, buffer, sizeof(int)))
		return false;

	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056F03F, buffer, sizeof(int)))
		return false;

	return true;
}

#pragma comment(lib, "winmm.lib")
#include <cmath>

// Detour variables
unsigned long constexpr SleepWell_Implementation_DetourSize = 10;
unsigned long constexpr SleepWell_Implementation_JmpFrom = 0x006E6253;
unsigned long constexpr SleepWell_Implementation_JmpBack = 0x006E6253 + SleepWell_Implementation_DetourSize;

// Function specific variables
static unsigned long SleepWell_Implementation_CurrentTime = timeGetTime();
static unsigned long SleepWell_Implementation_PreviousTime = timeGetTime();
static unsigned long SleepWell_Implementation_Difference = 0;
unsigned long constexpr SleepWell_Implementation_FrameLimit = 1000 / 25; // TODO: Decouple logics & rendering (clock is correct @ 25fps) <-- Seems to cause issues with some menu effects
__declspec(naked) void SleepWell_Implementation()
{
	__asm pushad;

	SleepWell_Implementation_CurrentTime = timeGetTime();
	SleepWell_Implementation_Difference = SleepWell_Implementation_CurrentTime - SleepWell_Implementation_PreviousTime;

	// Reset if way behind rendering schedule (Try to prevent negative effect of sleep inaccuracy)
	if (std::abs((long)SleepWell_Implementation_CurrentTime - (long)SleepWell_Implementation_PreviousTime) > 1000)
	{
#ifdef _DEBUG
		MessageBox(NULL, L"Rendering is behind schedule", L"FPS Limiter", MB_ICONINFORMATION);
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

	__asm popad;
	__asm jmp SleepWell_Implementation_JmpBack;
}

bool SleepWell()
{
	if (!Detour::Create(SleepWell_Implementation_JmpFrom, SleepWell_Implementation_DetourSize, (unsigned long)SleepWell_Implementation))
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

#ifdef _DEBUG
	// TODO Lots
	if (!NativeResolution())
	{
		MessageBox(NULL, L"Failed to apply Native Resolution patch", L"Patching error", MB_ICONERROR);
		return false;
	}
#endif

#ifdef _DEBUG
	// TODO Menu effects (Sleep based timing?) & Decoupling logics/rendering
	if (!SleepWell())
	{
		MessageBox(NULL, L"Failed to apply Sleep Well patch", L"Patching error", MB_ICONERROR);
		return false;
	}
#endif

	return true;
}