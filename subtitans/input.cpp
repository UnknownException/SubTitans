#include "subtitans.h"
#include "mousedevice.h"
#include "keyboarddevice.h"
#include "input.h"

Input::Input()
{
	ReferenceCount = 0;
}

Input::~Input()
{
}

uint32_t __stdcall Input::QueryInterface(GUID* guid, void** result)
{
	unsigned long guid4[] = { 0, 0 };
	memcpy(guid4, guid->Data4, sizeof(guid4));
	GetLogger()->Error("Unknown interface %08X-%04X-%04X-%08X%08X\n", guid->Data1, guid->Data2, guid->Data3, guid4[0], guid4[1]);
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall Input::AddRef()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	ReferenceCount++;

	return ResultCode::Ok;
}

uint32_t __stdcall Input::Release()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	if (--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

uint32_t __stdcall Input::CreateDevice(GUID* guid, DInput::IDInputDevice7** result, void*)
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	GUID mouseGuid;	
	mouseGuid.Data1 = 0x6F1D2B60;
	mouseGuid.Data2 = 0xD5A0;
	mouseGuid.Data3 = 0x11CF;
	uint32_t value = 0x4544C7BF;
	memcpy(mouseGuid.Data4, &value, sizeof(uint32_t));
	value = 0x00005453;
	memcpy(&mouseGuid.Data4[4], &value, sizeof(uint32_t));

	GUID keyboardGuid;
	memcpy(&keyboardGuid, &mouseGuid, sizeof(GUID));
	keyboardGuid.Data1 = 0x6F1D2B61;

	if (memcmp(guid, &mouseGuid, sizeof(GUID)) == 0)
	{
		GetLogger()->Informational("Mouse requested\n");
		*result = new MouseDevice();
		(*result)->AddRef();
	}
	else if (memcmp(guid, &keyboardGuid, sizeof(GUID)) == 0)
	{
		GetLogger()->Informational("Keyboard requested\n");
		*result = new KeyboardDevice();
		(*result)->AddRef();
	}
	else
	{
		GetLogger()->Error("Unsupported device (%08X)\n", guid->Data1);
		*result = nullptr;
		return ResultCode::InvalidArgument;
	}
	
	return ResultCode::Ok;
}

uint32_t __stdcall Input::EnumDevices(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Input::GetDeviceStatus(GUID* guid) {	GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Input::RunControlPanel(HWND, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Input::Initialize(HINSTANCE hInstance, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Input::FindDevice(GUID* guid, void* str, void* guid2) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall Input::CreateDeviceEx(GUID*, GUID, void*, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall DirectInputCreate(HINSTANCE hInstance, uint32_t version, DInput::IDInput7** result, void*)
{
	GetLogger()->Debug("DInput version %08X\n", version);
	
	*result = new Input();
	(*result)->AddRef();

	return ResultCode::Ok;
}
