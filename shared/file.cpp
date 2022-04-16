#include <Windows.h>
#include "file.h"

// Only Kernel32 dependent
bool File::Exists(const wchar_t* path)
{
	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(path, &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
		return false;

	FindClose(findHandle);
	return true;
}

// Only Kernel32 dependent
constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;
unsigned int File::CalculateChecksum(const wchar_t* path)
{
	HANDLE fileHandle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
	if (allocatedMemory == nullptr)
	{
		CloseHandle(fileHandle);
		return 0;
	}

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
		checkSumResult = checkSumTable[checkSumResult & 0xFF ^ *((uint8_t*)allocatedMemory + i)] ^ checkSumResult >> 8;
	}

	HeapFree(processHeap, 0, allocatedMemory);

	return ~checkSumResult;
}