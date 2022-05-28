#include "subtitans.h"
#include "device.h"
#include "openglrenderer.h"
#include "softwarerenderer.h"
#include "input.h"
#include "ddrawreplacementpatch.h"

namespace DDrawCreateDetour {
	// Detour variables
	constexpr unsigned long DetourSize = 5;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	__declspec(naked) void Implementation()
	{
		__asm call [DirectDrawCreate];
		__asm jmp [JmpBackAddress];
	}
}

namespace DInputCreateDetour {
	// Detour variables
	constexpr unsigned long DetourSize = 5;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	__declspec(naked) void Implementation()
	{
		__asm call [DirectInputCreate];
		__asm jmp [JmpBackAddress];
	}
}

namespace WindowRegisterClassDetour {
	// Detour variables
	constexpr unsigned long DetourSize = 6;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	// Function variables
	static WNDPROC WndProc = 0;

	static uint8_t KeyTranslationTable[256] = { 0, };

	LRESULT CALLBACK WndProcDetour(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
	{
		switch (message)
		{
			case WM_WINDOWPOSCHANGING:
			{
				GetLogger()->Debug("Handling WM_WINDOWPOSCHANGING\n");
				WINDOWPOS* windowPos = (WINDOWPOS*)lParam;
				if (windowPos->cx != Global::MonitorWidth || windowPos->cy != Global::MonitorHeight)
				{
					windowPos->cx = Global::MonitorWidth;
					windowPos->cy = Global::MonitorHeight;
					
					if (Global::GameWindow)
					{
						HDC deviceContext = GetDC(Global::GameWindow);

						RECT rect;
						rect.left = 0;
						rect.top = 0;
						rect.right = Global::MonitorWidth;
						rect.bottom = Global::MonitorHeight;

						HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
						FillRect(deviceContext, &rect, brush);
						DeleteObject(brush);

						ReleaseDC(Global::GameWindow, deviceContext);
					}
				}

				RECT rect;
				rect.left = 0;
				rect.top = 0;
				rect.right = Global::InternalWidth;
				rect.bottom = Global::InternalHeight;
				ClipCursor(&rect);
			} break;
			case WM_MOUSEMOVE:
				Global::MouseInformation.x = (int16_t)(lParam & 0xFFFF);
				Global::MouseInformation.y = (int16_t)(lParam >> 16);			

				// (BUG) Workaround for cursor issue when moving to 0, 0. 
				// Requires further investigation.
				if (Global::MouseInformation.x == 0 && Global::MouseInformation.y == 0)
					Global::MouseInformation.y = 1;

				break;
			case WM_MOUSEWHEEL:
				Global::MouseInformation.z += (int16_t)(wParam >> 16);
				break;
			case WM_LBUTTONUP:
				Global::MouseInformation.lPressed = Global::KeyReleasedFlag;
				break;
			case WM_LBUTTONDOWN:
				Global::MouseInformation.lPressed = Global::KeyPressedFlag;
				break;
			case WM_RBUTTONUP:
				Global::MouseInformation.rPressed = Global::KeyReleasedFlag;
				break;
			case WM_RBUTTONDOWN:
				Global::MouseInformation.rPressed = Global::KeyPressedFlag;
				break;
			case WM_MBUTTONUP:
				Global::MouseInformation.mPressed = Global::KeyReleasedFlag;
				break;
			case WM_MBUTTONDOWN:
				Global::MouseInformation.mPressed = Global::KeyPressedFlag;
				break;
			case WM_XBUTTONUP:
				// Filter on key?
				Global::MouseInformation.xPressed = Global::KeyReleasedFlag;
				break;
			case WM_XBUTTONDOWN:
				Global::MouseInformation.xPressed = Global::KeyPressedFlag;
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP: // ALT
				switch (wParam)
				{
					case VK_MENU:
						if (!(GetAsyncKeyState(VK_LMENU) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTALT] = Global::KeyReleasedFlag;
						if (!(GetAsyncKeyState(VK_RMENU) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTALT] = Global::KeyReleasedFlag;
						break;
					case VK_CONTROL:
						if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTCTRL] = Global::KeyReleasedFlag;
						if (!(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTCTRL] = Global::KeyReleasedFlag;
						break;
					case VK_SHIFT:
						if (!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTSHIFT] = Global::KeyReleasedFlag;
						if (!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTSHIFT] = Global::KeyReleasedFlag;
						break;
					default:
						Global::KeyboardInformation.keyPressed[KeyTranslationTable[wParam & 0xFF]] = Global::KeyReleasedFlag;
						break;
				} break;						
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN: // ALT
				switch (wParam)
				{
					case VK_MENU:
						if (GetAsyncKeyState(VK_LMENU) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTALT] = Global::KeyPressedFlag;
						if (GetAsyncKeyState(VK_RMENU) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTALT] = Global::KeyPressedFlag;
						if (Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTSHIFT]) Global::ImGuiEnabled = !Global::ImGuiEnabled;
						break;
					case VK_CONTROL:
						if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTCTRL] = Global::KeyPressedFlag;
						if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTCTRL] = Global::KeyPressedFlag;
						break;
					case VK_SHIFT:
						if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_LEFTSHIFT] = Global::KeyPressedFlag;
						if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) Global::KeyboardInformation.keyPressed[DInput::KEY_RIGHTSHIFT] = Global::KeyPressedFlag;
						break;
					default:
						Global::KeyboardInformation.keyPressed[KeyTranslationTable[wParam & 0xFF]] = Global::KeyPressedFlag;
						break;
				} break;
			case WM_ACTIVATEAPP:
			{
				memset(&Global::KeyboardInformation, 0, sizeof(Global::_KeyboardInformation));
				
				if (wParam == 1)
				{
					RECT rect;
					rect.left = 0;
					rect.top = 0;
					rect.right = Global::InternalWidth;
					rect.bottom = Global::InternalHeight;
					ClipCursor(&rect);
				}
				else
				{
					ClipCursor(nullptr); // Free the cursor
				}
			} break;
			default:
				break;
		}

		return WndProc(hWnd, message, wParam, lParam);
	}

	ATOM __stdcall RegisterClassDetour(WNDCLASSA* wndClass)
	{
		WndProc = wndClass->lpfnWndProc;
		wndClass->lpfnWndProc = WndProcDetour;
		return RegisterClassA(wndClass);
	}

	__declspec(naked) void Implementation()
	{
		__asm call [RegisterClassDetour];
		__asm jmp [JmpBackAddress];
	}
}

namespace WindowCreateDetour {
	// Detour variables
	constexpr unsigned long DetourSize = 6;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	// Function variables
	static bool ForceSoftwareRendering = false;

	void __stdcall CreateRenderer()
	{
		IRenderer* renderer = nullptr;
		if (!ForceSoftwareRendering)
		{
			GetLogger()->Informational("Using the OpenGL renderer\n");
			renderer = new OpenGLRenderer();
			if (!renderer->Create())
			{
				GetLogger()->Error("Failed to create the OpenGL renderer\n");
				GetLogger()->Informational("Falling back to software rendering\n");

				delete renderer;
				renderer = nullptr;
			}
		}

		if (!renderer)
		{
			GetLogger()->Informational("Using the software renderer\n");
			renderer = new SoftwareRenderer();
			renderer->Create();
		}

		Global::Backend = renderer;
	}

	__declspec(naked) void Implementation()
	{
		__asm call [CreateWindowExA];
		__asm mov [Global::GameWindow], eax;

		__asm push eax; // Preserve
		__asm call [CreateRenderer];
		__asm pop eax; // Restore

		__asm jmp [JmpBackAddress];
	}
}

namespace DInputAbsolutePositioning {
	constexpr unsigned long DetourSize = 7;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;

	__declspec(naked) void Implementation()
	{
		__asm mov edi, [Global::MouseInformation.x];
		__asm mov edx, [Global::MouseInformation.y];
		__asm jmp [JmpBackAddress];
	}
}

DDrawReplacementPatch::DDrawReplacementPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	DDrawDetourAddress = 0;
	DInputDetourAddress = 0;
	WindowRegisterClassDetourAddress = 0;
	WindowCreateDetourAddress = 0;
	DInputAbsolutePositioningDetourAddress = 0;
	ForceSoftwareRendering = false;
	DInputReplacement = false;
}

DDrawReplacementPatch::~DDrawReplacementPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);

	if (Global::Backend)
		delete Global::Backend;
}

bool DDrawReplacementPatch::Validate()
{
	return DDrawDetourAddress && DInputDetourAddress 
		&& WindowRegisterClassDetourAddress && WindowCreateDetourAddress
		&& DInputAbsolutePositioningDetourAddress;
}

void BuildKeyLookupTable()
{
	auto t = &WindowRegisterClassDetour::KeyTranslationTable;

	(*t)[VK_UP]			= DInput::KEY_UP;			// Scrolling
	(*t)[VK_DOWN]		= DInput::KEY_DOWN;			// Scrolling
	(*t)[VK_LEFT]		= DInput::KEY_LEFT;			// Scrolling
	(*t)[VK_RIGHT]		= DInput::KEY_RIGHT;		// Scrolling

	(*t)[VK_ESCAPE]		= DInput::KEY_ESCAPE;		// Game menu
	(*t)[VK_RETURN]		= DInput::KEY_ENTER;		// Chatbox
	(*t)[VK_BACK]		= DInput::KEY_BACKSPACE;	// Chatbox related
	
	(*t)[VK_ADD]		= DInput::KEY_ADD;			// Zoom in
	(*t)[VK_SUBTRACT]	= DInput::KEY_SUBTRACT;		// Zout out
	(*t)[VK_MULTIPLY]	= DInput::KEY_MULTIPLTY;	// Rotate camera CCW
	(*t)[VK_DIVIDE]		= DInput::KEY_DIVIDE;		// Rotate camera CW

	// 0-9 (Unit/building assignment)
	(*t)[0x30]			= DInput::KEY_0;
	(*t)[0x31]			= DInput::KEY_1;
	(*t)[0x32]			= DInput::KEY_2;
	(*t)[0x33]			= DInput::KEY_3;
	(*t)[0x34]			= DInput::KEY_4;
	(*t)[0x35]			= DInput::KEY_5;
	(*t)[0x36]			= DInput::KEY_6;
	(*t)[0x37]			= DInput::KEY_7;
	(*t)[0x38]			= DInput::KEY_8;
	(*t)[0x39]			= DInput::KEY_9;

	// A-Z (GUI shortcuts)
	(*t)[0x41]			= DInput::KEY_A;
	(*t)[0x42]			= DInput::KEY_B;
	(*t)[0x43]			= DInput::KEY_C;
	(*t)[0x44]			= DInput::KEY_D;
	(*t)[0x45]			= DInput::KEY_E;
	(*t)[0x46]			= DInput::KEY_F;
	(*t)[0x47]			= DInput::KEY_G;
	(*t)[0x48]			= DInput::KEY_H;
	(*t)[0x49]			= DInput::KEY_I;
	(*t)[0x4A]			= DInput::KEY_J;
	(*t)[0x4B]			= DInput::KEY_K;
	(*t)[0x4C]			= DInput::KEY_L;
	(*t)[0x4D]			= DInput::KEY_M;
	(*t)[0x4E]			= DInput::KEY_N;
	(*t)[0x4F]			= DInput::KEY_O;
	(*t)[0x50]			= DInput::KEY_P;
	(*t)[0x51]			= DInput::KEY_Q;
	(*t)[0x52]			= DInput::KEY_R;
	(*t)[0x53]			= DInput::KEY_S;
	(*t)[0x54]			= DInput::KEY_T;
	(*t)[0x55]			= DInput::KEY_U;
	(*t)[0x56]			= DInput::KEY_V;
	(*t)[0x57]			= DInput::KEY_W;
	(*t)[0x58]			= DInput::KEY_X;
	(*t)[0x59]			= DInput::KEY_Y;
	(*t)[0x5A]			= DInput::KEY_Z;

	// F1-F12 (Set view area)
	(*t)[VK_F1]			= DInput::KEY_F1;
	(*t)[VK_F2]			= DInput::KEY_F2;
	(*t)[VK_F3]			= DInput::KEY_F3;
	(*t)[VK_F4]			= DInput::KEY_F4;
	(*t)[VK_F5]			= DInput::KEY_F5;
	(*t)[VK_F6]			= DInput::KEY_F6;
	(*t)[VK_F7]			= DInput::KEY_F7;
	(*t)[VK_F8]			= DInput::KEY_F8;
	(*t)[VK_F9]			= DInput::KEY_F9;
	(*t)[VK_F10]		= DInput::KEY_F10;
	(*t)[VK_F11]		= DInput::KEY_F11;
	(*t)[VK_F12]		= DInput::KEY_F12;

	// Numpad 0-9 (1-5 used for object depth level) (BUG: Requires Num Lock!)
	(*t)[VK_NUMPAD0]	= DInput::KEY_NUMPAD0; //  (Original function increase depth isn't working)
	(*t)[VK_NUMPAD1]	= DInput::KEY_NUMPAD1;
	(*t)[VK_NUMPAD2]	= DInput::KEY_NUMPAD2;
	(*t)[VK_NUMPAD3]	= DInput::KEY_NUMPAD3;
	(*t)[VK_NUMPAD4]	= DInput::KEY_NUMPAD4;
	(*t)[VK_NUMPAD5]	= DInput::KEY_NUMPAD5;
	(*t)[VK_NUMPAD6]	= DInput::KEY_NUMPAD6;
	(*t)[VK_NUMPAD7]	= DInput::KEY_NUMPAD7;
	(*t)[VK_NUMPAD8]	= DInput::KEY_NUMPAD8;
	(*t)[VK_NUMPAD9]	= DInput::KEY_NUMPAD9;

	// Rebindable
	(*t)[VK_TAB]		= DInput::KEY_TAB; // Untested
	(*t)[VK_OEM_MINUS]	= DInput::KEY_DASH; // Untested
	(*t)[VK_OEM_PLUS]	= DInput::KEY_EQUAL; // Untested
	(*t)[VK_OEM_PERIOD] = DInput::KEY_PERIOD; // Untested
	(*t)[VK_OEM_COMMA]	= DInput::KEY_COMMA; // Untested
	(*t)[VK_OEM_1]		= DInput::KEY_SEMICOLON; // Untested
	(*t)[VK_OEM_2]		= DInput::KEY_FORWARDSLASH; // Untested
	(*t)[VK_OEM_3]		= DInput::KEY_TILDE; // Untested
	(*t)[VK_OEM_4]		= DInput::KEY_LEFTBRACKET; // Untested
	(*t)[VK_OEM_5]		= DInput::KEY_BACKSLASH; // Untested
	(*t)[VK_OEM_6]		= DInput::KEY_RIGHTBRACKET; // Untested
	(*t)[VK_OEM_7]		= DInput::KEY_APOSTROPHE; // Untested
	(*t)[VK_DECIMAL]	= DInput::KEY_DECIMAL;	// (Original function decrease depth isn't working)
	(*t)[VK_SPACE]		= DInput::KEY_SPACE;
	(*t)[VK_CAPITAL]	= DInput::KEY_CAPSLOCK; // Untested
	(*t)[VK_NUMLOCK]	= DInput::KEY_NUMLOCK; // Untested
	(*t)[VK_SCROLL]		= DInput::KEY_SCROLLLOCK; // Untested
	(*t)[VK_INSERT]		= DInput::KEY_INSERT; // Untested
	(*t)[VK_DELETE]		= DInput::KEY_DELETE; // Untested
	(*t)[VK_HOME]		= DInput::KEY_HOME; // Untested
	(*t)[VK_END]		= DInput::KEY_END; // Untested
	(*t)[VK_NEXT]		= DInput::KEY_PAGEDOWN; // Untested
	(*t)[VK_PRIOR]		= DInput::KEY_PAGEUP; // Untested
}

bool DDrawReplacementPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	DDrawCreateDetour::JmpFromAddress = DDrawDetourAddress;
	DDrawCreateDetour::JmpBackAddress = DDrawCreateDetour::JmpFromAddress + DDrawCreateDetour::DetourSize;
	if (!Detour::Create(DDrawCreateDetour::JmpFromAddress, DDrawCreateDetour::DetourSize, (unsigned long)DDrawCreateDetour::Implementation))
		return false;

	WindowRegisterClassDetour::JmpFromAddress = WindowRegisterClassDetourAddress;
	WindowRegisterClassDetour::JmpBackAddress = WindowRegisterClassDetour::JmpFromAddress + WindowRegisterClassDetour::DetourSize;
	if (!Detour::Create(WindowRegisterClassDetour::JmpFromAddress, WindowRegisterClassDetour::DetourSize, (unsigned long)WindowRegisterClassDetour::Implementation))
		return false;

	WindowCreateDetour::JmpFromAddress = WindowCreateDetourAddress;
	WindowCreateDetour::JmpBackAddress = WindowCreateDetour::JmpFromAddress + WindowCreateDetour::DetourSize;
	WindowCreateDetour::ForceSoftwareRendering = ForceSoftwareRendering;
	if (!Detour::Create(WindowCreateDetour::JmpFromAddress, WindowCreateDetour::DetourSize, (unsigned long)WindowCreateDetour::Implementation))
		return false;

	if (DInputReplacement)
	{
		DInputCreateDetour::JmpFromAddress = DInputDetourAddress;
		DInputCreateDetour::JmpBackAddress = DInputCreateDetour::JmpFromAddress + DInputCreateDetour::DetourSize;
		if (!Detour::Create(DInputCreateDetour::JmpFromAddress, DInputCreateDetour::DetourSize, (unsigned long)DInputCreateDetour::Implementation))
			return false;

		DInputAbsolutePositioning::JmpFromAddress = DInputAbsolutePositioningDetourAddress;
		DInputAbsolutePositioning::JmpBackAddress = DInputAbsolutePositioning::JmpFromAddress + DInputAbsolutePositioning::DetourSize;
		if (!Detour::Create(DInputAbsolutePositioning::JmpFromAddress, DInputAbsolutePositioning::DetourSize, (unsigned long)DInputAbsolutePositioning::Implementation))
			return false;

		BuildKeyLookupTable();
	}

	SetProcessDPIAware();
	// Reset DPI unaware values
	Global::MonitorWidth = GetSystemMetrics(SM_CXSCREEN);
	Global::MonitorHeight = GetSystemMetrics(SM_CYSCREEN);

	return true;
}