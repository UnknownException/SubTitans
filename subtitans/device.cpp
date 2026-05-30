#include "subtitans.h"
#include "surface.h"
#include "clipper.h"
#include "palette.h"
#include "irenderer.h"
#include "device.h"

using namespace DDraw;

Device::Device() 
{
	TRACELOG("%s\n", __FUNCTION__); 

	referenceCount = 0;
}
Device::~Device() 
{ 
	TRACELOG("%s\n", __FUNCTION__);
}

// IUnknown
uint32_t __stdcall Device::QueryInterface(GUID* guid, void** result) 
{ 
	TRACELOG("%s\n", __FUNCTION__);

	// IID_IDirectDraw4
	if (guid->Data1 == 0x9C59509A &&
		guid->Data2 == 0x39BD &&
		guid->Data3 == 0x11D1 &&
		guid->Data4[0] == 0x8C && guid->Data4[1] == 0x4A && guid->Data4[2] == 0x00 && guid->Data4[3] == 0xC0 &&
		guid->Data4[4] == 0x4F && guid->Data4[5] == 0xD9 && guid->Data4[6] == 0x30 && guid->Data4[7] == 0xC5)
	{
		*result = this;
		AddRef();

		return ResultCode::Ok;
	}

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall Device::AddRef() 
{
	TRACELOG("%s\n", __FUNCTION__);

	referenceCount++;

	return ResultCode::Ok; 
}

uint32_t __stdcall Device::Release() 
{ 
	TRACELOG("%s (Remaining references %i)\n", __FUNCTION__, referenceCount);

	if(--referenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// Direct Draw
uint32_t __stdcall Device::Compact() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Device::CreateClipper(uint32_t, IDDrawClipper** result, void*) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	*result = new Clipper();
	(*result)->AddRef();

	return ResultCode::Ok;
}

uint32_t __stdcall Device::CreatePalette(uint32_t flags, void* palette, IDDrawPalette** result, void* unused) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	*result = new Palette();
	(*result)->AddRef();

	return ((Palette*)*result)->CreatePallete(flags, (uint32_t*)palette) ? ResultCode::Ok : ResultCode::Unimplemented;
}

uint32_t __stdcall Device::CreateSurface(DDraw::SurfaceDescription* surfaceDescription, IDDrawSurface4** result, void* unused) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	*result = new Surface(surfaceDescription);
	(*result)->AddRef();

	return ResultCode::Ok;
}

uint32_t __stdcall Device::DuplicateSurface(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Device::EnumDisplayModes(uint32_t flags, void* surfaceDescription, void* appDef, EnumDisplayModesCallBack callback)
{ 
	TRACELOG("%s\n", __FUNCTION__);

	if (flags != 0)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "unexpected flags");
		return ResultCode::InvalidArgument;
	}

	if (surfaceDescription)
	{ 
		GetLogger()->Warning("%s %s\n", __FUNCTION__, "unexpected surface description");
		return ResultCode::InvalidArgument;
	}

	if (!appDef)
	{ 
		GetLogger()->Warning("%s %s\n", __FUNCTION__, "appDef is NULL");
		return ResultCode::InvalidArgument;
	}

	if (!callback)
	{ 
		GetLogger()->Warning("%s %s\n", __FUNCTION__, "missing callback");
		return ResultCode::InvalidArgument;
	}

	constexpr uint32_t bitsPerPixel[] = { 8, 32 };
	const std::vector<std::pair<uint32_t, uint32_t>> displayModes = { 
		std::make_pair(640, 480), 
		std::make_pair(800, 600), 
		std::make_pair(1024, 768), 
		std::make_pair(1280, 1024), 
		std::make_pair(Global::RenderWidth, Global::RenderHeight)
	};

	SurfaceDescription resultSurfaceDescription;
	for (auto bbp : bitsPerPixel)
	{
		for (auto displayMode : displayModes)
		{
			memset(&resultSurfaceDescription, 0, sizeof(SurfaceDescription));

			resultSurfaceDescription.size = sizeof(SurfaceDescription);
			resultSurfaceDescription.flags = SurfaceDescriptionFlag::Height | SurfaceDescriptionFlag::Width | SurfaceDescriptionFlag::Pitch
				| SurfaceDescriptionFlag::RefreshRate | SurfaceDescriptionFlag::PixelFormat;
			resultSurfaceDescription.width = displayMode.first;
			resultSurfaceDescription.height = displayMode.second;
			resultSurfaceDescription.pitch = ((displayMode.first * bbp + 31) & ~31) >> 3;
			resultSurfaceDescription.refreshRate = 0;
			resultSurfaceDescription.pixelFormat.size = sizeof(PixelFormat);
			resultSurfaceDescription.pixelFormat.flags = PixelFormatFlag::RGB;
			resultSurfaceDescription.pixelFormat.rgbBitCount = bbp;

			switch (bbp)
			{
				case 8:
					resultSurfaceDescription.pixelFormat.flags |= PixelFormatFlag::PalettedIndexed8;
					break;
				case 32:
					resultSurfaceDescription.pixelFormat.rBitMask = 0xFF0000;
					resultSurfaceDescription.pixelFormat.gBitMask = 0xFF00;
					resultSurfaceDescription.pixelFormat.bBitMask = 0xFF;
					break;
				default:
					GetLogger()->Critical("%s %ibbp not implemented!\n", __FUNCTION__, bbp);
					UNIMPLEMENTED_EXIT();
					break;
			}

			GetLogger()->Debug("%s calling callback for %ix%i %ibpp\n", __FUNCTION__, displayMode.first, displayMode.second, bbp);

			// callback 0 = stop; 1 = continue
			if (callback(&resultSurfaceDescription, appDef) == 0)
			{
				GetLogger()->Debug("%s callback early quit\n", __FUNCTION__);

				return ResultCode::Ok;
			}
		}
	}

	TRACELOG("%s finished\n", __FUNCTION__);

	return ResultCode::Ok; 
}

uint32_t __stdcall Device::EnumSurfaces(uint32_t, void*, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::FlipToGDISurface() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Device::GetCaps(Caps* caps, Caps* helCaps) 
{ 
	TRACELOG("%s\n", __FUNCTION__);

	memset(caps, 0, sizeof(Caps));
	caps->size = sizeof(Caps);

	memset(helCaps, 0, sizeof(Caps));
	helCaps->size = sizeof(Caps);

	return ResultCode::Ok;
}

uint32_t __stdcall Device::GetDisplayMode(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetFourCCCodes(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetGDISurface(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetMonitoryFrequency(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetScanLine(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetVerticalBlankStatus(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::Initialize(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Device::RestoreDisplayMode() 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	return ResultCode::Ok; 
}

uint32_t __stdcall Device::SetCooperativeLevel(HWND hwnd, uint32_t flags) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	return ResultCode::Ok;
}

uint32_t __stdcall Device::SetDisplayMode(uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t refreshRate, uint32_t flags) 
{ 
	TRACELOG("%s\n", __FUNCTION__);

	if (Global::Backend)
		Global::Backend->OnDestroyPrimarySurface();

	if (Global::VideoRequested)
		GetLogger()->Debug("Video %ix%ix%ibpp\n", width, height, bitsPerPixel);
	else
		GetLogger()->Debug("Game %ix%ix%ibpp\n", width, height, bitsPerPixel);

	Global::VideoRequested = false;

	Global::InternalWidth = width;
	Global::InternalHeight = height;
	Global::BitsPerPixel = bitsPerPixel;

	RECT rect;
	if (GetWindowRect(Global::GameWindow, &rect))
	{
		int32_t currWidth = rect.right - rect.left;
		int32_t currHeight = rect.bottom - rect.top;

		if (currWidth != Global::MonitorWidth || currHeight != Global::MonitorHeight)
			SetWindowPos(Global::GameWindow, nullptr, 0, 0, Global::MonitorWidth, Global::MonitorHeight, SWP_NOMOVE);
	}

	// RenderWindow must act as a click through overlay over the game window.
	ShowWindow(Global::RenderWindow, SW_SHOWNOACTIVATE);

	return ResultCode::Ok;
}

uint32_t __stdcall Device::WaitForVerticalBlank(uint32_t flag, void*) 
{ 
	TRACELOG("%s\n", __FUNCTION__);

	if (flag == 1)
		WaitForSingleObject(Global::VerticalBlankEvent, 65);
	else
		GetLogger()->Error("%s %s %08X\n", __FUNCTION__, "flag not implemented", flag);

	return ResultCode::Ok;
}

uint32_t __stdcall Device::GetAvailableVidMem(void*, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetSurfaceFromDC(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::RestoreAllSurfaces() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::TestCooperativeLevel() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Device::GetDeviceIdentifier(void*,uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall DirectDrawCreate(void*, IDDraw4** result, void*)
{
	*result = new Device();
	(*result)->AddRef();
	return ResultCode::Ok;
}