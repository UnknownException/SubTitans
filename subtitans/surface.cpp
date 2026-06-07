#include "subtitans.h"
#include "irenderer.h"
#include "surface.h"

using namespace DDraw;
static int s_surfaceCounter = 0;
static Surface* s_primarySurface = nullptr; // For palettes

static SurfaceDescription CopyDescription(SurfaceDescription* description)
{
	SurfaceDescription desc;
	memcpy(&desc, description, sizeof(SurfaceDescription));
	return desc;
}

static int32_t ParseWidth(const SurfaceDescription& description, bool isPrimary, uint32_t identifier)
{
	if (isPrimary) return Global::InternalWidth;
	else if (description.flags & SurfaceDescriptionFlag::Width) return description.width;

	GetLogger()->Error("%s %s (%i)\n", __FUNCTION__, "unexpected non set width", identifier);
	return Global::InternalWidth;
}

static int32_t ParseHeight(const SurfaceDescription& description, bool isPrimary, uint32_t identifier)
{
	if (isPrimary) return Global::InternalHeight;
	else if (description.flags & SurfaceDescriptionFlag::Height) return description.height;

	GetLogger()->Error("%s %s (%i)\n", __FUNCTION__, "unexpected non set height", identifier);
	return Global::InternalHeight;

}

static int32_t ParseBitsPerPixel(const SurfaceDescription& description, bool isPrimary, uint32_t identifier)
{
	if (isPrimary) return Global::BitsPerPixel;

	if (description.flags & SurfaceDescriptionFlag::PixelFormat)
	{
		if (description.pixelFormat.rgbBitCount == 32) return 32;
		else if (description.pixelFormat.rgbBitCount == 8) return 8;
		else
		{
			GetLogger()->Error("%s %s %i (%i)\n", __FUNCTION__, "unexpected bitcount", description.pixelFormat.rgbBitCount, identifier);
			UNIMPLEMENTED_EXIT();
		}
	}

	return Global::BitsPerPixel;
}

Surface::Surface(SurfaceDescription* description)
	: Identifier(++s_surfaceCounter)
	, Description(CopyDescription(description))
	, Width(ParseWidth(Description, IsPrimary(), Identifier))
	, Height(ParseHeight(Description, IsPrimary(), Identifier))
	, BitsPerPixel(ParseBitsPerPixel(Description, IsPrimary(), Identifier))
	, BytesPerPixel(BitsPerPixel / 8)
	, Stride(((Width* BitsPerPixel + 31) & ~31) >> 3)
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	IsPrimaryValid = false;
	MemoryDeviceContext = std::make_pair(nullptr, nullptr);

	if (IsPrimary())
	{
		GetLogger()->Debug("%i is the primary surface\n", Identifier);
		s_primarySurface = this;
	}

	SurfaceBuffer = new uint8_t[Stride * Height];
	memset(SurfaceBuffer, 0, Stride * Height);

	AttachedClipper = nullptr;
	AttachedPalette = nullptr;

	ReferenceCount = 0;

	MemoryDeviceBuffer = nullptr;

	if (IsPrimary() && Global::Backend)
		Global::Backend->OnCreatePrimarySurface(this);
}

Surface::~Surface()
{
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

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
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall Surface::AddRef() 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	ReferenceCount++;

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::Release() 
{ 
	TRACELOG("%s (%i) (Remaining references %i)\n", __FUNCTION__, Identifier, ReferenceCount);

	if(--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// Direct Draw
uint32_t __stdcall Surface::AddAttachedSurface(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Surface::AddOverlayDirtyRect(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t FillColorBlt(Surface* surface, const RECT* destinationRect, const uint32_t& fillColor)
{
	RECT destinationRectangle;
	if (!destinationRect)
	{
		destinationRectangle.top = 0;
		destinationRectangle.left = 0;
		destinationRectangle.bottom = surface->Height;
		destinationRectangle.right = surface->Width;

		destinationRect = &destinationRectangle;
	}

	const int32_t destinationWidth = destinationRect->right - destinationRect->left;

	if (surface->Width < destinationRect->right || surface->Height < destinationRect->bottom)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "attempted to fill outside of surface boundaries");
		return ResultCode::Ok;
	}

	if (surface->IsPrimary())
		surface->PrimaryDrawMutex.lock();

	if (surface->BitsPerPixel == 8 || fillColor == 0)
	{
		// Quick copy (Only if full width and starts at the upper left corner)
		if (destinationRect->top == 0 && destinationRect->left == 0 && destinationWidth == surface->Width)
		{
			uint32_t size = destinationRect->bottom * surface->Stride;
			memset(surface->SurfaceBuffer, (uint8_t)fillColor, size);
		}
		else
		{
			const uint32_t destinationWidthBits = destinationWidth * surface->BytesPerPixel; // TODO uint <-> int
			const int32_t destinationHeight = destinationRect->bottom - destinationRect->top;

			uint8_t* destination = surface->SurfaceBuffer + (destinationRect->left * surface->BytesPerPixel) + surface->Stride * destinationRect->top;
			for (int i = 0; i < destinationHeight; ++i)
			{
				memset(destination, (uint8_t)fillColor, destinationWidthBits);
				destination += surface->Stride;
			}
		}
	}
#if _DEBUG
	else
	{
		// It should be sufficient as-is for normal gameplay and the videos
		GetLogger()->Debug("%s %s %ibbp %i\n", __FUNCTION__, "unable to blt fill color", surface->BitsPerPixel, fillColor);
	}
#endif

	if (surface->IsPrimary())
	{
		surface->PrimaryDrawMutex.unlock();
		surface->IsPrimaryValid = true;
	}

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::Blt(RECT* destinationRect, IDDrawSurface4* sourceDDSurface, RECT* sourceRect, uint32_t flags, void* bltFx)
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	constexpr unsigned long BltFxFillColorOffset = 0x50;
	constexpr unsigned long BltFlagColorFill = 0x400;

	const bool colorFill = flags & BltFlagColorFill;
	if (colorFill)
		return FillColorBlt(this, destinationRect, *((uint32_t*)((uint8_t*)bltFx + BltFxFillColorOffset)));

	const Surface* sourceSurface = (Surface*)sourceDDSurface;
	if (sourceSurface == nullptr)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "unable to blt without source surface");
		return ResultCode::Ok;
	}

	if (BitsPerPixel != sourceSurface->BitsPerPixel)
	{
		GetLogger()->Error("%s %s %i -> %i\n", __FUNCTION__, "unable to blt different bbp", sourceSurface->BitsPerPixel, BitsPerPixel);
		return ResultCode::Ok;
	}

	RECT sourceRectangle;
	if (!sourceRect)
	{
		sourceRectangle.top = 0;
		sourceRectangle.left = 0;
		sourceRectangle.bottom = sourceSurface->Height;
		sourceRectangle.right = sourceSurface->Width;

		sourceRect = &sourceRectangle;
	}

	RECT destinationRectangle;
	if (!destinationRect)
	{
		destinationRectangle.top = 0;
		destinationRectangle.left = 0;
		destinationRectangle.bottom = Height;
		destinationRectangle.right = Width;

		destinationRect = &destinationRectangle;
	}

	if (Width < destinationRect->right || Height < destinationRect->bottom
		|| sourceSurface->Width < sourceRect->right || sourceSurface->Height < sourceRect->bottom)
	{
		GetLogger()->Error("%s %s (%i)\n", __FUNCTION__, "attempted to write outside of surface boundaries", Identifier);
#if _DEBUG
		GetLogger()->Informational("Src rect: l:%i t:%i r:%i b:%i (%ix%i@%ibpp)\nDst rect: l:%i t:%i r:%i b:%i (%ix%i@%ibpp)\n"
			, sourceRect->left, sourceRect->top, sourceRect->right, sourceRect->bottom
				, sourceSurface->Width
				, sourceSurface->Height
				, sourceSurface->BitsPerPixel
			, destinationRect->left, destinationRect->top, destinationRect->right, destinationRect->bottom, Width, Height, BitsPerPixel
		);
#endif

		return ResultCode::Ok;
	}

	const int32_t sourceWidth = sourceRect->right - sourceRect->left;
	const int32_t sourceHeight = sourceRect->bottom - sourceRect->top;
	const int32_t destinationWidth = destinationRect->right - destinationRect->left;
	const int32_t destinationHeight = destinationRect->bottom - destinationRect->top;
	if (sourceWidth != destinationWidth || sourceHeight != destinationHeight)
	{
		GetLogger()->Error("%s %s (%i, %i) -> (%i, %i)\n", __FUNCTION__, "unable to stretch blt", sourceWidth, sourceHeight, destinationWidth, destinationHeight);
		return ResultCode::Ok;
	}

	if (IsPrimary()) 
		PrimaryDrawMutex.lock();

	// Quick copy (Only if full width and starts at the upper left corner)
	if (sourceRect->top == 0 && destinationRect->top == 0
		&& sourceRect->left == 0 && destinationRect->left == 0
		&& destinationWidth == Width)
	{
		uint32_t size = sourceRect->bottom * Stride; // TODO uint <-> int
		if (sourceSurface != this)
			memcpy(SurfaceBuffer, sourceSurface->SurfaceBuffer, size);
		else
			memmove(SurfaceBuffer, sourceSurface->SurfaceBuffer, size);
	}
	else
	{
		const uint32_t destinationWidthBits = destinationWidth * BytesPerPixel;
		uint8_t* destination = SurfaceBuffer + (destinationRect->left * BytesPerPixel) + Stride * destinationRect->top;
		uint8_t* source = sourceSurface->SurfaceBuffer + (sourceRect->left * BytesPerPixel) + sourceSurface->Stride * sourceRect->top;

		const auto copyMethod = sourceSurface == this ? memmove : memcpy;

		for (int32_t i = 0; i < destinationHeight; ++i)
		{
			copyMethod(destination, source, destinationWidthBits);
			source += sourceSurface->Stride;
			destination += Stride;
		}
	}

	if (IsPrimary())
	{
		PrimaryDrawMutex.unlock();
		IsPrimaryValid = true;
	}

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::BltBatch(void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::BltFast(uint32_t, uint32_t, void*, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::DeleteAttachedSurface(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::EnumAttachedSurfaces(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::EnumOverlayZOrders(uint32_t, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::Flip(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetAttachedSurface(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetBltStatus(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetCaps(SurfaceCaps* surfaceCaps) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	memcpy(surfaceCaps, &Description.caps, sizeof(SurfaceCaps));

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::GetClipper(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetColorKey(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetDeviceContext(HDC* param1) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);
	
	auto createBitmapInfo = [this]() {
		BITMAPINFO* primaryBitmapInfo = (BITMAPINFO*)new uint8_t[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		memset(primaryBitmapInfo, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
		primaryBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		primaryBitmapInfo->bmiHeader.biWidth = Width;
		primaryBitmapInfo->bmiHeader.biHeight = -Height;
		primaryBitmapInfo->bmiHeader.biPlanes = 1;
		primaryBitmapInfo->bmiHeader.biCompression = BI_RGB;
		primaryBitmapInfo->bmiHeader.biBitCount = BitsPerPixel;
		primaryBitmapInfo->bmiHeader.biClrUsed = BitsPerPixel == 8 ? 256 : 0;
		primaryBitmapInfo->bmiHeader.biSizeImage = 0;

		if (BitsPerPixel == 8)
		{
			if (AttachedPalette)
				memcpy(primaryBitmapInfo->bmiColors, AttachedPalette->RawPalette, sizeof(AttachedPalette->RawPalette));
			else if (s_primarySurface && s_primarySurface->AttachedPalette)
				memcpy(primaryBitmapInfo->bmiColors, s_primarySurface->AttachedPalette->RawPalette, sizeof(s_primarySurface->AttachedPalette->RawPalette));
			else
				GetLogger()->Error("%s %s\n", __FUNCTION__, "no attached palette found");
		}

		return primaryBitmapInfo;
	};

	auto createDIBSection = [this](HDC deviceContext, BITMAPINFO* bitmapInfo) {
		auto bitmap = CreateDIBSection(deviceContext, bitmapInfo, DIB_RGB_COLORS, (void**)&MemoryDeviceBuffer, NULL, 0);
		if (!bitmap)
		{
			GetLogger()->Critical("%s %s\n", __FUNCTION__, "CreateDIBSection call has failed");
			Exit();

			return (HGDIOBJ)nullptr;
		}
					
		return SelectObject(deviceContext, bitmap);
	};
	
	if (!MemoryDeviceContext.first)
	{
		auto deviceContext = GetDC(Global::GameWindow);
		auto memoryDeviceContext = CreateCompatibleDC(deviceContext);
		ReleaseDC(Global::GameWindow, deviceContext);

		auto primaryBitmapInfo = createBitmapInfo();
		auto oldBitmap = createDIBSection(memoryDeviceContext, primaryBitmapInfo);
		delete[] (uint8_t*)primaryBitmapInfo;

		MemoryDeviceContext = std::make_pair(memoryDeviceContext, oldBitmap);
	}

	memcpy(MemoryDeviceBuffer, SurfaceBuffer, Stride * Height);
	*param1 = MemoryDeviceContext.first;

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::GetFlipStatus(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetOverlayPosition(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetPallete(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::GetPixelFormat(PixelFormat* result) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	memset(result, 0, sizeof(PixelFormat));
	result->size = sizeof(PixelFormat);
	result->flags = PixelFormatFlag::RGB;
	if (BitsPerPixel == 8)
	{
		result->flags |= PixelFormatFlag::PalettedIndexed8;
		result->rgbBitCount = 8;
	}
	else
	{
		result->rgbBitCount = 32;
		result->rBitMask = 0xFF0000;
		result->gBitMask = 0xFF00;
		result->bBitMask = 0xFF;
	}
	
	return ResultCode::Ok;
}

uint32_t __stdcall Surface::GetSurfaceDesc(DDraw::SurfaceDescription* desc)
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	if (desc->size != sizeof(DDraw::SurfaceDescription))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "unexpected size of description object", desc->size, sizeof(DDraw::SurfaceDescription));
		return ResultCode::InvalidObject;
	}

	memset(desc, 0, sizeof(DDraw::SurfaceDescription));
	desc->size = sizeof(DDraw::SurfaceDescription);
	desc->flags = SurfaceDescriptionFlag::Width | SurfaceDescriptionFlag::Height | SurfaceDescriptionFlag::PixelFormat;
	desc->width = Width;
	desc->height = Height;

	return GetPixelFormat(&desc->pixelFormat);
}

uint32_t __stdcall Surface::Initialize(void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::IsLost() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::Lock(RECT* rect, DDraw::SurfaceDescription* desc, uint32_t flags, void* unused)
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	uint32_t result = GetSurfaceDesc(desc);
	if (result != ResultCode::Ok)
		return result;

	if (IsPrimary())
		PrimaryDrawMutex.lock();

	desc->flags |= SurfaceDescriptionFlag::SurfacePointer | SurfaceDescriptionFlag::Pitch;
	desc->pitch = Stride;

	if (rect != nullptr)
		desc->surface = SurfaceBuffer + rect->top * Stride + rect->left * BytesPerPixel;
	else
		desc->surface = SurfaceBuffer;

	return result;
}

uint32_t __stdcall Surface::ReleaseDeviceContext(HDC deviceContext) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	if (!deviceContext)
		return ResultCode::Ok;

	// Ignore any other modifications to this DC
	if (deviceContext != MemoryDeviceContext.first)
		GetLogger()->Error("%s %s\n", __FUNCTION__, "trying to release (possibly invalid) device context");

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::Restore() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::SetClipper(IDDrawClipper* clipper) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	if (AttachedClipper)
		AttachedClipper->Release();

	AttachedClipper = nullptr;

	if (clipper != nullptr)
	{
		AttachedClipper = (Clipper*)clipper;
		AttachedClipper->AddRef();
	}

	return ResultCode::Ok;
}

uint32_t __stdcall Surface::SetColorKey(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetOverlayPosition(uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Surface::SetPalette(IDDrawPalette* palette) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	if (IsPrimary())
		PrimaryDrawMutex.lock();

	if (AttachedPalette)
		AttachedPalette->Release();

	AttachedPalette = nullptr;

	if (palette != nullptr)
	{
		AttachedPalette = (Palette*)palette;
		AttachedPalette->AddRef();
	}

	if (IsPrimary())
	{
		PrimaryDrawMutex.unlock();
		IsPrimaryValid = false;
	}

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::Unlock(RECT* rect) 
{ 
	TRACELOG("%s (%i)\n", __FUNCTION__, Identifier);

	if (IsPrimary())
		PrimaryDrawMutex.unlock();

	return ResultCode::Ok; 
}

uint32_t __stdcall Surface::UpdateOverlay(void*, void*, void*, uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::UpdateOverlayDisplay(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::UpdateOverlayZOrder(uint32_t, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetDDInterface(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::PageLock(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::PageUnlock(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetSurfaceDesc(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::SetPrivateData(void*, void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetPrivateData(void*, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::FreePrivateData(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::GetUniquenessValue(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Surface::ChangeUniquenessValue() { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }