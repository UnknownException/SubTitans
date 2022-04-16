#include "subtitans.h"
#include "windowedmodepatch.h"

// Deprecated in favor of the OpenGL/Software rendering implementation (See DDrawReplacementPatch)

WindowedModePatch::WindowedModePatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	FlagAddress1 = 0;
	FlagAddress2 = 0;
	RestoreDisplayModeAddress = 0;
}

WindowedModePatch::~WindowedModePatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool WindowedModePatch::Validate()
{
	return FlagAddress1 != 0 && FlagAddress2 != 0 && RestoreDisplayModeAddress != 0;
}

bool WindowedModePatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	// Overwrites FULLSCREEN | NOWINDOWCHANGES | ALLOWMODEX
	unsigned char normalFlag[] = { 0x08 };
	if (!MemoryWriter::Write(FlagAddress1, normalFlag, 1))
		return false;

	// No clue about the original intention but it has to be forced to 0x08
	unsigned char movNormalFlagIntoEDX[] = { 0xBA, 0x08, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
	if (!MemoryWriter::Write(FlagAddress2, movNormalFlagIntoEDX, sizeof(movNormalFlagIntoEDX)))
		return false;

	unsigned char nopArray[] = { 0x90, 0x90, 0x90 };
	if (!MemoryWriter::Write(RestoreDisplayModeAddress, nopArray, sizeof(nopArray)))
		return false;

	return true;
}