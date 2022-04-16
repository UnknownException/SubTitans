#pragma once
#include "iddraw4.h"
#include "clipper.h"
#include "palette.h"

class Surface : public DDraw::IDDrawSurface4 {
public:
	Surface(DDraw::SurfaceDescription* description);
	virtual ~Surface();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// Direct Draw
	virtual uint32_t __stdcall AddAttachedSurface(void*) override;
	virtual uint32_t __stdcall AddOverlayDirtyRect(void*) override;
	virtual uint32_t __stdcall Blt(RECT* sourceRect, IDDrawSurface4* sourceSurface, RECT* destinationRect, uint32_t flags, void* unknown2) override;
	virtual uint32_t __stdcall BltBatch(void*, uint32_t, uint32_t) override;
	virtual uint32_t __stdcall BltFast(uint32_t, uint32_t, void*, void*, uint32_t) override;
	virtual uint32_t __stdcall DeleteAttachedSurface(uint32_t, void*) override;
	virtual uint32_t __stdcall EnumAttachedSurfaces(void*, void*) override;
	virtual uint32_t __stdcall EnumOverlayZOrders(uint32_t, void*, void*) override;
	virtual uint32_t __stdcall Flip(void*, uint32_t) override;
	virtual uint32_t __stdcall GetAttachedSurface(void*, void*) override;
	virtual uint32_t __stdcall GetBltStatus(uint32_t) override;
	virtual uint32_t __stdcall GetCaps(DDraw::SurfaceCaps* surfaceCaps) override;
	virtual uint32_t __stdcall GetClipper(void*) override;
	virtual uint32_t __stdcall GetColorKey(uint32_t, void*) override;
	virtual uint32_t __stdcall GetDeviceContext(HDC*) override;
	virtual uint32_t __stdcall GetFlipStatus(uint32_t) override;
	virtual uint32_t __stdcall GetOverlayPosition(void*, void*) override;
	virtual uint32_t __stdcall GetPallete(void*) override;
	virtual uint32_t __stdcall GetPixelFormat(DDraw::PixelFormat* result) override;
	virtual uint32_t __stdcall GetSurfaceDesc(DDraw::SurfaceDescription* desc) override;
	virtual uint32_t __stdcall Initialize(void*, void*) override;
	virtual uint32_t __stdcall IsLost() override;
	virtual uint32_t __stdcall Lock(RECT* rect, DDraw::SurfaceDescription* desc, uint32_t flags, void* unused) override;
	virtual uint32_t __stdcall ReleaseDeviceContext(HDC dc) override;
	virtual uint32_t __stdcall Restore() override;
	virtual uint32_t __stdcall SetClipper(DDraw::IDDrawClipper* clipper) override;
	virtual uint32_t __stdcall SetColorKey(uint32_t, void*) override;
	virtual uint32_t __stdcall SetOverlayPosition(uint32_t, uint32_t) override;
	virtual uint32_t __stdcall SetPalette(DDraw::IDDrawPalette* palette) override;
	virtual uint32_t __stdcall Unlock(RECT* rect) override;
	virtual uint32_t __stdcall UpdateOverlay(void*, void*, void*, uint32_t, void*) override;
	virtual uint32_t __stdcall UpdateOverlayDisplay(uint32_t) override;
	virtual uint32_t __stdcall UpdateOverlayZOrder(uint32_t, void*) override;
	virtual uint32_t __stdcall GetDDInterface(void*) override;
	virtual uint32_t __stdcall PageLock(uint32_t) override;
	virtual uint32_t __stdcall PageUnlock(uint32_t) override;
	virtual uint32_t __stdcall SetSurfaceDesc(void*, uint32_t) override;
	virtual uint32_t __stdcall SetPrivateData(void*, void*, uint32_t, uint32_t) override;
	virtual uint32_t __stdcall GetPrivateData(void*, void*, void*) override;
	virtual uint32_t __stdcall FreePrivateData(void*) override;
	virtual uint32_t __stdcall GetUniquenessValue(void*) override;
	virtual uint32_t __stdcall ChangeUniquenessValue() override;

	// Custom
private:
	uint32_t ReferenceCount;
	uint32_t Identifier;

public:
	int32_t Width; // Read only
	int32_t Height; // Read only
	int32_t Stride; // Read only
	uint8_t* SurfaceBuffer;

	std::mutex PrimaryDrawMutex;
	std::atomic_bool PrimaryInvalid;

	Palette* AttachedPalette;

private:
	DDraw::SurfaceDescription Description;
	Clipper* AttachedClipper;

	bool IsPrimary() { return (Description.flags & DDraw::SurfaceDescriptionFlag::Caps) && (Description.caps.caps[0] & DDraw::SurfaceCapsFlag::Primary); }
	std::pair<HDC, HGDIOBJ> MemoryDeviceContext;
};