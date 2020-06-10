#pragma once
#include "patch.h"

class PreventResizePatch : public Patch {
public:
	PreventResizePatch();
	virtual ~PreventResizePatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long SetDisplayModeDetourAddress;
	unsigned long RestoreDisplayModeAddress;
};