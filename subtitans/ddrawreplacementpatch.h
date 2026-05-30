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
	unsigned long VideoScalingAddress;
	/// <summary>
	/// Optional; log which video is being prepared to play
	/// </summary>
	unsigned long VideoFormatCheckDetourAddress;
	uint32_t DInputAbsolutePositioningDetourAddress;
	bool ForceSoftwareRendering;
	bool DInputReplacement;
};