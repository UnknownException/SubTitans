#include "subtitans.h"
#include <string>
#include <VersionHelpers.h>
#include "windowedmodepatch.h"
#include "nativeresolutionpatch.h"
#include "highdpipatch.h"
#include "sleepwellpatch.h"
#include "disabledrawstackingpatch.h"
#include "steampatcher.h"

namespace Steam{
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

		if (MessageBox(NULL, L"The WINXPSP3 compatibilty flag is known to cause conflicts with this game.\nDo you want to remove the key?", L"Conflicting key found", MB_YESNO | MB_ICONWARNING) != IDYES)
			return false;

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

	bool RemoveBadFile(std::wstring file)
	{
		DWORD findResult = GetFileAttributes(file.c_str());
		if (findResult == INVALID_FILE_ATTRIBUTES || findResult & FILE_ATTRIBUTE_DIRECTORY)
			return false;

		std::wstring fileBackup = L"" + file + L".bak";
		MoveFileEx(file.c_str(), fileBackup.c_str(), MOVEFILE_REPLACE_EXISTING);

		return true;
	}

	bool Windows7PaletteFix(unsigned long directDrawId)
	{
		if (IsWindows8OrGreater() || !IsWindows7OrGreater())
			return false;

		constexpr unsigned long flags = 0x00000800;
		const std::wstring stExe = L"ST.exe";

		const std::wstring registryKeyName(L"SOFTWARE\\Microsoft\\DirectDraw\\Compatibility\\SubmarineTitans");

		HKEY registryKey;
		unsigned long keyOpenResult;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryKeyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &registryKey, &keyOpenResult) != ERROR_SUCCESS)
		{
			MessageBox(NULL, L"Failed to create Window 7 compatibility key", L"Error (Create Key)", MB_ICONERROR);
			return false;
		}

		// No need to create if key exists
		if (keyOpenResult == REG_OPENED_EXISTING_KEY)
		{
			RegCloseKey(registryKey);
			return false;
		}

		bool success = true;

		if (RegSetValueEx(registryKey, L"ID", 0, REG_BINARY, (const BYTE*)&directDrawId, sizeof(unsigned long)) != ERROR_SUCCESS)
			success = false;

		if (RegSetValueEx(registryKey, L"Flags", 0, REG_BINARY, (const BYTE*)&flags, sizeof(unsigned long)) != ERROR_SUCCESS)
			success = false;

		if (RegSetValueEx(registryKey, L"Name", 0, REG_SZ, (const BYTE*)stExe.c_str(), stExe.length() * sizeof(WCHAR) + 1) != ERROR_SUCCESS)
			success = false;

		RegCloseKey(registryKey);

		if (!success)
		{
			MessageBox(NULL, L"Failed to set values for Window 7 compatibility key", L"Error (Fill Key)", MB_ICONERROR);
			if (RegDeleteKey(HKEY_LOCAL_MACHINE, registryKeyName.c_str()) != ERROR_SUCCESS)
				MessageBox(NULL, L"Failed to remove Window 7 compatibility key", L"Error (Delete Key)", MB_ICONERROR);
		}

		return success;
	}
}

SteamPatcher::SteamPatcher()
{
	_directDrawId = 0x396913d4;
}

SteamPatcher::~SteamPatcher()
{
}

bool SteamPatcher::Initialize()
{
	bool dplayRemoved = Steam::RemoveBadFile(L"dplay.dll"); // Fix: Internal error @ Startup
	bool dplayxRemoved = Steam::RemoveBadFile(L"dplayx.dll"); // Fix: Internal error @ Startup
	bool steamScriptRemoved = Steam::RemoveBadFile(L"steam_installscript.vdf"); // Fix: Force resetting compatibility by Steam
	bool compatDisabled = Steam::DisableCompatibilityMode(); // Fix: Internal error @ Startup
	bool windows7PaletteFixApplied = Steam::Windows7PaletteFix(_directDrawId); // Fix: Broken palette (Windows 7)

	if (dplayRemoved || dplayxRemoved || steamScriptRemoved || compatDisabled || windows7PaletteFixApplied)
		return false;

	return true;
}

void SteamPatcher::Configure()
{
	auto windowedModePatch = new WindowedModePatch();
	windowedModePatch->FlagAddress1 = 0x006BAC5F;
	windowedModePatch->FlagAddress2 = 0x006BAEA6;
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
	nativeResolutionPatch->MovieWidthAddress = 0x00571D55;
	nativeResolutionPatch->MovieHeightAddress = 0x00571D5C;
	nativeResolutionPatch->RepositionBottomMenuDetourAddress = 0x004F8084;
	nativeResolutionPatch->RenameSettingsDetourAddress = 0x005309D0;
	nativeResolutionPatch->RenameSettingsFunctionAddress = 0x00712E20;
	nativeResolutionPatch->RedesignFrameDetourAddress = 0x00544872;
	nativeResolutionPatch->RedesignFrameTeamIdMemoryAddress = 0x0080911E;
	nativeResolutionPatch->RedesignFrameDrawFunctionAddress = 0x0040372E;
	_patches.push_back(nativeResolutionPatch);

	auto highDPIPatch = new HighDPIPatch();
	highDPIPatch->RetrieveCursorFromWindowsMessageDetourAddress = 0x006E62B9;
	highDPIPatch->IgnoreDInputMovementDetourAddress = 0x0071C9C0;
	highDPIPatch->OverrideWindowSizeDetourAddress = 0x006BAEEE;
	highDPIPatch->CheckIfValidResolutionAddress = 0x0056F088;
	_patches.push_back(highDPIPatch);

	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x006E6253;
	sleepWellPatch->FPSMemoryAddress = 0x00808024;
	_patches.push_back(sleepWellPatch);

	auto disableDrawStackingPatch = new DisableDrawStackingPatch();
	disableDrawStackingPatch->Address = 0x006B7293;
	_patches.push_back(disableDrawStackingPatch);
}