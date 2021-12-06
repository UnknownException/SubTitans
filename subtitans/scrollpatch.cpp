#include "subtitans.h"
#include "scrollpatch.h"

namespace ScrollSpeed {
	// Detour variables
	constexpr unsigned long DetourSize = 7;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;
	
	// Function specific variables
	const float SpeedMultiplier = 0.76f;
	const float SpeedLevels[3] = { 4.5f * SpeedMultiplier, 3.0f * SpeedMultiplier, 1.5f * SpeedMultiplier };

	__declspec(naked) void Implementation()
	{
		__asm pushfd; // store eflags previous cmp

		__asm cmp edx, 2;
		__asm jg UNEXPECTED_RANGE;

		__asm fmul dword ptr ds:[edx * 0x04 + SpeedLevels];

		__asm popfd;
		__asm jmp [JmpBackAddress];

	UNEXPECTED_RANGE:
		__asm fmul dword ptr ds:[edx * 0x04 + 0x7AC584]; // original code

		__asm popfd;
		__asm jmp [JmpBackAddress];
	}
}

ScrollPatch::ScrollPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	UpdateRateAddress = 0;
	DetourAddress = 0;
}

ScrollPatch::~ScrollPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool ScrollPatch::Validate()
{
	return UpdateRateAddress != 0 && DetourAddress != 0;
}

bool ScrollPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	// Update at least as fast as the normal game speed
	unsigned char updateRate[] = { 0x26 };
	if (!MemoryWriter::Write(UpdateRateAddress, updateRate, sizeof(updateRate)))
		return false;

	// Revert movement speed by changing the fast/med/slow movement presets
	ScrollSpeed::JmpFromAddress = DetourAddress;
	ScrollSpeed::JmpBackAddress = ScrollSpeed::JmpFromAddress + ScrollSpeed::DetourSize;
	if (!Detour::Create(ScrollSpeed::JmpFromAddress, ScrollSpeed::DetourSize, (unsigned long)ScrollSpeed::Implementation))
		return false;

	return true;
}

const wchar_t* ScrollPatch::ErrorMessage()
{
	return L"Failed to apply the scroll patch";
}
