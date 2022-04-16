#include "subtitans.h"
#include "windowedmodepatch.h"
#include "nativeresolutionpatch.h"
#include "highdpipatch.h"
#include "sleepwellpatch.h"
#include "disabledrawstackingpatch.h"
#include "movieheapcorruptionpatch.h"
#include "steampatcher.h"

namespace Steam {
	bool DisableCompatibilityMode()
	{
		const std::wstring registryKeyName(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");

		WCHAR applicationPath[MAX_PATH];
		GetModuleFileName(NULL, applicationPath, MAX_PATH);

		DWORD keyBufferSize = 1024;
		WCHAR* keyBuffer = new WCHAR[keyBufferSize];
		if (RegGetValue(HKEY_CURRENT_USER, registryKeyName.c_str(), applicationPath, RRF_RT_REG_SZ, NULL, keyBuffer, &keyBufferSize) != ERROR_SUCCESS) // Non existing or not enough rights? Don't bother for now.
		{
			delete[] keyBuffer;
			return false;
		}

		const std::wstring keyContent(keyBuffer);
		delete[] keyBuffer;

		if (keyContent.find(L"WINXPSP3") == std::wstring::npos)
			return false;

//		if (MessageBox(NULL, L"The WINXPSP3 compatibilty flag is known to cause conflicts with this game.\nDo you want to remove the key?", L"Conflicting key found", MB_YESNO | MB_ICONWARNING) != IDYES)
//			return false;

		HKEY registryKey;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, registryKeyName.c_str(), 0, KEY_SET_VALUE, &registryKey) != ERROR_SUCCESS) // Non existing or not enough rights? Don't bother for now.
		{
			MessageBox(NULL, L"Failed to remove the registry key", L"Error (Open Key)", MB_ICONERROR);
			return false;
		}

		bool keyDeleted = RegDeleteValue(registryKey, applicationPath) == ERROR_SUCCESS;
		if (!keyDeleted)
			MessageBox(NULL, L"Failed to remove the registry key", L"Error (Delete Key)", MB_ICONERROR);

		RegCloseKey(registryKey);
		return keyDeleted;
	}

	bool RemoveFile(std::wstring file)
	{
		DWORD findResult = GetFileAttributes(file.c_str());
		if (findResult == INVALID_FILE_ATTRIBUTES || findResult & FILE_ATTRIBUTE_DIRECTORY)
			return false;

		std::wstring fileBackup = L"" + file + L".bak";
		MoveFileEx(file.c_str(), fileBackup.c_str(), MOVEFILE_REPLACE_EXISTING);

		return true;
	}
}

SteamPatcher::SteamPatcher()
{
}

SteamPatcher::~SteamPatcher()
{
}

bool SteamPatcher::Initialize()
{
	bool restartRequired = false;

	restartRequired |= Steam::RemoveFile(L"dplay.dll");
	restartRequired |= Steam::RemoveFile(L"dplayx.dll");
	restartRequired |= Steam::RemoveFile(L"steam_installscript.vdf");
	restartRequired |= Steam::DisableCompatibilityMode();

	return !restartRequired;
}

void SteamPatcher::Configure()
{
	auto windowedModePatch = new WindowedModePatch();
	windowedModePatch->FlagAddress1 = 0x006BAC5F;
	windowedModePatch->FlagAddress2 = 0x006BAEA6;
	windowedModePatch->RestoreDisplayModeAddress = 0x006BAE26;
	_patches.push_back(windowedModePatch);

	auto nativeResolutionPatch = new NativeResolutionPatch();
	nativeResolutionPatch->GuiRescalerAddress = 0x004F3254;
	nativeResolutionPatch->QueueScreenAddress = 0x004F9CCB;
	nativeResolutionPatch->ScreenInitialResizeWidthAddress = 0x0053202B;
	nativeResolutionPatch->ScreenInitialResizeHeightAddress = 0x00532032;
	nativeResolutionPatch->ScreenResizeWidthCompareAddress = 0x0056EFCA;
	nativeResolutionPatch->ScreenResizeWidthAddress = 0x0056F035;
	nativeResolutionPatch->ScreenResizeHeightAddress = 0x0056F03F;
	nativeResolutionPatch->GamefieldPresetWidthAddress = 0x0056CA0C;
	nativeResolutionPatch->GamefieldPresetHeightAddress = 0x0056CA16;
	nativeResolutionPatch->GamefieldHeightReducingAddress = 0x004F88C7;
	nativeResolutionPatch->GamefieldHeightRestorationAddress = 0x004FCCF7;
	nativeResolutionPatch->MovieWidthAddress = 0x00571D55;
	nativeResolutionPatch->MovieHeightAddress = 0x00571D5C;
	nativeResolutionPatch->RepositionBottomMenuDetourAddress = 0x004F8084;
	nativeResolutionPatch->RenameSettingsDetourAddress = 0x005309D0;
	nativeResolutionPatch->RenameSettingsFunctionAddress = 0x00712E20;
	nativeResolutionPatch->RedesignFrameDetourAddress = 0x00544872;
	nativeResolutionPatch->RedesignFrameTeamIdMemoryAddress = 0x0080911E;
	nativeResolutionPatch->RedesignFrameDrawFunctionAddress = 0x0040372E;
	nativeResolutionPatch->RepositionBriefingDetourAddress = 0x004F8322;
	_patches.push_back(nativeResolutionPatch);

	auto highDPIPatch = new HighDPIPatch();
	highDPIPatch->RetrieveCursorFromWindowsMessageDetourAddress = 0x006E62B9;
	highDPIPatch->IgnoreDInputMovementDetourAddress = 0x0071C9C0;
	highDPIPatch->OverrideWindowSizeDetourAddress = 0x006BAEEE;
	highDPIPatch->MouseExclusiveFlagAddress = 0x0071C5E4;
	highDPIPatch->CheckIfValidResolutionAddress = 0x0056F088;
	_patches.push_back(highDPIPatch);

	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x006E6314;
	sleepWellPatch->FrameLimitMemoryAddress = 0x00808024;
	sleepWellPatch->DisableOriginalLimiterSleepAddress = 0x006E633B;
	_patches.push_back(sleepWellPatch);

	auto disableDrawStackingPatch = new DisableDrawStackingPatch();
	disableDrawStackingPatch->Address = 0x006B7293;
	_patches.push_back(disableDrawStackingPatch);

	auto movieHeapCorruptionPatch = new MovieHeapCorruptionPatch();
	movieHeapCorruptionPatch->AllocatedMemoryOffset = 0x008572D0;
	movieHeapCorruptionPatch->StructurePointer = 0x006D6BD3 + 3;
	movieHeapCorruptionPatch->StructureOffsets[0] = 0x006D6C60 + 1;
	movieHeapCorruptionPatch->StructureOffsets[1] = 0x006D6CBC + 1;
	movieHeapCorruptionPatch->StructureOffsets[2] = 0x006D6CC1 + 1;
	movieHeapCorruptionPatch->StructureOffsets[3] = 0x006D6CD1 + 2;
	movieHeapCorruptionPatch->StructureOffsets[4] = 0x006D6CDD + 2;
	movieHeapCorruptionPatch->StructureOffsets[5] = 0x006D6CE3 + 1;
	movieHeapCorruptionPatch->StructureOffsets[6] = 0x006D6CE8 + 1;
	movieHeapCorruptionPatch->StructureOffsets[7] = 0x006D6CF3 + 2;
	movieHeapCorruptionPatch->StructureOffsets[8] = 0x006D6CFF + 2;
	movieHeapCorruptionPatch->StructureOffsets[9] = 0x006D6D05 + 2;
	movieHeapCorruptionPatch->StructureOffsets[10] = 0x006D6D15 + 2;
	movieHeapCorruptionPatch->StructureOffsets[11] = 0x006D6D23 + 2;
	movieHeapCorruptionPatch->StructureOffsets[12] = 0x006D6D29 + 3;
	movieHeapCorruptionPatch->StructureOffsets[13] = 0x006D6D34 + 1;
	movieHeapCorruptionPatch->StructureOffsets[14] = 0x006D6D39 + 3;
	movieHeapCorruptionPatch->StructureOffsets[15] = 0x006D6D40 + 1;
	movieHeapCorruptionPatch->StructureOffsets[16] = 0x006D6D64 + 1;
	movieHeapCorruptionPatch->StructureOffsets[17] = 0x006D6D6E + 2;
	movieHeapCorruptionPatch->StructureOffsets[18] = 0x006D6D88 + 1;
	movieHeapCorruptionPatch->StructureOffsets[19] = 0x006D6D92 + 1;
	movieHeapCorruptionPatch->StructureOffsets[20] = 0x006D6D97 + 2;
	movieHeapCorruptionPatch->StructureOffsets[21] = 0x006D6DCD + 1;
	movieHeapCorruptionPatch->StructureOffsets[22] = 0x006D6DDB + 2;
	movieHeapCorruptionPatch->StructureOffsets[23] = 0x006D6DE7 + 2;
	movieHeapCorruptionPatch->StructureOffsets[24] = 0x006D6DF1 + 2;
	movieHeapCorruptionPatch->StructureOffsets[25] = 0x006D6E8E + 1;
	movieHeapCorruptionPatch->DetourAddress = 0x006D6C4C;
	_patches.push_back(movieHeapCorruptionPatch);
}