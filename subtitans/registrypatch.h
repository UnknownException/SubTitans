#pragma once
#include "patch.h"

class RegistryPatch : public Patch {
public:
	RegistryPatch();
	virtual ~RegistryPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override { return L"Failed to apply the registry patch"; }

	unsigned long DetourAddress;
};