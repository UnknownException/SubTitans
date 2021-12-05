#pragma once
#include <Windows.h>

namespace MemoryWriter {
	bool Write(unsigned long address, unsigned char* bytes, SIZE_T length, bool enforceNoIntersecting = true);
};
