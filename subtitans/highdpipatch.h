#pragma once
#include "patch.h"

class HighDPIPatch : public Patch {
public:
	HighDPIPatch();
	virtual ~HighDPIPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long RetrieveCursorFromWindowsMessageDetourAddress;
	unsigned long IgnoreDInputMovementDetourAddress;
	unsigned long OverrideWindowSizeDetourAddress;

	unsigned long CheckIfValidResolutionAddress;
};
