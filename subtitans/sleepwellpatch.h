#pragma once
#include "patch.h"

class SleepWellPatch : public Patch {
public:
	SleepWellPatch();
	virtual ~SleepWellPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long DetourAddress;
	unsigned long FPSMemoryAddress;
};