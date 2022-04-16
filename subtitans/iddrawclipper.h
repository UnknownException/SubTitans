#pragma once

namespace DDraw {
	class IDDrawClipper {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// Direct Draw Clipper
		virtual uint32_t __stdcall GetClipList(void*, void*, void*) = 0;
		virtual uint32_t __stdcall GetHWnd(void*) = 0;
		virtual uint32_t __stdcall Initialize(void*, uint32_t) = 0;
		virtual uint32_t __stdcall IsClipListChanged(uint32_t) = 0;
		virtual uint32_t __stdcall SetClipList(void*, uint32_t) = 0;
		virtual uint32_t __stdcall SetHWnd(uint32_t, void*) = 0;
	};
}