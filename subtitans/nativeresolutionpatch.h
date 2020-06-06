#pragma once
#include "patch.h"

class NativeResolutionPatch : public Patch {
public:
	NativeResolutionPatch();
	virtual ~NativeResolutionPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long GuiRescalerAddress;

	unsigned long QueueScreenAddress;

	unsigned long ScreenInitialResizeWidthAddress;
	unsigned long ScreenInitialResizeHeightAddress;

	unsigned long ScreenResizeWidthCompareAddress;
	unsigned long ScreenResizeWidthAddress;
	unsigned long ScreenResizeHeightAddress;

	unsigned long GamefieldPresetWidthAddress;
	unsigned long GamefieldPresetHeightAddress;

	unsigned long MovieWidthAddress;
	unsigned long MovieHeightAddress;

	unsigned long RepositionBottomMenuDetourAddress;
	unsigned long RenameSettingsDetourAddress;
	unsigned long RenameSettingsFunctionAddress;
	unsigned long RedesignFrameDetourAddress;
	unsigned long RedesignFrameTeamIdMemoryAddress;
	unsigned long RedesignFrameDrawFunctionAddress;
};