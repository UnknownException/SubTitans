#include <Windows.h>
#include <string>
#include <VersionHelpers.h>
#include "patches.h"

bool DisableCompatibilityMode()
{
	std::wstring registryKeyName(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");

	WCHAR applicationPath[MAX_PATH];
	GetModuleFileName(NULL, applicationPath, MAX_PATH);

	DWORD keyBufferSize = 1024;
	WCHAR* keyBuffer = new WCHAR[keyBufferSize];
	if (RegGetValue(HKEY_CURRENT_USER, registryKeyName.c_str(), applicationPath, RRF_RT_REG_SZ, NULL, keyBuffer, &keyBufferSize) != ERROR_SUCCESS) // Non existing or not enough rights? Don't bother for now.
	{
		delete[] keyBuffer;
		return false;
	}

	std::wstring keyContent(keyBuffer);
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

bool Windows7PaletteFix()
{
	if (IsWindows8OrGreater() || !IsWindows7OrGreater())
		return false;

	const unsigned long directDrawId = 0x396913d4;
	const unsigned long flags = 0x00000800;
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

#pragma comment(linker, "/EXPORT:InitializeLibrary=_InitializeLibrary@0")
extern "C" void __stdcall InitializeLibrary()
{
	bool dplayRemoved = RemoveBadFile(L"dplay.dll"); // Fix: Internal error @ Startup
	bool dplayxRemoved = RemoveBadFile(L"dplayx.dll"); // Fix: Internal error @ Startup
	bool steamScriptRemoved = RemoveBadFile(L"steam_installscript.vdf"); // Fix: Force resetting compatibility by Steam
	bool compatDisabled = DisableCompatibilityMode(); // Fix: Internal error @ Startup
	bool windows7PaletteFixApplied = Windows7PaletteFix(); // Fix: Broken palette (Windows 7)
	if (dplayRemoved || dplayxRemoved || steamScriptRemoved || compatDisabled || windows7PaletteFixApplied)
	{
		MessageBox(NULL, L"Please restart the game.", L"Information", MB_ICONINFORMATION);
		ExitProcess(0);
	}

	if (!Patches::Apply())
		ExitProcess(-1);
}

#pragma comment(linker, "/EXPORT:ReleaseLibrary=_ReleaseLibrary@0")
extern "C" void __stdcall ReleaseLibrary()
{
	Patches::Release();
}

BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	return TRUE;
}