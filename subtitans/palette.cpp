#include "subtitans.h"
#include "palette.h"

using namespace DDraw;

Palette::Palette() 
{ 
	TRACELOG("%s\n", __FUNCTION__); 

	ReferenceCount = 0;

	memset(RawPalette, 0, sizeof(RawPalette));
}

Palette::~Palette() { TRACELOG("%s\n", __FUNCTION__); }

// IUnknown
uint32_t __stdcall Palette::QueryInterface(GUID* guid, void** result)
{
	TRACELOG("%s\n", __FUNCTION__);

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}
uint32_t __stdcall Palette::AddRef() 
{ 
	TRACELOG("%s\n", __FUNCTION__); 
	
	ReferenceCount++;

	return ResultCode::Ok; 
}

uint32_t __stdcall Palette::Release()
{
	TRACELOG("%s (Remaining references %i)\n", __FUNCTION__, ReferenceCount);

	if(--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// Palette
uint32_t __stdcall Palette::GetCaps(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Palette::GetEntries(uint32_t shouldBeZero, uint32_t index, uint32_t count, uint32_t* result) 
{ 
	TRACELOG("%s\n", __FUNCTION__); 

	if (shouldBeZero != 0)
	{
		GetLogger()->Error("%s %s %i != 0\n", __FUNCTION__, "shouldBeZero", shouldBeZero);
		return ResultCode::InvalidArgument;
	}

	if (index > 255 || count > 256 - index)
	{
		GetLogger()->Error("%s %s index:%i count:%i\n", __FUNCTION__, "requested palette range is out of bounds", index, count);
		return ResultCode::InvalidArgument;
	}

	Mutex.lock();

	// BGR -> RGB
	for (uint32_t i = 0; i < count; ++i)
	{
		const uint32_t pixel = RawPalette[index + i];
		result[i] = (pixel & 0xFF00FF00)
			| ((pixel << 16) & 0x00FF0000)
			| ((pixel >> 16) & 0x000000FF);
	}

	Mutex.unlock();

	return ResultCode::Ok;
}

uint32_t __stdcall Palette::Initialize(void*, uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Palette::SetEntries(uint32_t shouldBeZero, uint32_t index, uint32_t count, uint32_t* input)
{
	TRACELOG("%s\n", __FUNCTION__); 

	if (shouldBeZero != 0)
	{
		GetLogger()->Error("%s %s %i != 0\n", __FUNCTION__, "shouldBeZero", shouldBeZero);
		return ResultCode::InvalidArgument;
	}

	if (index > 255 || count > 256 - index)
	{
		GetLogger()->Error("%s %s index:%i count:%i\n", __FUNCTION__, "requested palette range is out of bounds", index, count);
		return ResultCode::InvalidArgument;
	}

	Mutex.lock();

	// RGB -> BGR
	for (uint32_t i = 0; i < count; ++i)
	{
		const uint32_t pixel = input[i];
		RawPalette[index + i] = (pixel & 0xFF00FF00)
			| ((pixel << 16) & 0x00FF0000)
			| ((pixel >> 16) & 0x000000FF);
	}

	Mutex.unlock();

	return ResultCode::Ok;
}

bool Palette::CreatePallete(uint32_t flags, uint32_t* palette)
{
	TRACELOG("%s\n", __FUNCTION__);

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
	for (uint32_t i = 0; i < 256; ++i)
	{
		const uint32_t pixel = palette[i];
		RawPalette[i] = (pixel & 0xFF00FF00)
			| ((pixel << 16) & 0x00FF0000)
			| ((pixel >> 16) & 0x000000FF);
	}

	Mutex.unlock();

	return true;
}