#include <Windows.h>
#include <math.h>
#include <cstdint>
#include "types.h"
#include "../shared/gameversion.h"
#include "injector.h"

#pragma comment(linker, "/EXPORT:D3DRMVectorModulus=_D3DRMVectorModulus@4")
extern "C" float __stdcall D3DRMVectorModulus(D3DVector3* vector)
{
	if (vector == nullptr)
		return 0.f;

	return static_cast<float>(sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z));
}

bool FileExists(const WCHAR* filePath)
{
	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(filePath, &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
		return false;

	FindClose(findHandle);
	return true;
}

constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;
uint32_t CalculateFileChecksum(WCHAR* filePath)
{
	HANDLE fileHandle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return 0;

	uint32_t allocatedSize = GetFileSize(fileHandle, NULL);

	HANDLE processHeap = GetProcessHeap();
	if (processHeap == NULL)
	{
		CloseHandle(fileHandle);

		return 0;
	}

	void* allocatedMemory = HeapAlloc(processHeap, HEAP_ZERO_MEMORY, allocatedSize);
	DWORD bytesRead = 0;
	if (!ReadFile(fileHandle, allocatedMemory, allocatedSize, &bytesRead, 0) || bytesRead != allocatedSize)
	{
		CloseHandle(fileHandle);
		HeapFree(processHeap, 0, allocatedMemory);

		return 0;
	}

	CloseHandle(fileHandle);

	uint32_t checkSumTable[256];
	for (uint32_t tableIt = 0; tableIt < sizeof(checkSumTable) / sizeof(uint32_t); ++tableIt)
	{
		checkSumTable[tableIt] = tableIt;
		for (uint32_t i = 0; i < 8; ++i)
		{
			checkSumTable[tableIt] = checkSumTable[tableIt] & 1 ?
				checkSumTable[tableIt] >> 1 ^ CRC32_POLYNOMIAL
				: checkSumTable[tableIt] >> 1;
		}
	}

	uint32_t checkSumResult = ~0;
	for (uint32_t i = 0; i < allocatedSize; ++i)
	{
		checkSumResult = checkSumTable[checkSumResult & 0xFF ^ ((uint8_t*)allocatedMemory)[i]] ^ checkSumResult >> 8;
	}

	HeapFree(processHeap, 0, allocatedMemory);

	return ~checkSumResult;
}

uint32_t GetSubTitansVersion()
{
	WCHAR applicationPath[MAX_PATH];
	GetModuleFileName(NULL, applicationPath, MAX_PATH);

	uint32_t checkSum = CalculateFileChecksum(applicationPath);
	
	switch (checkSum)
	{
		case Shared::ST_GAMEVERSION_RETAIL_UNPATCHED:
		case Shared::ST_GAMEVERSION_RETAIL_PATCHED:
		case Shared::ST_GAMEVERSION_GOG_MODIFIED:
			return checkSum;
		default:
			return 0;
	}
}

// Use d3drm for injecting custom DLL
BOOLEAN __stdcall DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	// Warning: Only use functions available in Kernel32
	if (reason == DLL_PROCESS_ATTACH)
	{
		uint32_t gameVersion = GetSubTitansVersion();
		if (gameVersion == 0)
			return FALSE;

		// Allow uninstalling patch by removing SubTitans.dll
		if (!FileExists(L"SubTitans.dll"))
			return TRUE;

		return Injector::Apply(gameVersion);
	}

	return TRUE;
}