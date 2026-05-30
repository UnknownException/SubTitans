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
	memset(primaryBitmapInfo, 0, sizeof(BITMAPINFOHEADER));
	primaryBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	primaryBitmapInfo->bmiHeader.biPlanes = 1;
	primaryBitmapInfo->bmiHeader.biCompression = BI_RGB;
	primaryBitmapInfo->bmiHeader.biBitCount = 8;
	primaryBitmapInfo->bmiHeader.biClrUsed = 256;
	primaryBitmapInfo->bmiHeader.biSizeImage = 0;

	uint8_t* surfaceBuffer = nullptr;

	Mutex.lock();

	Global::RenderInformation currentRenderInformation;
	memset(&currentRenderInformation, 0, sizeof(Global::RenderInformation));

	int32_t currentWidth = 0;
	int32_t currentHeight = 0;
	
	while (IsThreadRunning)
	{
		Mutex.unlock();

		uint32_t drawTime = timeGetTime();

		if (surfaceBuffer)
		{
			auto dc = GetDC(Global::RenderWindow);

			if (currentRenderInformation.InternalWidth == currentRenderInformation.MonitorWidth && currentRenderInformation.InternalHeight == currentRenderInformation.MonitorHeight)
				SetDIBitsToDevice(dc, 0, 0, currentWidth, currentHeight, 0, 0, 0, currentHeight, surfaceBuffer, primaryBitmapInfo, DIB_RGB_COLORS);
			else
				StretchDIBits(dc, currentRenderInformation.Padding, 0, currentRenderInformation.AspectRatioCompensatedWidth, currentRenderInformation.MonitorHeight, 0, 0, currentWidth, currentHeight, surfaceBuffer, primaryBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

			ReleaseDC(Global::RenderWindow, dc);
		}

		drawTime = timeGetTime() - drawTime;
		
		// Render if event has been dispatched OR if the rendering will drop under 25fps as some screens like the loading screen do not dispatch the event
		WaitForSingleObject(Global::RenderEvent, 40 - min(drawTime, 40));

		Mutex.lock();

		// Not initialized
		if (PrimarySurface == nullptr)
			continue;

		// Stop rendering if the game isn't being focussed on
		HWND foregroundWindow = GetForegroundWindow();
		if (foregroundWindow != Global::GameWindow && foregroundWindow != Global::RenderWindow)
			continue;

		// Prevent flickering caused by palette changes
		if (PrimarySurface->PrimaryInvalid)
			continue;

		if (PrimarySurface->BitsPerPixel == 8)
		{
			// Can't render without palette
			if (!PrimarySurface->AttachedPalette)
				continue;

			primaryBitmapInfo->bmiHeader.biBitCount = 8;
			primaryBitmapInfo->bmiHeader.biClrUsed = 256;
		}
		else
		{
			primaryBitmapInfo->bmiHeader.biBitCount = 32;
			primaryBitmapInfo->bmiHeader.biClrUsed = 0;
		}

		primaryBitmapInfo->bmiHeader.biWidth = PrimarySurface->Width;
		primaryBitmapInfo->bmiHeader.biHeight = -PrimarySurface->Height;

		if (!surfaceBuffer || RecalculateSurface)
		{
			if (surfaceBuffer)
			{
				delete[] surfaceBuffer;

				auto dc = GetDC(Global::RenderWindow);
				PatBlt(dc, 0, 0, Global::MonitorWidth, Global::MonitorHeight, BLACKNESS);
				ReleaseDC(Global::RenderWindow, dc);
			}

			surfaceBuffer = new uint8_t[PrimarySurface->Stride * PrimarySurface->Height];
			RecalculateSurface = false;

			memcpy(&currentRenderInformation, &RenderInformation, sizeof(Global::RenderInformation));
		}

		PrimarySurface->PrimaryDrawMutex.lock();

		if (PrimarySurface->BitsPerPixel == 8)
		{
			PrimarySurface->AttachedPalette->Mutex.lock();
			memcpy(primaryBitmapInfo->bmiColors, PrimarySurface->AttachedPalette->RawPalette, sizeof(PrimarySurface->AttachedPalette->RawPalette));
			PrimarySurface->AttachedPalette->Mutex.unlock();
		}

		memcpy(surfaceBuffer, PrimarySurface->SurfaceBuffer, PrimarySurface->Stride * PrimarySurface->Height);

		SetEvent(Global::VerticalBlankEvent);

		PrimarySurface->PrimaryDrawMutex.unlock();

		currentWidth = PrimarySurface->Width;
		currentHeight = PrimarySurface->Height;
	}
	Mutex.unlock();

	if(surfaceBuffer)
		delete[] surfaceBuffer;

	delete[] (char*)primaryBitmapInfo;
}