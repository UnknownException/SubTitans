#pragma once
#include "patch.h"

class DDrawReplacementPatch : public Patch {
public:
	DDrawReplacementPatch();
	virtual ~DDrawReplacementPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override { return L"Failed to apply the directdraw replacement patch"; }

	unsigned long DDrawDetourAddress;
	unsigned long DInputDetourAddress;
	unsigned long WindowRegisterClassDetourAddress;
	unsigned long WindowCreateDetourAddress;
	uint32_t DInputAbsolutePositioningDetourAddress;
	bool ForceSoftwareRendering;
	bool DInputReplacement;
};