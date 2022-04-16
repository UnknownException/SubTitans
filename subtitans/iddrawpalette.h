#pragma once

namespace DDraw {
	namespace PaletteCapabilityFlags {
		constexpr uint32_t Bits_1 = 0x00000100;
		constexpr uint32_t Bits_2 = 0x00000200;
		constexpr uint32_t Bits_4 = 0x00000001;
		constexpr uint32_t Bits_8 = 0x00000004;
		constexpr uint32_t Use8BitIndex = 0x00000002;
		constexpr uint32_t PrimarySurface = 0x00000010;
		constexpr uint32_t PrimarySurfaceLeft = 0x00000020;
		constexpr uint32_t AllowFullPalette = 0x00000040;
		constexpr uint32_t VSync = 0x00000080;
		constexpr uint32_t UseAlpha = 0x00000400;
	};

	class IDDrawPalette {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// Direct Draw Palette
		virtual uint32_t __stdcall GetCaps(void*) = 0;
		virtual uint32_t __stdcall GetEntries(uint32_t, uint32_t, uint32_t, uint8_t*) = 0;
		virtual uint32_t __stdcall Initialize(void*, uint32_t, void*) = 0;
		virtual uint32_t __stdcall SetEntries(uint32_t, uint32_t, uint32_t, uint8_t*) = 0;
	};
}