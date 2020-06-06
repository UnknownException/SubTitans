#pragma once
#include <Windows.h>

namespace Detour {
	bool Create(unsigned long origin, SIZE_T length, unsigned long destination);
};
