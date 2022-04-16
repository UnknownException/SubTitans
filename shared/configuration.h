#pragma once
#include <string>

class Configuration {
	std::wstring _configFilePath;

public:
	Configuration(std::wstring configFileName);
	virtual ~Configuration();

	bool GetBoolean(std::wstring category, std::wstring key, bool placeholder = false);
	std::wstring GetString(std::wstring category, std::wstring key, std::wstring placeholder = L"");
	int32_t GetInt32(std::wstring category, std::wstring key, int32_t placeholder = 0);
};
