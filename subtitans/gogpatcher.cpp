#include "subtitans.h"
#include "nativeresolutionpatch.h"
#include "sleepwellpatch.h"
#include "movieheapcorruptionpatch.h"
#include "scrollpatch.h"
#include "gogpatcher.h"
#include "missionskippatch.h"

GOGPatcher::GOGPatcher()
{

}

GOGPatcher::~GOGPatcher()
{

}

void GOGPatcher::Configure()
{
	auto nativeResolutionPatch = new NativeResolutionPatch();
	nativeResolutionPatch->GuiRescalerAddress = 0x004F19E4;
	nativeResolutionPatch->QueueScreenAddress = 0x004F845B;
	nativeResolutionPatch->ScreenInitialResizeWidthAddress = 0x005307EB;
	nativeResolutionPatch->ScreenInitialResizeHeightAddress = 0x005307F2;
	nativeResolutionPatch->ScreenResizeWidthCompareAddress = 0x0056D78A;
	nativeResolutionPatch->ScreenResizeWidthAddress = 0x0056D7F5;
	nativeResolutionPatch->ScreenResizeHeightAddress = 0x0056D7FF;
	nativeResolutionPatch->GamefieldPresetWidthAddress = 0x0056B1CC;
	nativeResolutionPatch->GamefieldPresetHeightAddress = 0x0056B1D6;
	nativeResolutionPatch->GamefieldHeightReducingAddress = 0x004F7057;
	nativeResolutionPatch->GamefieldHeightRestorationAddress = 0x004FB487;
	nativeResolutionPatch->MovieWidthAddress = 0x00570515;
	nativeResolutionPatch->MovieHeightAddress = 0x0057051C;
	nativeResolutionPatch->RepositionBottomMenuDetourAddress = 0x004F6814;
	nativeResolutionPatch->RenameSettingsDetourAddress = 0x0052F190;
	nativeResolutionPatch->RenameSettingsFunctionAddress = 0x00711B70;
	nativeResolutionPatch->RedesignFrameDetourAddress = 0x00543032;
	nativeResolutionPatch->RedesignFrameTeamIdMemoryAddress = 0x0080874E;
	nativeResolutionPatch->RedesignFrameDrawFunctionAddress = 0x00403738;
	nativeResolutionPatch->RepositionBriefingDetourAddress = 0x004F6AB2;
	_patches.push_back(nativeResolutionPatch);

	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x006E5064;
	sleepWellPatch->FrameLimitMemoryAddress = 0x00807654;
	sleepWellPatch->DisableOriginalLimiterSleepAddress = 0x006E508B;
	_patches.push_back(sleepWellPatch);

	auto movieHeapCorruptionPatch = new MovieHeapCorruptionPatch();
	movieHeapCorruptionPatch->AllocatedMemoryOffset = 0x00856900;
	movieHeapCorruptionPatch->StructurePointer = 0x006D5923 + 3;
	movieHeapCorruptionPatch->StructureOffsets[0] = 0x006D59B0 + 1;
	movieHeapCorruptionPatch->StructureOffsets[1] = 0x006D5A0C + 1;
	movieHeapCorruptionPatch->StructureOffsets[2] = 0x006D5A11 + 1;
	movieHeapCorruptionPatch->StructureOffsets[3] = 0x006D5A21 + 2;
	movieHeapCorruptionPatch->StructureOffsets[4] = 0x006D5A2D + 2;
	movieHeapCorruptionPatch->StructureOffsets[5] = 0x006D5A33 + 1;
	movieHeapCorruptionPatch->StructureOffsets[6] = 0x006D5A38 + 1;
	movieHeapCorruptionPatch->StructureOffsets[7] = 0x006D5A43 + 2;
	movieHeapCorruptionPatch->StructureOffsets[8] = 0x006D5A4F + 2;
	movieHeapCorruptionPatch->StructureOffsets[9] = 0x006D5A55 + 2;
	movieHeapCorruptionPatch->StructureOffsets[10] = 0x006D5A65 + 2;
	movieHeapCorruptionPatch->StructureOffsets[11] = 0x006D5A73 + 2;
	movieHeapCorruptionPatch->StructureOffsets[12] = 0x006D5A79 + 3;
	movieHeapCorruptionPatch->StructureOffsets[13] = 0x006D5A84 + 1;
	movieHeapCorruptionPatch->StructureOffsets[14] = 0x006D5A89 + 3;
	movieHeapCorruptionPatch->StructureOffsets[15] = 0x006D5A90 + 1;
	movieHeapCorruptionPatch->StructureOffsets[16] = 0x006D5AB4 + 1;
	movieHeapCorruptionPatch->StructureOffsets[17] = 0x006D5ABE + 2;
	movieHeapCorruptionPatch->StructureOffsets[18] = 0x006D5AD8 + 1;
	movieHeapCorruptionPatch->StructureOffsets[19] = 0x006D5AE2 + 1;
	movieHeapCorruptionPatch->StructureOffsets[20] = 0x006D5AE7 + 2;
	movieHeapCorruptionPatch->StructureOffsets[21] = 0x006D5B1D + 1;
	movieHeapCorruptionPatch->StructureOffsets[22] = 0x006D5B2B + 2;
	movieHeapCorruptionPatch->StructureOffsets[23] = 0x006D5B37 + 2;
	movieHeapCorruptionPatch->StructureOffsets[24] = 0x006D5B41 + 2;
	movieHeapCorruptionPatch->StructureOffsets[25] = 0x006D5BDE + 1;
	movieHeapCorruptionPatch->DetourAddress = 0x006D599C;
	_patches.push_back(movieHeapCorruptionPatch);

	auto scrollPatch = new ScrollPatch();
	scrollPatch->UpdateRateAddress = 0x004AB083 + 2;
	scrollPatch->DetourAddress = 0x004AB0EF;
	_patches.push_back(scrollPatch);

	auto missionSkipPatch = new MissionSkipPatch();
	missionSkipPatch->AddCheatCodeDetourAddress = 0x00522B66;
	missionSkipPatch->AddCheatCodeAlternativeReturnAddress = 0x00522B85;
	missionSkipPatch->FullMapPathDetourAddress = 0x00593442;
	missionSkipPatch->RelativeMapPathDetourAddress = 0x00593476;
	missionSkipPatch->CurrentMapNameVariable = 0x0080EF1E;
	missionSkipPatch->CheatValidationFunctionAddress = 0x0072E6F0;
	_patches.push_back(missionSkipPatch);
}