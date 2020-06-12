#pragma once
#include "patch.h"

class WindowedModePatch : public Patch {
public:
	WindowedModePatch();
	virtual ~WindowedModePatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long FlagAddress1;
	unsigned long FlagAddress2;
	unsigned long RestoreDisplayModeAddress;
};