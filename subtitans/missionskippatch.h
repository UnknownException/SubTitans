#pragma once
#include "patch.h"

class MissionSkipPatch : public Patch {
public:
	MissionSkipPatch();
	virtual ~MissionSkipPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override { return L"Failed to apply the mission skip patch"; }

	unsigned long AddCheatCodeDetourAddress;
	unsigned long AddCheatCodeAlternativeReturnAddress;
	unsigned long FullMapPathDetourAddress;
	unsigned long RelativeMapPathDetourAddress;

	unsigned long CurrentMapNameVariable;
	unsigned long CheatValidationFunctionAddress;
};