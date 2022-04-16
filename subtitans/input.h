#pragma once
#include "idinput7.h"

class Input : public DInput::IDInput7 {
public:
	Input();
	virtual ~Input();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// DInput
	virtual uint32_t __stdcall CreateDevice(GUID* guid, DInput::IDInputDevice7** result, void*) override;
	virtual uint32_t __stdcall EnumDevices(void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall GetDeviceStatus(GUID* guid) override;
	virtual uint32_t __stdcall RunControlPanel(HWND, uint32_t) override;
	virtual uint32_t __stdcall Initialize(HINSTANCE hInstance, uint32_t) override;
	virtual uint32_t __stdcall FindDevice(GUID* guid, void* str, void* guid2) override;
	virtual uint32_t __stdcall CreateDeviceEx(GUID*, GUID, void*, void*) override;

	// Custom
	uint32_t ReferenceCount;
};

uint32_t __stdcall DirectInputCreate(HINSTANCE hInstance, uint32_t version, DInput::IDInput7** result, void*);
