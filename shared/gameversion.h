#pragma once

namespace Shared {
	// Technology Demo
	constexpr unsigned long ST_GAMEVERSION_0_0_6 = 0x15C56BB2;
	// Demo
	constexpr unsigned long ST_GAMEVERSION_0_1_6 = 0x6B08028C;
	// Retail, Steam
	constexpr unsigned long ST_GAMEVERSION_1_0_0 = 0x826D8625;
	// Retail
	constexpr unsigned long ST_GAMEVERSION_1_1_0 = 0xB5BD8D15;
	// GOG
	constexpr unsigned long ST_GAMEVERSION_1_1_0_GOG = 0x18C26D0A;
	// Retail - DYNAMICBASE (ASLR) and NXCOMPAT (DEP) flags enabled
	constexpr unsigned long ST_GAMEVERSION_1_1_0_ASLR_DEP = 0x0191DC88;
	// GOG - DYNAMICBASE (ASLR) and NXCOMPAT (DEP) flags enabled
	constexpr unsigned long ST_GAMEVERSION_1_1_0_GOG_ASLR_DEP = 0xACEE3C97;

	// Who cares? perhaps I should remove this
	constexpr unsigned long ST_LANGUAGE_ENGLISH_UNPATCHED = 0xCE56F1FC; // 1.0 English
	constexpr unsigned long ST_LANGUAGE_ENGLISH_PATCHED = 0x0EB5715F; // 1.1 English
	constexpr unsigned long ST_LANGUAGE_ENGLISH_DEMO = 0xE73A5959; // Demo English

	constexpr unsigned long IMAGE_BASE = 0x00400000;
}