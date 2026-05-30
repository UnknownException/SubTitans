#include "subtitans.h"
#include "patcher.h"

Patcher::Patcher()
{
}

Patcher::~Patcher()
{
	for (auto it = _patches.begin(); it != _patches.end(); ++it)
	{
		delete *it;
	}
}

bool Patcher::Apply()
{
	for (auto it = _patches.begin(); it != _patches.end(); ++it)
	{
		auto patch = *it;
		if (!patch->Validate())
		{
			MessageBox(NULL, (*it)->ErrorMessage(), L"Patch validation error", MB_ICONERROR);
			return false;
		}

		if (!patch->Apply())
		{
			MessageBox(NULL, (*it)->ErrorMessage(), L"Patching error", MB_ICONERROR);
			return false;
		}
	}

	return true;
}

bool Patcher::RestoreVersion(uint32_t expectedVersionValue)
{
	const std::wstring registryKeyName(L"SOFTWARE\\Ellipse Studios\\Submarine Titans\\Version");
	const std::wstring versionValueName(L"version");

	HKEY registryKey;
	LSTATUS resultCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKeyName.c_str(), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &registryKey);
	if (resultCode != ERROR_SUCCESS)
	{
		GetLogger()->Warning("Insufficient rights to read or write to the version registry key. Result: %i\n", resultCode);
		return false;
	}

	DWORD versionValue = 0;
	DWORD dwSize = sizeof(versionValue);
	DWORD dwType;

	resultCode = RegQueryValueEx(registryKey, versionValueName.c_str(), NULL, &dwType, (LPBYTE)&versionValue, &dwSize);
	if (resultCode != ERROR_SUCCESS)
	{
		GetLogger()->Warning("Failed to query the version registry value. Result: %i\n", resultCode);
		return false;
	}

	bool registryModified = false;
	if (versionValue != expectedVersionValue)
	{
		resultCode = RegSetValueEx(registryKey, versionValueName.c_str(), 0, dwType, (const BYTE*)&expectedVersionValue, dwSize);

		registryModified = resultCode == ERROR_SUCCESS;
		if (!registryModified)
			GetLogger()->Warning("Failed to update the version registry key. Result: %i\n", resultCode);
	}


	RegCloseKey(registryKey);
	return registryModified;
}
