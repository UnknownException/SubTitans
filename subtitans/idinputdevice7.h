#pragma once

namespace DInput {
	enum KeyboardKey {
		KEY_ESCAPE = 1,		// Esc
		KEY_1,				// 1
		KEY_2,				// 2
		KEY_3,				// 3
		KEY_4,				// 4
		KEY_5,				// 5
		KEY_6,				// 6
		KEY_7,				// 7
		KEY_8,				// 8
		KEY_9,				// 9
		KEY_0,				// 0
		KEY_DASH,			// -
		KEY_EQUAL,			// =
		KEY_BACKSPACE,		// <-
		KEY_TAB,			// |<- ->|
		KEY_Q,				// Q
		KEY_W,				// W
		KEY_E,				// E
		KEY_R,				// R
		KEY_T,				// T
		KEY_Y,				// Y
		KEY_U,				// U
		KEY_I,				// I
		KEY_O,				// O
		KEY_P,				// P
		KEY_LEFTBRACKET,	// [
		KEY_RIGHTBRACKET,	// ]
		KEY_ENTER,			// Enter
		KEY_LEFTCTRL,		// Left Ctrl (Control)
		KEY_A,				// A
		KEY_S,				// S
		KEY_D,				// D
		KEY_F,				// F
		KEY_G,				// G
		KEY_H,				// H
		KEY_J,				// J
		KEY_K,				// K
		KEY_L,				// L
		KEY_SEMICOLON,		// ;
		KEY_APOSTROPHE,		// '
		KEY_TILDE,			// ~
		KEY_LEFTSHIFT,		// Left Shift
		KEY_BACKSLASH,		/* \ */
		KEY_Z,				// Z
		KEY_X,				// X
		KEY_C,				// C
		KEY_V,				// V
		KEY_B,				// B
		KEY_N,				// N
		KEY_M,				// M
		KEY_COMMA,			// ,
		KEY_PERIOD,			// .
		KEY_FORWARDSLASH,	// /
		KEY_RIGHTSHIFT,		// Right Shift
		KEY_MULTIPLTY,		// *
		KEY_LEFTALT,		// Left Alt
		KEY_SPACE,			// Space
		KEY_CAPSLOCK,		// Caps Lock
		KEY_F1,				// F1
		KEY_F2,				// F2
		KEY_F3,				// F3
		KEY_F4,				// F4
		KEY_F5,				// F5
		KEY_F6,				// F6
		KEY_F7,				// F7
		KEY_F8,				// F8
		KEY_F9,				// F9
		KEY_F10,			// F10
		KEY_NUMLOCK,		// Num Lock
		KEY_SCROLLLOCK,		// Scroll Lock
		KEY_NUMPAD7,		// Numpad 7
		KEY_NUMPAD8,		// Numpad 8
		KEY_NUMPAD9,		// Numpad 9
		KEY_SUBTRACT,		// Numpad - 
		KEY_NUMPAD4,		// Numpad 4
		KEY_NUMPAD5,		// Numpad 5
		KEY_NUMPAD6,		// Numpad 6
		KEY_ADD,			// Numpad +
		KEY_NUMPAD1,		// Numpad 1
		KEY_NUMPAD2,		// Numpad 2
		KEY_NUMPAD3,		// Numpad 3
		KEY_NUMPAD0,		// Numpad 0
		KEY_DECIMAL,		// Numpad Del

		KEY_F11 = 87,		// F11
		KEY_F12 = 88,		// F12

		KEY_RIGHTCTRL = 157, // Right CTRL

		KEY_DIVIDE = 181,	// Numpad /

//		KEY_PRINTSCRN = 183,// Printscreen
		KEY_RIGHTALT = 184,	// Right alt

//		KEY_PAUSE = 197, // Pause/Break

		KEY_HOME = 199,		// Home
		KEY_UP = 200,		// Up
		KEY_PAGEUP = 201,	// Page Up

		KEY_LEFT = 203,		// Left

		KEY_RIGHT = 205,	// Right

		KEY_END = 207,		// End
		KEY_DOWN = 208,		// Down
		KEY_PAGEDOWN = 209, // Page Down
		KEY_INSERT = 210,	// Insert
		KEY_DELETE = 211,	// Delete
	};

#pragma pack(push, 1)
	struct ObjectDataFormat {
		GUID* guid;
		uint32_t offset;
		uint32_t type;
		uint32_t flags;
	};

	struct DataFormat {
		uint32_t size; // sizeof(DataFormat)
		uint32_t objectSize; // sizeof(ObjectDataFormat);
		uint32_t flags;
		uint32_t contentSize; // return object
		uint32_t objectCount;
		ObjectDataFormat* objects;
	};

	struct Caps {
		uint32_t size;
		uint32_t flags;
		uint32_t devType;
		uint32_t numberOfAxes;
		uint32_t numberOfButtons;
		uint32_t pointOfViewControllers;
		uint32_t ffSamplePeriod;
		uint32_t ffMinimumTimeResolution;
		uint32_t firmwareRevision;
		uint32_t hardwareRevision;
		uint32_t ffDriverRevision;
	};
#pragma pack(pop)

	class IDInputDevice7 {
	public:
		// IUnknown
		virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) = 0;
		virtual uint32_t __stdcall AddRef() = 0;
		virtual uint32_t __stdcall Release() = 0;

		// DirectInput7
		virtual uint32_t __stdcall GetCapabilities(Caps* caps) = 0;
		virtual uint32_t __stdcall EnumObjects(void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall GetProperty(GUID& guid, void*) = 0;
		virtual uint32_t __stdcall SetProperty(GUID& guid, void*) = 0;
		virtual uint32_t __stdcall Acquire() = 0;
		virtual uint32_t __stdcall Unacquire() = 0;
		virtual uint32_t __stdcall GetDeviceState(uint32_t, void*) = 0;
		virtual uint32_t __stdcall GetDeviceData(uint32_t, void*, void*, uint32_t) = 0;
		virtual uint32_t __stdcall SetDataFormat(DataFormat*) = 0;
		virtual uint32_t __stdcall SetEventNotification(void*) = 0;
		virtual uint32_t __stdcall SetCooperativeLevel(void*, uint32_t) = 0;
		virtual uint32_t __stdcall GetObjectInfo(void*, uint32_t, uint32_t) = 0;
		virtual uint32_t __stdcall GetDeviceInfo(void*) = 0;
		virtual uint32_t __stdcall RunControlPanel(void*, uint32_t) = 0;
		virtual uint32_t __stdcall Initialize(void*, int32_t, GUID&) = 0;
		virtual uint32_t __stdcall CreateEffect(GUID&, void*, void**, void*) = 0;
		virtual uint32_t __stdcall EnumEffects(void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall GetEffectInfo(void*, GUID&) = 0;
		virtual uint32_t __stdcall GetForceFeedbackState(uint32_t*) = 0;
		virtual uint32_t __stdcall SendForceFeedbackCommand(uint32_t) = 0;
		virtual uint32_t __stdcall EnumCreatedEffectObjects(void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall Escape(void*) = 0;
		virtual uint32_t __stdcall Poll() = 0;
		virtual uint32_t __stdcall SendDeviceData(void*, void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall EnumEffectsInFile(void*, void* callback, void*, uint32_t) = 0;
		virtual uint32_t __stdcall WriteEffectsToFile(void*, uint32_t, void*, uint32_t) = 0;
	};
}