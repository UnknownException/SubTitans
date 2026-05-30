#include "subtitans.h"
#include "nativeresolutionpatch.h"
#include "sleepwellpatch.h"
#include "ddrawreplacementpatch.h"
#include "movieheapcorruptionpatch.h"
#include "scrollpatch.h"
#include "missionskippatch.h"
#include "gogpatcher.h"

GOGPatcher::GOGPatcher(uint32_t baseOffset)
{
	_baseOffset = baseOffset;
}

GOGPatcher::~GOGPatcher()
{

}

void GOGPatcher::Configure()
{
#define RELOC_ADDR(originalAddress) originalAddress - Shared::IMAGE_BASE + _baseOffset

	Global::RenderWidth = GetConfiguration()->GetInt32(L"SETTING", L"Width", 0);
	if (Global::RenderWidth == 0)
		Global::RenderWidth = Global::MonitorWidth;

	Global::RenderHeight = GetConfiguration()->GetInt32(L"SETTING", L"Height", 0);
	if (Global::RenderHeight == 0)
		Global::RenderHeight = Global::MonitorHeight;

	auto renderingBackend = GetConfiguration()->GetInt32(L"FEATURE", L"Renderer", Global::RenderingBackend::Automatic);

	// Always enable the improved frames per second limiter when DirectDraw isn't used
	// The alternative renderers are listening to the events published by SleepWellPatch
	if (renderingBackend != Global::RenderingBackend::DirectDraw ||
		GetConfiguration()->GetBoolean(L"FIX", L"ImprovedFPSLimiter", true))
	{
		auto sleepWellPatch = new SleepWellPatch();
		sleepWellPatch->DetourAddress = RELOC_ADDR(0x006E5064);
		sleepWellPatch->FrameLimitMemoryAddress = RELOC_ADDR(0x00807654);
		sleepWellPatch->DisableOriginalLimiterSleepAddress = RELOC_ADDR(0x006E508B);
		_patches.push_back(sleepWellPatch);
	}

	if (renderingBackend == Global::RenderingBackend::DirectDraw)
	{
		GetLogger()->Informational("Using DirectDraw\n");
	}
	else
	{
		GetLogger()->Informational("Using a DirectDraw replacement\n");

		auto ddrawReplacementPatch = new DDrawReplacementPatch();
		ddrawReplacementPatch->DDrawDetourAddress = RELOC_ADDR(0x006B9981);
		ddrawReplacementPatch->DInputDetourAddress = RELOC_ADDR(0x0071B2BE);
		ddrawReplacementPatch->WindowRegisterClassDetourAddress = RELOC_ADDR(0x0056AEC7);
		ddrawReplacementPatch->WindowCreateDetourAddress = RELOC_ADDR(0x0056AF13);
		ddrawReplacementPatch->VideoFormatCheckDetourAddress = RELOC_ADDR(0x006C2A00);
		ddrawReplacementPatch->VideoScalingAddress = RELOC_ADDR(0x006C3D97);
		ddrawReplacementPatch->DInputAbsolutePositioningDetourAddress = RELOC_ADDR(0x0071B6CC);
		ddrawReplacementPatch->ForceSoftwareRendering = renderingBackend == Global::RenderingBackend::Software;
		ddrawReplacementPatch->DInputReplacement = GetConfiguration()->GetBoolean(L"FEATURE", L"CustomInput", true);
		_patches.push_back(ddrawReplacementPatch);

		Global::RetroShader = GetConfiguration()->GetBoolean(L"FEATURE", L"RetroShader", false);
	}

	if (GetConfiguration()->GetBoolean(L"FEATURE", L"NativeResolution", true))
	{
		auto nativeResolutionPatch = new NativeResolutionPatch();
		nativeResolutionPatch->GuiRescalerAddress = RELOC_ADDR(0x004F19E4);
		nativeResolutionPatch->QueueScreenAddress = RELOC_ADDR(0x004F845B);
		nativeResolutionPatch->ScreenInitialResizeWidthAddress = RELOC_ADDR(0x005307EB);
		nativeResolutionPatch->ScreenInitialResizeHeightAddress = RELOC_ADDR(0x005307F2);
		nativeResolutionPatch->ScreenResizeWidthCompareAddress = RELOC_ADDR(0x0056D78A);
		nativeResolutionPatch->ScreenResizeWidthAddress = RELOC_ADDR(0x0056D7F5);
		nativeResolutionPatch->ScreenResizeHeightAddress = RELOC_ADDR(0x0056D7FF);
		nativeResolutionPatch->GamefieldPresetWidthAddress = RELOC_ADDR(0x0056B1CC);
		nativeResolutionPatch->GamefieldPresetHeightAddress = RELOC_ADDR(0x0056B1D6);
		nativeResolutionPatch->GamefieldHeightReducingAddress = RELOC_ADDR(0x004F7057);
		nativeResolutionPatch->GamefieldHeightRestorationAddress = RELOC_ADDR(0x004FB487);
		nativeResolutionPatch->RepositionBottomMenuDetourAddress = RELOC_ADDR(0x004F6814);
		nativeResolutionPatch->RenameSettingsDetourAddress = RELOC_ADDR(0x0052F190);
		nativeResolutionPatch->RenameSettingsFunctionAddress = RELOC_ADDR(0x00711B70);
		nativeResolutionPatch->RedesignFrameDetourAddress = RELOC_ADDR(0x00543032);
		nativeResolutionPatch->RedesignFrameTeamIdMemoryAddress = RELOC_ADDR(0x0080874E);
		nativeResolutionPatch->RedesignFrameDrawFunctionAddress = RELOC_ADDR(0x00403738);
		nativeResolutionPatch->RepositionBriefingDetourAddress = RELOC_ADDR(0x004F6AB2);
		nativeResolutionPatch->CurrentScreenWidthAddress = RELOC_ADDR(0x00806730);
		_patches.push_back(nativeResolutionPatch);
	}

	if (GetConfiguration()->GetBoolean(L"FIX", L"MovieHeap", true))
	{
		auto movieHeapCorruptionPatch = new MovieHeapCorruptionPatch();
		movieHeapCorruptionPatch->AllocatedMemoryOffset = RELOC_ADDR(0x00856900);
		movieHeapCorruptionPatch->StructurePointer = RELOC_ADDR(0x006D5923 + 3);
		movieHeapCorruptionPatch->StructureOffsets[0] = RELOC_ADDR(0x006D59B0 + 1);
		movieHeapCorruptionPatch->StructureOffsets[1] = RELOC_ADDR(0x006D5A0C + 1);
		movieHeapCorruptionPatch->StructureOffsets[2] = RELOC_ADDR(0x006D5A11 + 1);
		movieHeapCorruptionPatch->StructureOffsets[3] = RELOC_ADDR(0x006D5A21 + 2);
		movieHeapCorruptionPatch->StructureOffsets[4] = RELOC_ADDR(0x006D5A2D + 2);
		movieHeapCorruptionPatch->StructureOffsets[5] = RELOC_ADDR(0x006D5A33 + 1);
		movieHeapCorruptionPatch->StructureOffsets[6] = RELOC_ADDR(0x006D5A38 + 1);
		movieHeapCorruptionPatch->StructureOffsets[7] = RELOC_ADDR(0x006D5A43 + 2);
		movieHeapCorruptionPatch->StructureOffsets[8] = RELOC_ADDR(0x006D5A4F + 2);
		movieHeapCorruptionPatch->StructureOffsets[9] = RELOC_ADDR(0x006D5A55 + 2);
		movieHeapCorruptionPatch->StructureOffsets[10] = RELOC_ADDR(0x006D5A65 + 2);
		movieHeapCorruptionPatch->StructureOffsets[11] = RELOC_ADDR(0x006D5A73 + 2);
		movieHeapCorruptionPatch->StructureOffsets[12] = RELOC_ADDR(0x006D5A79 + 3);
		movieHeapCorruptionPatch->StructureOffsets[13] = RELOC_ADDR(0x006D5A84 + 1);
		movieHeapCorruptionPatch->StructureOffsets[14] = RELOC_ADDR(0x006D5A89 + 3);
		movieHeapCorruptionPatch->StructureOffsets[15] = RELOC_ADDR(0x006D5A90 + 1);
		movieHeapCorruptionPatch->StructureOffsets[16] = RELOC_ADDR(0x006D5AB4 + 1);
		movieHeapCorruptionPatch->StructureOffsets[17] = RELOC_ADDR(0x006D5ABE + 2);
		movieHeapCorruptionPatch->StructureOffsets[18] = RELOC_ADDR(0x006D5AD8 + 1);
		movieHeapCorruptionPatch->StructureOffsets[19] = RELOC_ADDR(0x006D5AE2 + 1);
		movieHeapCorruptionPatch->StructureOffsets[20] = RELOC_ADDR(0x006D5AE7 + 2);
		movieHeapCorruptionPatch->StructureOffsets[21] = RELOC_ADDR(0x006D5B1D + 1);
		movieHeapCorruptionPatch->StructureOffsets[22] = RELOC_ADDR(0x006D5B2B + 2);
		movieHeapCorruptionPatch->StructureOffsets[23] = RELOC_ADDR(0x006D5B37 + 2);
		movieHeapCorruptionPatch->StructureOffsets[24] = RELOC_ADDR(0x006D5B41 + 2);
		movieHeapCorruptionPatch->StructureOffsets[25] = RELOC_ADDR(0x006D5BDE + 1);
		movieHeapCorruptionPatch->DetourAddress = RELOC_ADDR(0x006D599C);
		_patches.push_back(movieHeapCorruptionPatch);
	}

	if (GetConfiguration()->GetBoolean(L"FIX", L"SmoothScroll", true))
	{
		auto scrollPatch = new ScrollPatch();
		scrollPatch->UpdateRateAddress = RELOC_ADDR(0x004AB083 + 2);
		scrollPatch->DetourAddress = RELOC_ADDR(0x004AB0EF);
		scrollPatch->OriginalSpeedModifiersAddress = RELOC_ADDR(0x007AC584);
		_patches.push_back(scrollPatch);
	}

	if (GetConfiguration()->GetBoolean(L"FEATURE", L"MissionSkipCheat", true))
	{
		auto missionSkipPatch = new MissionSkipPatch();
		missionSkipPatch->AddCheatCodeDetourAddress = RELOC_ADDR(0x00522B66);
		missionSkipPatch->AddCheatCodeAlternativeReturnAddress = RELOC_ADDR(0x00522B85);
		missionSkipPatch->FullMapPathDetourAddress = RELOC_ADDR(0x00593442);
		missionSkipPatch->RelativeMapPathDetourAddress = RELOC_ADDR(0x00593476);
		missionSkipPatch->CurrentMapNameVariable = RELOC_ADDR(0x0080EF1E);
		missionSkipPatch->CheatValidationFunctionAddress = RELOC_ADDR(0x0072E6F0);
		_patches.push_back(missionSkipPatch);
	}

#undef RELOC_ADDR
}