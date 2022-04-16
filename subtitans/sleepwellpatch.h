#pragma once
#include "patch.h"

class SleepWellPatch : public Patch {
public:
	SleepWellPatch();
	virtual ~SleepWellPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override { return L"Failed to apply Sleep Well patch"; }

	unsigned long DetourAddress;
	unsigned long FrameLimitMemoryAddress;
	unsigned long DisableOriginalLimiterSleepAddress;
};