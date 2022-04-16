#pragma once
#include "iddrawsurface4.h"
#include "iddrawclipper.h"
#include "iddrawpalette.h"

namespace DDraw {
	namespace PixelFormatFlag {
		constexpr uint32_t PalettedIndexed8 = 0x00000020;
		constexpr uint32_t RGB = 0x00000040;
	}

	typedef uint32_t(__stdcall* EnumDisplayModesCallBack)(void*, void*);

#pragma pack(push, 1)
	struct _ExternalDeviceCaps {
		uint32_t caps;
		uint32_t colorKeycaps;
		uint32_t effectCaps;
		uint32_t rops[8];
	};

	struct _ExternalDeviceCapsExt {
		uint32_t caps;
		_ExternalDeviceCaps base;
	};

	struct Caps { // 380
		uint32_t size;
		uint32_t caps[2];
		uint32_t colorKeyCaps;
		uint32_t effectCaps;
		uint32_t effectAlphaCaps;
		uint32_t palleteCaps;
		uint32_t stereoVisionCaps;
		uint32_t alphaBltConstantBitDepth;
		uint32_t alphaBltPixelBitDepth;
		uint32_t alphaBltSurfaceBitDepth;
		uint32_t alphaOverlayConstantBitDepth;
		uint32_t alphaOverlayPixelBitDepth;
		uint32_t alphaOverlaySurfaceBitDepth;
		uint32_t zBufferBitDepth;
		uint32_t memoryTotal;
		uint32_t memoryFree;
		uint32_t maximumVisibleOverlays;
		uint32_t currentVisibleOverlays;
		uint32_t fourCCCodesCount;
		uint32_t alignBoundarySource;
		uint32_t alignSizeSource;
		uint32_t alignBoundaryDestination;
		uint32_t alignSizeDestination;
		uint32_t alignStride;
		uint32_t rops[8];
		uint32_t legacyCaps;
		uint32_t minimumOverlayStretch;
		uint32_t maximumOverlayStretch;
		uint32_t minimumLiveVideoStretch;
		uint32_t maximumLiveVideoStretch;
		uint32_t minimumHardwareCodecStretch;
		uint32_t maximumHardwareCodecStretch;
		uint32_t reserved[3];
		_ExternalDeviceCaps systemToVirtualMemoryCaps;
		_ExternalDeviceCaps virtualMemoryToSystemCaps;
		_ExternalDeviceCaps systemToSystemCaps;
		uint32_t maximumVideoPorts;
		uint32_t currentVideoPorts;
		uint32_t systemToVirtualMemoryCaps2;
		_ExternalDeviceCapsExt nonLocalToLocalCaps;
		SurfaceCaps surfaceCaps;
	};
#pragma pack(pop)

	class IDDraw4 {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// Direct Draw
		virtual uint32_t __stdcall Compact() = 0;
		virtual uint32_t __stdcall CreateClipper(uint32_t, IDDrawClipper** result, void*) = 0;
		virtual uint32_t __stdcall CreatePalette(uint32_t flags, void* palette, IDDrawPalette** result, void* unused) = 0;
		virtual uint32_t __stdcall CreateSurface(SurfaceDescription* surfaceDescription, IDDrawSurface4** result, void* unused) = 0;
		virtual uint32_t __stdcall DuplicateSurface(void*, void*) = 0;
		virtual uint32_t __stdcall EnumDisplayModes(uint32_t flags, void* surfaceDescription, void* appDef, EnumDisplayModesCallBack callback) = 0;
		virtual uint32_t __stdcall EnumSurfaces(uint32_t, void*, void*, void*) = 0;
		virtual uint32_t __stdcall FlipToGDISurface() = 0;
		virtual uint32_t __stdcall GetCaps(Caps* caps, Caps* helCaps) = 0;
		virtual uint32_t __stdcall GetDisplayMode(void*) = 0;
		virtual uint32_t __stdcall GetFourCCCodes(void*, void*) = 0;
		virtual uint32_t __stdcall GetGDISurface(void*) = 0;
		virtual uint32_t __stdcall GetMonitoryFrequency(void*) = 0;
		virtual uint32_t __stdcall GetScanLine(void*) = 0;
		virtual uint32_t __stdcall GetVerticalBlankStatus(void*) = 0;
		virtual uint32_t __stdcall Initialize(void*) = 0;
		virtual uint32_t __stdcall RestoreDisplayMode() = 0;
		virtual uint32_t __stdcall SetCooperativeLevel(HWND hwnd, uint32_t flags) = 0;
		virtual uint32_t __stdcall SetDisplayMode(uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t refreshRate, uint32_t flags) = 0;
		virtual uint32_t __stdcall WaitForVerticalBlank(uint32_t, void*) = 0;
		virtual uint32_t __stdcall GetAvailableVidMem(void*, void*, void*) = 0;
		virtual uint32_t __stdcall GetSurfaceFromDC(void*, void*) = 0;
		virtual uint32_t __stdcall RestoreAllSurfaces() = 0;
		virtual uint32_t __stdcall TestCooperativeLevel() = 0;
		virtual uint32_t __stdcall GetDeviceIdentifier(void*, uint32_t) = 0;
	};
}