#include "subtitans.h"
#include "clipper.h"

using namespace DDraw;

Clipper::Clipper()
{
	TRACELOG("%s\n", __FUNCTION__);
	referenceCount = 0;
}

Clipper::~Clipper() { TRACELOG("%s\n", __FUNCTION__); }

// IUnknown
uint32_t __stdcall Clipper::QueryInterface(GUID* guid, void** result)
{
	TRACELOG("%s\n", __FUNCTION__);

	GetLogger()->Error("%s %s\n", __FUNCTION__, "unknown interface");
	*result = nullptr;
	return ResultCode::NoInterface;
}

uint32_t __stdcall Clipper::AddRef() 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	
	referenceCount++;

	return ResultCode::Ok; 
}

uint32_t __stdcall Clipper::Release() 
{ 
	TRACELOG("%s (Remaining references %i)\n", __FUNCTION__, referenceCount);

	if (--referenceCount == 0)
		delete this;

	return ResultCode::Ok;
}

// clipper
uint32_t __stdcall Clipper::GetClipList(void*, void*, void*) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	return ResultCode::NoClipList;
}

uint32_t __stdcall Clipper::GetHWnd(void*) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Clipper::Initialize(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Clipper::IsClipListChanged(uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }
uint32_t __stdcall Clipper::SetClipList(void*, uint32_t) { GetLogger()->Error("%s\n", __FUNCTION__); UNIMPLEMENTED_EXIT(); return ResultCode::Ok; }

uint32_t __stdcall Clipper::SetHWnd(uint32_t, void*) 
{ 
	TRACELOG("%s\n", __FUNCTION__);
	return ResultCode::Ok;
}