#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <cmath>

#pragma comment(lib, "winmm.lib")

#include "../shared/gameversion.h"
#include "../shared/detour.h"
#include "../shared/memorywriter.h"
#include "../shared/logger.h"

#ifdef _DEBUG
	#pragma comment(lib, "../Debug/shared.lib")
#else
	#pragma comment(lib, "../Release/shared.lib")
#endif

Logger* GetLogger(); // Singleton