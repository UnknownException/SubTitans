#include <Windows.h>
#include <string>
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

//	if (MessageBox(NULL, L"The found DLL is known to cause issues.\nWhen deleting this file you'll be prompted to install DirectPlay, which you should.\nDo you want to delete this file?", dll.c_str(), MB_YESNO | MB_ICONWARNING) != IDYES)
//		return false;

	std::wstring fileBackup = L"" + file + L".bak";
	MoveFileEx(file.c_str(), fileBackup.c_str(), MOVEFILE_REPLACE_EXISTING);

	return true;
}

BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			bool dplayRemoved = RemoveBadFile(L"dplay.dll"); // Fix: Internal error @ Startup
			bool dplayxRemoved = RemoveBadFile(L"dplayx.dll"); // Fix: Internal error @ Startup
			bool steamScriptRemoved = RemoveBadFile(L"steam_installscript.vdf"); // Fix: Force resetting compatibility by Steam
			bool compatDisabled = DisableCompatibilityMode(); // Fix: Internal error @ Startup
			if (dplayRemoved || dplayxRemoved || steamScriptRemoved || compatDisabled)
			{
				MessageBox(NULL, L"Please restart the game.", L"Information", MB_ICONINFORMATION);
				ExitProcess(0);
			}

			if (!Patches::Apply())
				ExitProcess(-1);

		}	break;
		case DLL_PROCESS_DETACH:
		default:
			break;
	}

	return TRUE;
}