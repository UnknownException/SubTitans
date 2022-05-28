#pragma once

#include <Windows.h>
#include <VersionHelpers.h>

#include <cmath>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

#pragma comment(lib, "winmm.lib")

#include "../shared/gameversion.h"
#include "../shared/detour.h"
#include "../shared/memorywriter.h"
#include "../shared/logger.h"
#include "../shared/configuration.h"
#include "../shared/file.h"

#ifdef _DEBUG
	#pragma comment(lib, "../Debug/shared.lib")
#else
	#pragma comment(lib, "../Release/shared.lib")
#endif

Logger* GetLogger(); // Singleton
Configuration* GetConfiguration(); // Singleton

namespace ResultCode {
	constexpr uint32_t Ok				= 0x00000000;
	constexpr uint32_t InvalidArgument	= 0x80070057; // DDraw invalid parameter, DInput invalid parameter
	constexpr uint32_t Unimplemented	= 0x80004001; // DDraw unsupported
	constexpr uint32_t NoInterface		= 0x80004002;

	// DDraw
	constexpr uint32_t NoClipList		= 0x887600CD;
	constexpr uint32_t InvalidObject	= 0x88760082;

	// DInput
	constexpr uint32_t NotInitialized	= 0x80070015;
}

class IRenderer;
namespace Global {
	enum RenderingBackend {
		Automatic = 0, // Same as using OpenGL
		DirectDraw,
		OpenGL,
		Software
	};

	extern int32_t InternalWidth; // Variable
	extern int32_t InternalHeight; // Variable
	extern int32_t MonitorWidth; // Constant
	extern int32_t MonitorHeight; // Constant
	extern int32_t RenderWidth; // Constant
	extern int32_t RenderHeight; // Constant

	extern int32_t BitsPerPixel; // Variable

	extern bool VideoWorkaround; // Variable

	extern HWND GameWindow; // Don't use this with GetDC/ReleaseDC on different threads

	extern HANDLE RenderEvent;
	extern HANDLE VerticalBlankEvent;

	extern IRenderer* Backend; // Create/Delete called from ddrawreplacement

	extern bool RetroShader; // Constant
	extern std::atomic<bool> ImGuiEnabled;

	__forceinline float GetScaleFactor() { return (float)MonitorHeight / (float)InternalHeight; }
	__forceinline int32_t GetAspectRatioCompensatedWidth() { return (int32_t)(InternalWidth * GetScaleFactor()); }
	__forceinline int32_t GetPadding() { return (int32_t)((MonitorWidth - GetAspectRatioCompensatedWidth()) / 2.f); }

	struct RenderInformation {
		int32_t InternalWidth;
		int32_t InternalHeight;
		int32_t MonitorWidth;
		int32_t MonitorHeight;
		int32_t RenderWidth;
		int32_t RenderHeight;
		int32_t BitsPerPixel;
		int32_t AspectRatioCompensatedWidth;
		int32_t Padding;
	};

	__forceinline RenderInformation GetRenderInformation() {
		RenderInformation ri;
		memset(&ri, 0, sizeof(RenderInformation));

		ri.InternalWidth = InternalWidth;
		ri.InternalHeight = InternalHeight;
		ri.MonitorWidth = MonitorWidth;
		ri.MonitorHeight = MonitorHeight;
		ri.RenderWidth = RenderWidth;
		ri.RenderHeight = RenderHeight;
		ri.BitsPerPixel = BitsPerPixel;
		ri.AspectRatioCompensatedWidth = GetAspectRatioCompensatedWidth();
		ri.Padding = GetPadding();

		return ri;
	}

#pragma pack(push, 1)
	struct _MouseInformation {
		int32_t x;
		int32_t y;
		int32_t z;
		uint8_t lPressed;
		uint8_t rPressed;
		uint8_t mPressed;
		uint8_t xPressed;
	};

	struct _KeyboardInformation {
		uint8_t keyPressed[256];
	};
#pragma pack(pop)

	extern _MouseInformation MouseInformation;
	extern _KeyboardInformation KeyboardInformation;

	constexpr uint8_t KeyPressedFlag = 0x80;
	constexpr uint8_t KeyReleasedFlag = 0x00;
}