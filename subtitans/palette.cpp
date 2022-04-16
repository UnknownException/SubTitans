#include "subtitans.h"
#include "palette.h"

using namespace DDraw;

Palette::Palette() 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	ReferenceCount = 0;

	memset(RawPalette, 0, sizeof(RawPalette));
}

Palette::~Palette() { GetLogger()->Trace("%s\n", __FUNCTION__); }

// IUnknown
uint32_t __stdcall Palette::QueryInterface(GUID* guid, void** result)
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}
uint32_t __stdcall Palette::AddRef() 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 
	
	ReferenceCount++;

	return ResultCode::Ok; 
}

uint32_t __stdcall Palette::Release()
{
	GetLogger()->Trace("%s (Remaining references %i)\n", __FUNCTION__, ReferenceCount);

	if(--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// Palette
uint32_t __stdcall Palette::GetCaps(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Palette::GetEntries(uint32_t shouldBeZero, uint32_t index, uint32_t count, uint8_t* result) 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (shouldBeZero != 0)
	{
		GetLogger()->Error("%s %s %i != 0\n", __FUNCTION__, "shouldBeZero", shouldBeZero);
		return ResultCode::InvalidArgument;
	}

	Mutex.lock();

	// BGR -> RGB
	for (uint32_t i = 0; i < count * 4; i += 4)
	{
		result[i] = RawPalette[index * 4 + i + 2];
		result[i + 1] = RawPalette[index * 4 + i + 1];
		result[i + 2] = RawPalette[index * 4 + i];
		result[i + 3] = RawPalette[index * 4 + i + 3];
	}

	Mutex.unlock();

	return ResultCode::Ok;
}

uint32_t __stdcall Palette::Initialize(void*, uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Palette::SetEntries(uint32_t shouldBeZero, uint32_t index, uint32_t count, uint8_t* input)
{
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (shouldBeZero != 0)
	{
		GetLogger()->Error("%s %s %i != 0\n", __FUNCTION__, "shouldBeZero", shouldBeZero);
		return ResultCode::InvalidArgument;
	}

	Mutex.lock();

	// RGB -> BGR
	for (uint32_t i = 0; i < count * 4; i += 4)
	{
		RawPalette[index * 4 + i] = input[i + 2];
		RawPalette[index * 4 + i + 1] = input[i + 1];
		RawPalette[index * 4 + i + 2] = input[i];
		RawPalette[index * 4 + i + 3] = input[i + 3];
	}

	Mutex.unlock();

	return ResultCode::Ok;
}

bool Palette::CreatePallete(uint32_t flags, uint8_t* palette)
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	// Not implemented
	if (flags & PaletteCapabilityFlags::Bits_1) { GetLogger()->Error("%s %s\n", __FUNCTION__, "Bits_1"); return false; }
	if (flags & PaletteCapabilityFlags::Bits_2) { GetLogger()->Error("%s %s\n", __FUNCTION__, "Bits_2"); return false; }
	if (flags & PaletteCapabilityFlags::Bits_4) { GetLogger()->Error("%s %s\n", __FUNCTION__, "Bits_4"); return false; }
	//if (flags & PaletteCapabilityFlags::Bits_8) { GetLogger()->Debug("%s %s\n", __FUNCTION__, "Bits_8"); return false; }
	if (flags & PaletteCapabilityFlags::Use8BitIndex) { GetLogger()->Error("%s %s\n", __FUNCTION__, "Use8BitIndex"); return false; }
	if (flags & PaletteCapabilityFlags::PrimarySurface) { GetLogger()->Error("%s %s\n", __FUNCTION__, "PrimarySurface"); return false; }
	if (flags & PaletteCapabilityFlags::PrimarySurfaceLeft) { GetLogger()->Error("%s %s\n", __FUNCTION__, "PrimarySurfaceLeft"); return false; }
	if (flags & PaletteCapabilityFlags::AllowFullPalette) { GetLogger()->Error("%s %s\n", __FUNCTION__, "AllowFullPalette"); return false; }
	if (flags & PaletteCapabilityFlags::VSync) { GetLogger()->Error("%s %s\n", __FUNCTION__, "VSync"); return false; }
	if (flags & PaletteCapabilityFlags::UseAlpha) { GetLogger()->Error("%s %s\n", __FUNCTION__, "UseAlpha"); return false; }

	Mutex.lock();

	// RGB -> BGR
	for (int i = 0; i < 1024; i += 4)
	{
		RawPalette[i] = palette[i+2];
		RawPalette[i+1] = palette[i+1];
		RawPalette[i+2] = palette[i];
		RawPalette[i+3] = palette[i+3];
	}

	Mutex.unlock();

	return true;
}