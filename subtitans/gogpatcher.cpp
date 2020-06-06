#include "subtitans.h"
#include "nativeresolutionpatch.h"
//#include "disabledrawstackingpatch.h"
#include "gogpatcher.h"

GOGPatcher::GOGPatcher()
{

}

GOGPatcher::~GOGPatcher()
{

}

void GOGPatcher::Configure()
{
	// (Windowed mode) not required (gog ddraw)

	auto nativeResolutionPatch = new NativeResolutionPatch();
	nativeResolutionPatch->GuiRescalerAddress = 0x004F19E4; // 0x004F3254;
	nativeResolutionPatch->QueueScreenAddress = 0x004F845B; //0x004F9CCB;
	nativeResolutionPatch->ScreenInitialResizeWidthAddress = 0x005307EB; //0x0053202B;
	nativeResolutionPatch->ScreenInitialResizeHeightAddress = 0x005307F2; // 0x00532032;
	nativeResolutionPatch->ScreenResizeWidthCompareAddress = 0x0056D78A; // 0x0056EFCA;
	nativeResolutionPatch->ScreenResizeWidthAddress = 0x0056D7F5; // 0x0056F035;
	nativeResolutionPatch->ScreenResizeHeightAddress = 0x0056D7FF; // 0x0056F03F;
	nativeResolutionPatch->GamefieldPresetWidthAddress = 0x0056B1CC; //0x0056CA0C;
	nativeResolutionPatch->GamefieldPresetHeightAddress = 0x0056B1D6; //0x0056CA16;
	nativeResolutionPatch->MovieWidthAddress = 0x00570515; // 0x00571D55;
	nativeResolutionPatch->MovieHeightAddress = 0x0057051C; // 0x00571D5C;
	nativeResolutionPatch->RepositionBottomMenuDetourAddress = 0x004F6814; // 0x004F8084;
	nativeResolutionPatch->RenameSettingsDetourAddress = 0x0052F190; // 0x005309D0;
	nativeResolutionPatch->RenameSettingsFunctionAddress = 0x00711B70; // 0x00712E20;
	nativeResolutionPatch->RedesignFrameDetourAddress = 0x00543032; // 0x00544872;
	nativeResolutionPatch->RedesignFrameTeamIdMemoryAddress = 0x0080874E; // 0x0080911E;
	nativeResolutionPatch->RedesignFrameDrawFunctionAddress = 0x00403738; //0x0040372E;
	_patches.push_back(nativeResolutionPatch);

	// (HighDPI) GOG seems to affect high dpi resolutions; ignore this for now

	// (SleepWell) Implementation seems to require some attention for high speed setting; not important for the gog release right now

	// (DisableDrawStacking) Doesn't result in a noticable improvement for GOG version (gog ddraw)
	//auto disableDrawStackingPatch = new DisableDrawStackingPatch();
	//disableDrawStackingPatch->Address = 0x006B5FE3; //0x006B7293;
	//_patches.push_back(disableDrawStackingPatch);
}