#include "subtitans.h"
#include "windowedmodepatch.h"
#include "nativeresolutionpatch.h"
#include "highdpipatch.h"
#include "sleepwellpatch.h"
#include "disabledrawstackingpatch.h"
#include "steampatchedpatcher.h"

SteamPatchedPatcher::SteamPatchedPatcher()
{
	_directDrawId = 0x399B2C1C;
	_windows7CompatibilityKeyPostfix = L"110";
}

SteamPatchedPatcher::~SteamPatchedPatcher()
{

}

void SteamPatchedPatcher::Configure()
{
	auto windowedModePatch = new WindowedModePatch();
	windowedModePatch->FlagAddress1 = 0x006B99AF; // 0x006BAC5F;
	windowedModePatch->FlagAddress2 = 0x006B9BF6; // 0x006BAEA6;
	windowedModePatch->RestoreDisplayModeAddress = 0x006B9B76; // 0x006BAE26
	_patches.push_back(windowedModePatch);

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

	auto highDPIPatch = new HighDPIPatch();
	highDPIPatch->RetrieveCursorFromWindowsMessageDetourAddress = 0x006E5009; // 0x006E62B9;
	highDPIPatch->IgnoreDInputMovementDetourAddress = 0x0071B6CC; // 0x0071C9C0;
	highDPIPatch->OverrideWindowSizeDetourAddress = 0x006B9C3E; // 0x006BAEEE;
	highDPIPatch->MouseExclusiveFlagAddress = 0x0071B32F; // 0x0071C5E4;
	highDPIPatch->CheckIfValidResolutionAddress = 0x0056D848; // 0x0056F088;
	_patches.push_back(highDPIPatch);

	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x006E5064; // 0x006E6314;
	sleepWellPatch->FrameLimitMemoryAddress = 0x00807654; // 0x00808024;
	sleepWellPatch->DisableOriginalLimiterSleepAddress = 0x006E508B; // 0x006E633B;
	_patches.push_back(sleepWellPatch);

	auto disableDrawStackingPatch = new DisableDrawStackingPatch();
	disableDrawStackingPatch->Address = 0x006B5FE3; //0x006B7293;
	_patches.push_back(disableDrawStackingPatch);
}