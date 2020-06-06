#include "memorywriter.h"
#include "error.h"

bool MemoryWriter::Write(unsigned long address, unsigned char* bytes, SIZE_T length)
{
	HANDLE currentProcess = GetCurrentProcess();
	unsigned long* addressPointer = (unsigned long*)address;

	unsigned long previousProtection = 0;
	if (VirtualProtectEx(currentProcess, addressPointer, length, PAGE_EXECUTE_READWRITE, &previousProtection) == FALSE)
		return Error::WriteErrorAndReturnFalse(L"Failed to remove protection", address);

	memcpy(addressPointer, bytes, length);

	if (FlushInstructionCache(currentProcess, addressPointer, length) == FALSE)
		Error::WriteErrorAndReturnFalse(L"Failed to remove protection", address); // its ok, just continue

	if (VirtualProtectEx(currentProcess, addressPointer, length, previousProtection, &previousProtection) == FALSE)
		return Error::WriteErrorAndReturnFalse(L"Failed to remove protection", address);

	return true;
}