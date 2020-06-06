#include "error.h"
#include <Windows.h>
#include <string>

bool Error::WriteErrorAndReturnFalse(const wchar_t* message, unsigned long address)
{
#ifdef _DEBUG
	if (!message)
		return false;

	std::wstring errorMessage(message);
	errorMessage.append(L" Code: ");
	errorMessage.append(std::to_wstring(GetLastError()));

	MessageBox(NULL, errorMessage.c_str(), std::to_wstring(address).c_str(), 0);
#endif

	return false;
}