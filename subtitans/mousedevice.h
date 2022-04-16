#pragma once
#include "idinputdevice7.h"

class MouseDevice : public DInput::IDInputDevice7 {
public:
	MouseDevice();
	virtual ~MouseDevice();

	// IUnknown
	virtual uint32_t __stdcall QueryInterface(GUID* guid, void** result) override;
	virtual uint32_t __stdcall AddRef() override;
	virtual uint32_t __stdcall Release() override;

	// Inherited via IDInputDevice7
	virtual uint32_t __stdcall GetCapabilities(DInput::Caps* caps) override;
	virtual uint32_t __stdcall EnumObjects(void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall GetProperty(GUID& guid, void*) override;
	virtual uint32_t __stdcall SetProperty(GUID& guid, void*) override;
	virtual uint32_t __stdcall Acquire() override;
	virtual uint32_t __stdcall Unacquire() override;
	virtual uint32_t __stdcall GetDeviceState(uint32_t, void*) override;
	virtual uint32_t __stdcall GetDeviceData(uint32_t, void*, void*, uint32_t) override;
	virtual uint32_t __stdcall SetDataFormat(DInput::DataFormat*) override;
	virtual uint32_t __stdcall SetEventNotification(void*) override;
	virtual uint32_t __stdcall SetCooperativeLevel(void*, uint32_t) override;
	virtual uint32_t __stdcall GetObjectInfo(void*, uint32_t, uint32_t) override;
	virtual uint32_t __stdcall GetDeviceInfo(void*) override;
	virtual uint32_t __stdcall RunControlPanel(void*, uint32_t) override;
	virtual uint32_t __stdcall Initialize(void*, int32_t, GUID&) override;
	virtual uint32_t __stdcall CreateEffect(GUID&, void*, void**, void*) override;
	virtual uint32_t __stdcall EnumEffects(void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall GetEffectInfo(void*, GUID&) override;
	virtual uint32_t __stdcall GetForceFeedbackState(uint32_t*) override;
	virtual uint32_t __stdcall SendForceFeedbackCommand(uint32_t) override;
	virtual uint32_t __stdcall EnumCreatedEffectObjects(void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall Escape(void*) override;
	virtual uint32_t __stdcall Poll() override;
	virtual uint32_t __stdcall SendDeviceData(void*, void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall EnumEffectsInFile(void*, void* callback, void*, uint32_t) override;
	virtual uint32_t __stdcall WriteEffectsToFile(void*, uint32_t, void*, uint32_t) override;

	// Custom
	uint32_t ReferenceCount;

	bool Initialized;
};