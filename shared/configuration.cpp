#include <windows.h>
#include "configuration.h"

std::wstring PrependCurrentWorkingDirectory(std::wstring configFileName)
{
	uint32_t length = GetCurrentDirectory(0, NULL);
	if (length == 0)
		return configFileName;

	WCHAR* applicationPath = new WCHAR[length + 1];
	GetCurrentDirectory(length + 1, applicationPath);
	if (length == 0)
	{
		delete[] applicationPath;
		return configFileName;
	}

	std::wstring result = std::wstring(applicationPath) + L'/' + configFileName;

	delete[] applicationPath;
	return result;
}

Configuration::Configuration(std::wstring configFileName)
{	
	_configFilePath = PrependCurrentWorkingDirectory(configFileName);
}

Configuration::~Configuration()
{

}

bool Configuration::GetBoolean(std::wstring category, std::wstring key, bool placeholder)
{
	WCHAR buffer[6];
	GetPrivateProfileString(category.c_str(), key.c_str(), placeholder ? L"true" : L"false", buffer, sizeof(buffer) / 2, _configFilePath.c_str());

	bool result = _wcsicmp(L"true", buffer) == 0;
	result |= _wcsicmp(L"yes", buffer) == 0;
	result |= _wcsicmp(L"on", buffer) == 0;
	result |= _wcsicmp(L"1", buffer) == 0;

	return result;
}

std::wstring Configuration::GetString(std::wstring category, std::wstring key, std::wstring placeholder)
{
	WCHAR buffer[64];
	GetPrivateProfileString(category.c_str(), key.c_str(), placeholder.c_str(), buffer, sizeof(buffer) / 2, _configFilePath.c_str());

	return std::wstring(buffer);
}

int32_t Configuration::GetInt32(std::wstring category, std::wstring key, int32_t placeholder)
{
	return GetPrivateProfileInt(category.c_str(), key.c_str(), placeholder, _configFilePath.c_str());
}