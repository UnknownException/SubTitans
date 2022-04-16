#include "subtitans.h"
#include "keyboarddevice.h"

KeyboardDevice::KeyboardDevice()
{
	ReferenceCount = 0;
	Initialized = false;
}

KeyboardDevice::~KeyboardDevice()
{
}

uint32_t __stdcall KeyboardDevice::QueryInterface(GUID* guid, void** result)
{
	unsigned long guid4[] = { 0, 0 };
	memcpy(guid4, guid->Data4, sizeof(guid4));
	GetLogger()->Error("%s %s %08X-%04X-%04X-%08X%08X\n", __FUNCTION__, "unknown interface", guid->Data1, guid->Data2, guid->Data3, guid4[0], guid4[1]);
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall KeyboardDevice::AddRef()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	ReferenceCount++;

	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::Release()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	if (--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::GetCapabilities(DInput::Caps* caps) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::EnumObjects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::GetProperty(GUID& guid, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::SetProperty(GUID& guid, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall KeyboardDevice::Acquire() 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 
	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::Unacquire() 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 
	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::GetDeviceState(uint32_t bufferSize, void* buffer)
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (bufferSize != sizeof(Global::_KeyboardInformation))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "unexpected size", bufferSize, sizeof(Global::_KeyboardInformation));
		return ResultCode::InvalidArgument;
	}

	if (Initialized)
		memcpy(buffer, &Global::KeyboardInformation, sizeof(Global::_KeyboardInformation));
	else
		memset(buffer, 0, sizeof(Global::_KeyboardInformation));

	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::GetDeviceData(uint32_t, void*, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall KeyboardDevice::SetDataFormat(DInput::DataFormat* dataFormat)
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__);
	Initialized = false;

	if (dataFormat->size != sizeof(DInput::DataFormat))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "data format has an unexpected size", dataFormat->size, sizeof(DInput::DataFormat));
		return ResultCode::InvalidArgument;
	}

	if (dataFormat->objectSize != sizeof(DInput::ObjectDataFormat))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "object data format has an unexpected size", dataFormat->objectSize, sizeof(DInput::ObjectDataFormat));
		return ResultCode::InvalidArgument;
	}

	if (dataFormat->contentSize != sizeof(Global::_KeyboardInformation))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "unexpected data buffer size", dataFormat->contentSize, sizeof(Global::_KeyboardInformation));
		return ResultCode::InvalidArgument;
	}

	Initialized = true;

	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::SetEventNotification(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall KeyboardDevice::SetCooperativeLevel(void*, uint32_t) 
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__);
	return ResultCode::Ok;
}

uint32_t __stdcall KeyboardDevice::GetObjectInfo(void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::GetDeviceInfo(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::RunControlPanel(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::Initialize(void*, int32_t, GUID&) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::CreateEffect(GUID&, void*, void**, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::EnumEffects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::GetEffectInfo(void*, GUID&) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::GetForceFeedbackState(uint32_t*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::SendForceFeedbackCommand(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::EnumCreatedEffectObjects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::Escape(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::Poll() { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::SendDeviceData(void*, void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::EnumEffectsInFile(void*, void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall KeyboardDevice::WriteEffectsToFile(void*, uint32_t, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
