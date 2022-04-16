#pragma once

class Surface;
class IRenderer {
protected:
	std::thread* RenderThread;
	std::atomic_bool IsThreadRunning;
	std::mutex Mutex;

	Surface* PrimarySurface;
	bool RecalculateSurface;

	Global::RenderInformation RenderInformation; 	// Copied from Shared for thread safety

public:
	IRenderer() {
		RenderThread = nullptr;
		IsThreadRunning = false;
		PrimarySurface = nullptr;
		RecalculateSurface = true;
		memset(&RenderInformation, 0, sizeof(Global::RenderInformation));
	}
	virtual ~IRenderer() {
		if (RenderThread) {
			IsThreadRunning = false;
			RenderThread->join();
			delete RenderThread;
			RenderThread = nullptr;
		}
	}

	virtual bool Create() = 0;
	virtual void OnCreatePrimarySurface(Surface* primarySurface) = 0;
	virtual void OnDestroyPrimarySurface() = 0;

protected:
	virtual void Run() = 0; // Render thread
};