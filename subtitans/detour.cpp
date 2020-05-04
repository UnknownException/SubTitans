#include "detour.h"
#include "memorywriter.h"
#include "error.h"

bool Detour::Create(unsigned long origin, SIZE_T length, unsigned long destination)
{
	if (length < 5)
		return Error::WriteErrorAndReturnFalse(L"Detour length should be bigger or equal 5", origin);

	unsigned char* newBytes = new unsigned char[length];
	memset(newBytes, 0x90, length); // Fill with nops
	newBytes[0] = 0xE9; // Jump instruction

	unsigned long jumpDistance = destination - origin - 5;
	memcpy(&newBytes[1], &jumpDistance, sizeof(unsigned long)); // Jump distance

	if (!MemoryWriter::Write(origin, newBytes, length))
	{
		delete[] newBytes;
		return Error::WriteErrorAndReturnFalse(L"Failed to create detour", origin);
	}

	delete[] newBytes;
	return true;
}