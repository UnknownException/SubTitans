#pragma once
#include "iddraw4.h"

class Clipper : public DDraw::IDDrawClipper {
public:
	Clipper();
	virtual ~Clipper();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// Direct Draw Clipper
	virtual uint32_t __stdcall GetClipList(void*, void*, void*) override;
	virtual uint32_t __stdcall GetHWnd(void*) override;
	virtual uint32_t __stdcall Initialize(void*, uint32_t) override;
	virtual uint32_t __stdcall IsClipListChanged(uint32_t) override;
	virtual uint32_t __stdcall SetClipList(void*, uint32_t) override;
	virtual uint32_t __stdcall SetHWnd(uint32_t, void*) override;

	// Custom
private:
	uint32_t referenceCount;
};