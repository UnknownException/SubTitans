#include <vector>
#include "memorywriter.h"

namespace MemoryWriter {
	struct _reservedAddressSpace 
	{
		unsigned long address;
		SIZE_T length;

		_reservedAddressSpace(unsigned long x, SIZE_T y)
		{
			address = x;
			length = y;
		}
	};
	static std::vector<_reservedAddressSpace> _reservedAddressSpaces;

	bool CheckIfAddressesIntersect(unsigned long left, unsigned long leftSize, unsigned long right, unsigned long rightSize)
	{
		unsigned long leftEnd = left + leftSize;
		unsigned long rightEnd = right + rightSize;
		if (left >= right && left < rightEnd)
			return true;
		else if (leftEnd > right && leftEnd < rightEnd)
			return true;
		else if (left <= right && leftEnd >= rightEnd)
			return true;

		return false;
	}

	bool Write(unsigned long address, unsigned char* bytes, SIZE_T length, bool enforceNoIntersecting)
	{
		// Prevent intersecting detours/overwrites
		bool addressesIntersect = false;
		for (auto it = _reservedAddressSpaces.begin(); it != _reservedAddressSpaces.end(); ++it)
		{
			if (CheckIfAddressesIntersect(address, length, it->address, it->length))
			{
				addressesIntersect = true;
				break;
			}
		}

		if (!addressesIntersect)
			_reservedAddressSpaces.push_back(_reservedAddressSpace(address, length));
		else if (addressesIntersect && enforceNoIntersecting)
			return false;

		HANDLE currentProcess = GetCurrentProcess();
		unsigned long* addressPointer = (unsigned long*)address;

		unsigned long previousProtection = 0;
		if (VirtualProtectEx(currentProcess, addressPointer, length, PAGE_EXECUTE_READWRITE, &previousProtection) == FALSE)
			return false;

		memcpy(addressPointer, bytes, length);

		FlushInstructionCache(currentProcess, addressPointer, length); // Failing is ok...ish

		return VirtualProtectEx(currentProcess, addressPointer, length, previousProtection, &previousProtection) == TRUE;
	}
}