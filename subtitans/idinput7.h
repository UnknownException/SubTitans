#pragma once
#include "idinputdevice7.h"

namespace DInput {
	class IDInput7 {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// DInput7
		virtual uint32_t __stdcall CreateDevice(GUID* guid, IDInputDevice7** result, void*) = 0;
		virtual uint32_t __stdcall EnumDevices(void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall GetDeviceStatus(GUID* guid) = 0;
		virtual uint32_t __stdcall RunControlPanel(HWND, uint32_t) = 0;
		virtual uint32_t __stdcall Initialize(HINSTANCE hInstance, uint32_t) = 0;
		virtual uint32_t __stdcall FindDevice(GUID* guid, void* str, void* guid2) = 0;
		virtual uint32_t __stdcall CreateDeviceEx(GUID*, GUID, void*, void*) = 0;
	};
}