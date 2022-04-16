#pragma once
#include <cstdint>

namespace File {
	bool Exists(const wchar_t* path); // Only Kernel32 dependent
	uint32_t CalculateChecksum(const wchar_t* path); // Only Kernel32 dependent
}