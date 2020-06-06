#include "subtitans.h"
#include "sleepwellpatch.h"

namespace SleepWell{
	// Detour variables
	constexpr unsigned long DetourSize = 10;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;
	static unsigned long FPSMemoryAddress = 0;

	// Function specific variables
	static unsigned long CurrentTime = timeGetTime();
	static unsigned long PreviousTime = timeGetTime();
	static unsigned long DeltaTime = 0;
	static unsigned long FrameLimit = 1000 / 25;

	__declspec(naked) void Implementation()
	{
		__asm pushad;
		__asm pushfd;

		__asm mov eax, [FPSMemoryAddress];
		__asm mov eax, [eax];
		__asm mov [FrameLimit], eax;

		CurrentTime = timeGetTime();
		DeltaTime = CurrentTime - PreviousTime;

		// Sleep (inaccurate) to prevent rendering too many frames
		if (DeltaTime < FrameLimit)
			Sleep(FrameLimit - DeltaTime);
		// Reset if 5 frames behind schedule
		else if (DeltaTime > FrameLimit * 5)
			PreviousTime = CurrentTime - FrameLimit * 2;

		// Try to prevent negative effect of sleep inaccuracy
		PreviousTime += FrameLimit;

		__asm popfd;
		__asm popad;
		__asm jmp [JmpBackAddress];
	}
}

SleepWellPatch::SleepWellPatch()
{
	GetLogger()->Informational("Initializing %s\n", __func__);

	DetourAddress = 0;
	FPSMemoryAddress = 0;
}

SleepWellPatch::~SleepWellPatch()
{
}

bool SleepWellPatch::Validate()
{
	return DetourAddress != 0 && FPSMemoryAddress != 0;
}

bool SleepWellPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	SleepWell::JmpFromAddress = DetourAddress;
	SleepWell::JmpBackAddress = SleepWell::JmpFromAddress + SleepWell::DetourSize;
	SleepWell::FPSMemoryAddress = FPSMemoryAddress;

	if (!Detour::Create(SleepWell::JmpFromAddress, SleepWell::DetourSize, (unsigned long)SleepWell::Implementation))
		return false;

	return true;
}

const wchar_t* SleepWellPatch::ErrorMessage()
{
	return L"Failed to apply Sleep Well patch";
}