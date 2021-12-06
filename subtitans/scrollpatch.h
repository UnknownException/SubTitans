#pragma once
#include "patch.h"

class ScrollPatch : public Patch {
public:
	ScrollPatch();
	virtual ~ScrollPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long UpdateRateAddress;
	unsigned long DetourAddress;
};