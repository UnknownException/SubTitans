#pragma once
#include "patch.h"

class DisableDrawStackingPatch : public Patch {
public:
	DisableDrawStackingPatch();
	virtual ~DisableDrawStackingPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long Address;
};