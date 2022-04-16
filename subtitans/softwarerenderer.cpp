#include "subtitans.h"
#include "surface.h"
#include "softwarerenderer.h"

SoftwareRenderer::SoftwareRenderer()
{
}

SoftwareRenderer::~SoftwareRenderer()
{
}

bool SoftwareRenderer::Create()
{
	if (!Global::GameWindow)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "GameWindow hasn't been set");
		return false;
	}

	if (RenderThread)
	{
		GetLogger()->Warning("%s %s\n", __FUNCTION__, "already created");
		return false;
	}

	IsThreadRunning = true;
	RenderThread = new std::thread(&SoftwareRenderer::Run, this);

	return true;
}

void SoftwareRenderer::OnCreatePrimarySurface(Surface* primarySurface)
{
	Mutex.lock();
	Global::RenderInformation oldInformation;
	memcpy(&oldInformation, &RenderInformation, sizeof(Global::RenderInformation));
	RenderInformation = Global::GetRenderInformation();

	if (memcmp(&oldInformation, &RenderInformation, sizeof(Global::RenderInformation)) != 0)
		RecalculateSurface = true;

	PrimarySurface = primarySurface;
	Mutex.unlock();
}

void SoftwareRenderer::OnDestroyPrimarySurface()
{
	Mutex.lock();
	PrimarySurface = nullptr;
	Mutex.unlock();
}

void SoftwareRenderer::Run()
{
	BITMAPINFO* primaryBitmapInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
	uint8_t* surfaceBuffer = nullptr;

	Mutex.lock();
	while (IsThreadRunning)
	{
		Mutex.unlock();

		// Render if event has been dispatched OR if the rendering will drop under 25fps as some screens like the loading screen do not dispatch the event
		WaitForSingleObject(Global::RenderEvent, 40);

		Mutex.lock();

		// Not initialized
		if (PrimarySurface == nullptr)
			continue;

		// Stop rendering if the game isn't being focussed on
		if (GetForegroundWindow() != Global::GameWindow)
			continue;

		// Prevent flickering caused by palette changes
		if (PrimarySurface->PrimaryInvalid)
			continue;

		memset(primaryBitmapInfo, 0, sizeof(BITMAPINFOHEADER));
		primaryBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		primaryBitmapInfo->bmiHeader.biWidth = PrimarySurface->Stride; // Read only value
		primaryBitmapInfo->bmiHeader.biHeight = -PrimarySurface->Height; // Read only value
		primaryBitmapInfo->bmiHeader.biPlanes = 1;
		primaryBitmapInfo->bmiHeader.biCompression = BI_RGB;
		primaryBitmapInfo->bmiHeader.biBitCount = 8;
		primaryBitmapInfo->bmiHeader.biClrUsed = 256;
		primaryBitmapInfo->bmiHeader.biSizeImage = 0;

		PrimarySurface->PrimaryDrawMutex.lock();

		if (PrimarySurface->AttachedPalette)
		{
			PrimarySurface->AttachedPalette->Mutex.lock();
			memcpy(primaryBitmapInfo->bmiColors, PrimarySurface->AttachedPalette->RawPalette, sizeof(PrimarySurface->AttachedPalette->RawPalette));
			PrimarySurface->AttachedPalette->Mutex.unlock();
		}

		if (!surfaceBuffer || RecalculateSurface)
		{
			if (surfaceBuffer)
				delete[] surfaceBuffer;

			surfaceBuffer = new uint8_t[PrimarySurface->Stride * PrimarySurface->Height];
			RecalculateSurface = false;
		}

		memcpy(surfaceBuffer, PrimarySurface->SurfaceBuffer, PrimarySurface->Stride * PrimarySurface->Height);

		PrimarySurface->PrimaryDrawMutex.unlock();

		auto dc = GetDC(nullptr);

		if (RenderInformation.InternalWidth == RenderInformation.MonitorWidth && RenderInformation.InternalHeight == RenderInformation.MonitorHeight)
			SetDIBitsToDevice(dc, 0, 0, PrimarySurface->Width, PrimarySurface->Height, 0, 0, 0, PrimarySurface->Height, surfaceBuffer, primaryBitmapInfo, DIB_RGB_COLORS);
		else
			StretchDIBits(dc, RenderInformation.Padding, 0, RenderInformation.AspectRatioCompensatedWidth, RenderInformation.MonitorHeight, 0, 0, PrimarySurface->Width, PrimarySurface->Height, surfaceBuffer, primaryBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

		ReleaseDC(nullptr, dc);

		SetEvent(Global::VerticalBlankEvent);
	}
	Mutex.unlock();

	if(surfaceBuffer)
		delete[] surfaceBuffer;

	delete[] (char*)primaryBitmapInfo;
}