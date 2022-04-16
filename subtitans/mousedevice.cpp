#include "subtitans.h"
#include "mousedevice.h"

MouseDevice::MouseDevice()
{
	ReferenceCount = 0;
	Initialized = true;
}

MouseDevice::~MouseDevice()
{
}

uint32_t __stdcall MouseDevice::QueryInterface(GUID* guid, void** result)
{
	unsigned long guid4[] = { 0, 0 };
	memcpy(guid4, guid->Data4, sizeof(guid4));
	GetLogger()->Error("%s %s %08X-%04X-%04X-%08X%08X\n", __FUNCTION__, "unknown interface", guid->Data1, guid->Data2, guid->Data3, guid4[0], guid4[1]);
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall MouseDevice::AddRef()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	ReferenceCount++;

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::Release()
{
	GetLogger()->Trace("%s\n", __FUNCTION__);

	if (--ReferenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::GetCapabilities(DInput::Caps* caps)
{
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (caps->size != sizeof(DInput::Caps))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "caps structure has an unexpected size", caps->size, sizeof(DInput::Caps));
		return ResultCode::InvalidArgument;
	}

	caps->flags = 1; // Attached
	caps->devType = 2; // Mouse
	caps->numberOfAxes = 3; // X, Y and Z
	caps->numberOfButtons = 4; // L, R, M and X

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::EnumObjects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::GetProperty(GUID& guid, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::SetProperty(GUID& guid, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall MouseDevice::Acquire()
{
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (!Initialized)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "data format has to be set");
		return ResultCode::NotInitialized;
	}

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::Unacquire()
{
	GetLogger()->Trace("%s\n", __FUNCTION__); 
	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::GetDeviceState(uint32_t bufferSize, void* buffer)
{ 
	GetLogger()->Trace("%s\n", __FUNCTION__); 

	if (!Initialized)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "data format has to be set");
		return ResultCode::NotInitialized;
	}

	if (bufferSize != sizeof(Global::_MouseInformation))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "received an unexpected buffer size", bufferSize, sizeof(Global::MouseInformation));
		return ResultCode::InvalidArgument;
	}

	memcpy(buffer, &Global::MouseInformation, sizeof(Global::_MouseInformation));
	// Don't care about relative X & Y positioning.
	// X & Y values are modified by a detour later on.

	Global::MouseInformation.z = 0;

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::GetDeviceData(uint32_t, void*, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall MouseDevice::SetDataFormat(DInput::DataFormat* dataFormat)
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

	if (dataFormat->contentSize != sizeof(Global::_MouseInformation))
	{
		GetLogger()->Error("%s %s %i != %i\n", __FUNCTION__, "unexpected data buffer size", dataFormat->contentSize, sizeof(Global::_MouseInformation));
		return ResultCode::InvalidArgument;
	}

	Initialized = true;

	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::SetEventNotification(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }

uint32_t __stdcall MouseDevice::SetCooperativeLevel(void*, uint32_t)
{
	GetLogger()->Trace("%s\n", __FUNCTION__);
	return ResultCode::Ok;
}

uint32_t __stdcall MouseDevice::GetObjectInfo(void*, uint32_t, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::GetDeviceInfo(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::RunControlPanel(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::Initialize(void*, int32_t, GUID&) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::CreateEffect(GUID&, void*, void**, void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::EnumEffects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::GetEffectInfo(void*, GUID&) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::GetForceFeedbackState(uint32_t*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::SendForceFeedbackCommand(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::EnumCreatedEffectObjects(void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::Escape(void*) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::Poll() { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::SendDeviceData(void*, void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::EnumEffectsInFile(void*, void* callback, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }
uint32_t __stdcall MouseDevice::WriteEffectsToFile(void*, uint32_t, void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); return ResultCode::Ok; }