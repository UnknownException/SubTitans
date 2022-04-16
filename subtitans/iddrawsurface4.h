#pragma once

namespace DDraw {
	class IDDrawClipper;
	class IDDrawPalette;

	namespace SurfaceDescriptionFlag {
		constexpr uint32_t Caps = 0x00000001;
		constexpr uint32_t Height = 0x00000002;
		constexpr uint32_t Width = 0x00000004;
		constexpr uint32_t Pitch = 0x00000008;
		constexpr uint32_t SurfacePointer = 0x00000800;
		constexpr uint32_t PixelFormat = 0x00001000;
		constexpr uint32_t RefreshRate = 0x00040000;
	}

	namespace SurfaceCapsFlag {
		constexpr uint32_t Primary = 0x00000200;
	}

#pragma pack(push, 1)
	struct SurfaceCaps {
		uint32_t caps[3];
		union {
			uint32_t capsExt;
			uint32_t volumeDepth;
		};
	};

	struct ColorKey {
		uint32_t colorSpaceLow;
		uint32_t colorSpaceHigh;
	};

	struct PixelFormat {
		uint32_t size;
		uint32_t flags;
		uint32_t fourCC;
		union {
			uint32_t rgbBitCount;
			uint32_t yuvBitCount;
			uint32_t zBufferBitDepth;
			uint32_t alphaBitDepth;
			uint32_t luminanceBitCount;
			uint32_t bumpBitCount;
			uint32_t privateFormatBitCount;
		};
		union {
			uint32_t rBitMask;
			uint32_t yBitMask;
			uint32_t stencilBitDepth;
			uint32_t luminanceBitMask;
			uint32_t bumpDeltaUBitMask;
			uint32_t operations;
		};
		union {
			uint32_t gBitMask;
			uint32_t uBitMask;
			uint32_t zBitMask;
			uint32_t bumpDeltaVBitMask;
			struct {
				uint16_t flipMultisampleTypes;
				uint16_t bltMultisampleTypes;
			};
		};
		union {
			uint32_t bBitMask;
			uint32_t vBitMask;
			uint32_t stencilBitMask;
			uint32_t bumpLuminanceBitMask;
		};
		union {
			uint32_t rgbAlphaBitMask;
			uint32_t yuvAlphaBitMask;
			uint32_t luminanceAlphaBitMask;
			uint32_t rgbzBitMask;
			uint32_t yuvzBitMask;
		};
	};

	struct SurfaceDescription { // should be 124 size
		uint32_t size;
		uint32_t flags;
		uint32_t height;
		uint32_t width;
		union {
			int32_t pitch;
			uint32_t linearSize;
		};
		union {
			uint32_t backBufferCount;
			uint32_t depth;
		};
		union {
			uint32_t mipMapCount;
			uint32_t refreshRate;
			uint32_t srcVBHandle;
		};
		uint32_t alphaBitDepth;
		uint32_t reserved;
		void* surface;
		union {
			ColorKey colorKeyDestOverlay;
			uint32_t emptyFaceColor;
		};
		ColorKey colorKeyDestBlt;
		ColorKey colorKeySrcOverlay;
		ColorKey colorKeySrcBlt;
		union {
			PixelFormat  pixelFormat;
			uint32_t fvf;
		};
		SurfaceCaps caps;
		uint32_t textureStage;
	};
#pragma pack(pop)

	class IDDrawSurface4 {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// DirectDraw Surface
		virtual uint32_t __stdcall AddAttachedSurface(void*) = 0;
		virtual uint32_t __stdcall AddOverlayDirtyRect(void*) = 0;
		virtual uint32_t __stdcall Blt(RECT* destinationRect, IDDrawSurface4* sourceSurface, RECT* sourceRect, uint32_t flags, void* bltFx) = 0;
		virtual uint32_t __stdcall BltBatch(void*, uint32_t, uint32_t) = 0;
		virtual uint32_t __stdcall BltFast(uint32_t, uint32_t, void*, void*, uint32_t) = 0;
		virtual uint32_t __stdcall DeleteAttachedSurface(uint32_t, void*) = 0;
		virtual uint32_t __stdcall EnumAttachedSurfaces(void*, void*) = 0;
		virtual uint32_t __stdcall EnumOverlayZOrders(uint32_t, void*, void*) = 0;
		virtual uint32_t __stdcall Flip(void*, uint32_t) = 0;
		virtual uint32_t __stdcall GetAttachedSurface(void*, void*) = 0;
		virtual uint32_t __stdcall GetBltStatus(uint32_t) = 0;
		virtual uint32_t __stdcall GetCaps(SurfaceCaps* surfaceCaps) = 0;
		virtual uint32_t __stdcall GetClipper(void*) = 0;
		virtual uint32_t __stdcall GetColorKey(uint32_t, void*) = 0;
		virtual uint32_t __stdcall GetDeviceContext(HDC*) = 0;
		virtual uint32_t __stdcall GetFlipStatus(uint32_t) = 0;
		virtual uint32_t __stdcall GetOverlayPosition(void*, void*) = 0;
		virtual uint32_t __stdcall GetPallete(void*) = 0;
		virtual uint32_t __stdcall GetPixelFormat(PixelFormat* result) = 0;
		virtual uint32_t __stdcall GetSurfaceDesc(SurfaceDescription* desc) = 0;
		virtual uint32_t __stdcall Initialize(void*, void*) = 0;
		virtual uint32_t __stdcall IsLost() = 0;
		virtual uint32_t __stdcall Lock(RECT* rect, SurfaceDescription* desc, uint32_t flags , void* unused) = 0;
		virtual uint32_t __stdcall ReleaseDeviceContext(HDC dc) = 0;
		virtual uint32_t __stdcall Restore() = 0;
		virtual uint32_t __stdcall SetClipper(IDDrawClipper* clipper) = 0;
		virtual uint32_t __stdcall SetColorKey(uint32_t, void*) = 0;
		virtual uint32_t __stdcall SetOverlayPosition(uint32_t, uint32_t) = 0;
		virtual uint32_t __stdcall SetPalette(IDDrawPalette* palette) = 0;
		virtual uint32_t __stdcall Unlock(RECT* rect) = 0;
		virtual uint32_t __stdcall UpdateOverlay(void*, void*, void*, uint32_t, void*) = 0;
		virtual uint32_t __stdcall UpdateOverlayDisplay(uint32_t) = 0;
		virtual uint32_t __stdcall UpdateOverlayZOrder(uint32_t, void*) = 0;
		virtual uint32_t __stdcall GetDDInterface(void*) = 0;
		virtual uint32_t __stdcall PageLock(uint32_t) = 0;
		virtual uint32_t __stdcall PageUnlock(uint32_t) = 0;
		virtual uint32_t __stdcall SetSurfaceDesc(void*, uint32_t) = 0;
		virtual uint32_t __stdcall SetPrivateData(void*, void*, uint32_t, uint32_t) = 0;
		virtual uint32_t __stdcall GetPrivateData(void*, void*, void*) = 0;
		virtual uint32_t __stdcall FreePrivateData(void*) = 0;
		virtual uint32_t __stdcall GetUniquenessValue(void*) = 0;
		virtual uint32_t __stdcall ChangeUniquenessValue() = 0;
	};
}
