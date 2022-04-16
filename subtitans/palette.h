#pragma once
#include "iddraw4.h"

class Palette : public DDraw::IDDrawPalette {
public:
	Palette();
	virtual ~Palette();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// Direct Draw Palette
	virtual uint32_t __stdcall GetCaps(void*) override;
	virtual uint32_t __stdcall GetEntries(uint32_t, uint32_t, uint32_t, uint8_t*) override;
	virtual uint32_t __stdcall Initialize(void*, uint32_t, void*) override;
	virtual uint32_t __stdcall SetEntries(uint32_t, uint32_t, uint32_t, uint8_t*) override;

	// Custom
private:
	uint32_t ReferenceCount;
public:
	bool CreatePallete(uint32_t flags, uint8_t* palette);

	std::mutex Mutex;

	uint8_t RawPalette[1024];
};