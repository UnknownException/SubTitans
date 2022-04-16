#pragma once
#include "iddraw4.h"

class Device : public DDraw::IDDraw4{
public:
	Device();
	virtual ~Device();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// Direct Draw
	virtual uint32_t __stdcall Compact() override;
	virtual uint32_t __stdcall CreateClipper(uint32_t, DDraw::IDDrawClipper** result, void*) override;
	virtual uint32_t __stdcall CreatePalette(uint32_t, void*, DDraw::IDDrawPalette** result, void*) override;
	virtual uint32_t __stdcall CreateSurface(DDraw::SurfaceDescription* surfaceDescription, DDraw::IDDrawSurface4** result, void* unused) override;
	virtual uint32_t __stdcall DuplicateSurface(void*, void*) override;
	virtual uint32_t __stdcall EnumDisplayModes(uint32_t flags, void* surfaceDescription, void* appDef, DDraw::EnumDisplayModesCallBack callback) override;
	virtual uint32_t __stdcall EnumSurfaces(uint32_t, void*, void*, void*) override;
	virtual uint32_t __stdcall FlipToGDISurface() override;
	virtual uint32_t __stdcall GetCaps(DDraw::Caps* caps, DDraw::Caps* helCaps) override;
	virtual uint32_t __stdcall GetDisplayMode(void*) override;
	virtual uint32_t __stdcall GetFourCCCodes(void*, void*) override;
	virtual uint32_t __stdcall GetGDISurface(void*) override;
	virtual uint32_t __stdcall GetMonitoryFrequency(void*) override;
	virtual uint32_t __stdcall GetScanLine(void*) override;
	virtual uint32_t __stdcall GetVerticalBlankStatus(void*) override;
	virtual uint32_t __stdcall Initialize(void*) override;
	virtual uint32_t __stdcall RestoreDisplayMode() override;
	virtual uint32_t __stdcall SetCooperativeLevel(HWND hwnd, uint32_t flags) override;
	virtual uint32_t __stdcall SetDisplayMode(uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t refreshRate, uint32_t flags) override;
	virtual uint32_t __stdcall WaitForVerticalBlank(uint32_t, void*) override;
	virtual uint32_t __stdcall GetAvailableVidMem(void*, void*, void*) override;
	virtual uint32_t __stdcall GetSurfaceFromDC(void*, void*) override;
	virtual uint32_t __stdcall RestoreAllSurfaces() override;
	virtual uint32_t __stdcall TestCooperativeLevel() override;
	virtual uint32_t __stdcall GetDeviceIdentifier(void*, uint32_t) override;

	// Custom
private:
	uint32_t referenceCount;
};

uint32_t __stdcall DirectDrawCreate(void*, DDraw::IDDraw4** result, void*);