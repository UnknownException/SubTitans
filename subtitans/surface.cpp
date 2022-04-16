#include "subtitans.h"
#include "irenderer.h"
#include "surface.h"

using namespace DDraw;
static int s_surfaceCounter = 0;
static Surface* s_primarySurface = nullptr; // For palettes

Surface::Surface(SurfaceDescription* description) 
{ 
	Identifier = ++s_surfaceCounter;

	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	memcpy(&Description, description, sizeof(SurfaceDescription));

	PrimaryInvalid = false;
	MemoryDeviceContext = std::make_pair(nullptr, nullptr);

	if (IsPrimary())
	{
		GetLogger()->Debug("%i is the primary surface\n", Identifier);
		Width = Global::InternalWidth;
		Height = Global::InternalHeight;

		s_primarySurface = this;
	}
	else if(Description.flags & SurfaceDescriptionFlag::Height | SurfaceDescriptionFlag::Width)
	{
		Width = Description.width;
		Height = Description.height;
	}
	else
	{
		GetLogger()->Error("%s %s (%i)\n", __FUNCTION__, "unexpected non set width / height", Identifier);
		Width = Global::InternalWidth;
		Height = Global::InternalHeight;
	}

	Stride = ((Width * 8 + 31) & ~31) >> 3;

	SurfaceBuffer = new uint8_t[Stride * Height];
	memset(SurfaceBuffer, 0, Stride * Height);

	AttachedClipper = nullptr;
	AttachedPalette = nullptr;

	ReferenceCount = 0;

	if (IsPrimary() && Global::Backend)
		Global::Backend->OnCreatePrimarySurface(this);
}

Surface::~Surface()
{
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	if (IsPrimary())
	{
		if (Global::Backend)
			Global::Backend->OnDestroyPrimarySurface();

		s_primarySurface = nullptr;
	}

	if (MemoryDeviceContext.first)
	{
		auto bitmap = SelectObject(MemoryDeviceContext.first, MemoryDeviceContext.second);
		DeleteObject(bitmap);
		DeleteDC(MemoryDeviceContext.first);
	}

	if (AttachedClipper)
		AttachedClipper->Release();

	if (AttachedPalette)
		AttachedPalette->Release();

	delete[] SurfaceBuffer;
}

// IUnknown
uint32_t __stdcall Surface::QueryInterface(GUID* guid, void** result)
{
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall Surface::AddRef() 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	ReferenceCount++;

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::Release() 
{ 
	GetLogger()->Trace("%s (%i) (Remaining references %i)\n", __FUNCTION__, Identifier, ReferenceCount);

	if(--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// Direct Draw
uint32_t __stdcall Surface::AddAttachedSurface(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::AddOverlayDirtyRect(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::Blt(RECT* destinationRect, IDDrawSurface4* sourceDDSurface, RECT* sourceRect, uint32_t flags, void* bltFx)
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier); 
	constexpr unsigned long BltFxFillColorOffset = 0x50;
	constexpr unsigned long BltFlagColorFill = 0x400;

	bool colorFill = flags & BltFlagColorFill;
	unsigned long fillColor = 0;

	Surface* sourceSurface = (Surface*)sourceDDSurface;
	if (sourceSurface == nullptr && !colorFill) // Unexpected
		return ResultCode::Ok;

	if (colorFill)
	{
		sourceSurface = this;
		sourceRect = destinationRect;

		memcpy(&fillColor, (uint8_t*)bltFx + BltFxFillColorOffset, sizeof(unsigned long));
	}

	RECT sourceRectangle;
	if (sourceRect)
		sourceRectangle = *sourceRect;
	else
	{
		sourceRectangle.top = 0;
		sourceRectangle.left = 0;
		sourceRectangle.bottom = sourceSurface->Height;
		sourceRectangle.right = sourceSurface->Width;
	}

	RECT destinationRectangle;
	if (destinationRect)
		destinationRectangle = *destinationRect;
	else
	{
		destinationRectangle.top = 0;
		destinationRectangle.left = 0;
		destinationRectangle.bottom = Height;
		destinationRectangle.right = Width;
	}

	int32_t sourceWidth = sourceRectangle.right - sourceRectangle.left;
	int32_t sourceHeight = sourceRectangle.bottom - sourceRectangle.top;
	int32_t destinationWidth = destinationRectangle.right - destinationRectangle.left;
	int32_t destinationHeight = destinationRectangle.bottom - destinationRectangle.top;

	if (sourceWidth == destinationWidth && sourceHeight == destinationHeight)
	{
		PrimaryDrawMutex.lock();

		// Quick copy (Only if full width and starts at the upper left corner)
		if (sourceRectangle.top == 0 && destinationRectangle.top == 0
			&& sourceRectangle.left == 0 && destinationRectangle.left == 0
			&& destinationWidth == Width)
		{
			if (colorFill)
			{
				memset(SurfaceBuffer, (uint8_t)fillColor, sourceRectangle.bottom * Stride);
			}
			else
			{
				if (sourceSurface != this)
				{
					memcpy(SurfaceBuffer, sourceSurface->SurfaceBuffer, sourceRectangle.bottom * Stride);
				}
				else
				{
					memmove(SurfaceBuffer, sourceSurface->SurfaceBuffer, sourceRectangle.bottom * Stride);
				}
			}
		}
		else
		{
			for (int i = 0; i < destinationHeight; ++i)
			{
				if (colorFill)
				{
					memset((char*)SurfaceBuffer + Stride * (i + destinationRectangle.top) + destinationRectangle.left,
						(uint8_t)fillColor,
						destinationWidth);
				}
				else
				{
					if (sourceSurface != this)
					{
						memcpy((char*)SurfaceBuffer + Stride * (i + destinationRectangle.top) + destinationRectangle.left,
							(char*)sourceSurface->SurfaceBuffer + sourceSurface->Stride * (i + sourceRectangle.top) + sourceRectangle.left,
							destinationWidth);
					}
					else
					{
						memmove((char*)SurfaceBuffer + Stride * (i + destinationRectangle.top) + destinationRectangle.left,
							(char*)sourceSurface->SurfaceBuffer + sourceSurface->Stride * (i + sourceRectangle.top) + sourceRectangle.left,
							destinationWidth);
					}
				}
			}
		}

		PrimaryDrawMutex.unlock();

		if (PrimaryInvalid)
			PrimaryInvalid = false;
	}
	else
	{
		GetLogger()->Error("%s %s (%i, %i) -> (%i, %i)\n", __FUNCTION__, "unable to stretch blt", sourceWidth, sourceHeight, destinationWidth, destinationHeight);
	}

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::BltBatch(void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::BltFast(uint32_t, uint32_t, void*, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::DeleteAttachedSurface(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::EnumAttachedSurfaces(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::EnumOverlayZOrders(uint32_t, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::Flip(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetAttachedSurface(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetBltStatus(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetCaps(SurfaceCaps* surfaceCaps) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	memcpy(surfaceCaps, &Description.caps, sizeof(SurfaceCaps));

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::GetClipper(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetColorKey(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetDeviceContext(HDC* param1) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	BITMAPINFO* primaryBitmapInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
	memset(primaryBitmapInfo, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
	primaryBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	primaryBitmapInfo->bmiHeader.biWidth = Global::VideoWorkaround ? Global::MonitorWidth : Stride;
	primaryBitmapInfo->bmiHeader.biHeight = Global::VideoWorkaround ? -Global::MonitorHeight : -Height;
	primaryBitmapInfo->bmiHeader.biPlanes = 1;
	primaryBitmapInfo->bmiHeader.biCompression = BI_RGB;
	primaryBitmapInfo->bmiHeader.biBitCount = Global::BitsPerPixel;
	primaryBitmapInfo->bmiHeader.biClrUsed = Global::BitsPerPixel == 8 ? 256 : 0;
	primaryBitmapInfo->bmiHeader.biSizeImage = 0;
	
	if (Global::BitsPerPixel == 8)
	{
		if (AttachedPalette)
			memcpy(primaryBitmapInfo->bmiColors, AttachedPalette->RawPalette, sizeof(AttachedPalette->RawPalette));
		else if (s_primarySurface && s_primarySurface->AttachedPalette)
			memcpy(primaryBitmapInfo->bmiColors, s_primarySurface->AttachedPalette->RawPalette, sizeof(s_primarySurface->AttachedPalette->RawPalette));
		else
			GetLogger()->Error("%s %s\n", __FUNCTION__, "no attached palette found");
	}

	auto copyBufferToDIB = [this](HDC deviceContext, BITMAPINFO* bitmapInfo) {
		void* BitmapPointer = nullptr;
		auto bitmap = CreateDIBSection(deviceContext, bitmapInfo, DIB_RGB_COLORS, &BitmapPointer, NULL, 0);
		if (!bitmap)
		{
			GetLogger()->Error("%s %s\n", __FUNCTION__, "CreateDIBSection call has failed");
			return (HGDIOBJ)nullptr;
		}

		if (Global::VideoWorkaround)
			memset(BitmapPointer, 0, Global::MonitorWidth * Global::MonitorHeight);
		else
			memcpy(BitmapPointer, SurfaceBuffer, Stride * Height);

		return SelectObject(deviceContext, bitmap);
	};

	if (!MemoryDeviceContext.first)
	{
		auto deviceContext = GetDC(nullptr);
		auto memoryDeviceContext = CreateCompatibleDC(deviceContext);
		ReleaseDC(nullptr, deviceContext);

		auto oldBitmap = copyBufferToDIB(memoryDeviceContext, primaryBitmapInfo);
		MemoryDeviceContext = std::make_pair(memoryDeviceContext, oldBitmap);
	}
	else
	{
		auto oldBitmap = copyBufferToDIB(MemoryDeviceContext.first, primaryBitmapInfo);
		DeleteObject(oldBitmap);
	}


	delete[] (char*)primaryBitmapInfo;

	*param1 = MemoryDeviceContext.first;

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::GetFlipStatus(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetOverlayPosition(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetPallete(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetPixelFormat(PixelFormat* result) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	memset(result, 0, sizeof(PixelFormat));
	result->size = sizeof(PixelFormat);
	result->flags = PixelFormatFlag::RGB | PixelFormatFlag::PalettedIndexed8;
	result->rgbBitCount = 8;
	
	return ResultCode::Ok;
}

uint32_t __stdcall Surface::GetSurfaceDesc(DDraw::SurfaceDescription* desc)
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);
	if (desc->size != sizeof(DDraw::SurfaceDescription))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "unexpected size of description object", desc->size, sizeof(DDraw::SurfaceDescription));
		return ResultCode::InvalidObject;
	}

	memset(desc, 0, sizeof(DDraw::SurfaceDescription));
	desc->size = sizeof(DDraw::SurfaceDescription);
	desc->flags = SurfaceDescriptionFlag::Width | SurfaceDescriptionFlag::Height | SurfaceDescriptionFlag::Pitch 
		| SurfaceDescriptionFlag::SurfacePointer | SurfaceDescriptionFlag::PixelFormat;
	desc->width = Width;
	desc->height = Height;
	desc->pitch = Stride;
	desc->surface = SurfaceBuffer;

	return GetPixelFormat(&desc->pixelFormat);
}

uint32_t __stdcall Surface::Initialize(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::IsLost() { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::Lock(RECT* rect, DDraw::SurfaceDescription* desc, uint32_t flags, void* unused)
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	uint32_t result = GetSurfaceDesc(desc);

	if (rect != nullptr)
		desc->surface = SurfaceBuffer + rect->top * Stride + rect->left;

	return result;
}

uint32_t __stdcall Surface::ReleaseDeviceContext(HDC deviceContext) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	if (!deviceContext)
		return ResultCode::Ok;

	if (deviceContext == MemoryDeviceContext.first)
	{
		if (Global::VideoWorkaround)
		{
			auto dc = GetDC(Global::GameWindow);

			int32_t x = Global::MonitorWidth / 2 - Width / 2;
			int32_t y = Global::MonitorHeight / 2 - Height / 2;

			StretchBlt(dc, Global::GetPadding(), 0, Global::GetAspectRatioCompensatedWidth(), Global::MonitorHeight, MemoryDeviceContext.first, x, y, Width, Height, SRCCOPY);

			ReleaseDC(Global::GameWindow, dc);
		}

		// Ignore any other modifications to this DC
	}
	else
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "trying to release (possibly invalid) device context");
	}


	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::Restore() { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::SetClipper(IDDrawClipper* clipper) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	if (AttachedClipper)
		AttachedClipper->Release();

	AttachedClipper = (Clipper*)clipper;
	AttachedClipper->AddRef();

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::SetColorKey(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetOverlayPosition(uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall Surface::SetPalette(IDDrawPalette* palette) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);

	PrimaryDrawMutex.lock();

	if (AttachedPalette)
		AttachedPalette->Release();

	AttachedPalette = (Palette*)palette;
	AttachedPalette->AddRef();

	PrimaryDrawMutex.unlock();

	PrimaryInvalid = true;

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::Unlock(RECT* rect) 
{ 
	GetLogger()->Trace("%s (%i)\n", __FUNCTION__, Identifier);
	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::UpdateOverlay(void*, void*, void*, uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::UpdateOverlayDisplay(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::UpdateOverlayZOrder(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetDDInterface(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::PageLock(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::PageUnlock(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetSurfaceDesc(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetPrivateData(void*, void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetPrivateData(void*, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::FreePrivateData(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetUniquenessValue(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::ChangeUniquenessValue() { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }