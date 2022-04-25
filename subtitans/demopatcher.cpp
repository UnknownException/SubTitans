#include "subtitans.h"
#include <VersionHelpers.h>
#include "sleepwellpatch.h"
#include "movieheapcorruptionpatch.h"
#include "ddrawreplacementpatch.h"
#include "demopatcher.h"

// The bare minimum to get the Submarine Titans demo running.
// DirectDraw support is dropped, OpenGL or Software rendering is forced.
// No support for custom resolutions/widescreen is provided.
// If the game keeps crashing you must delete the savegame folder and create a profile in-game.
// Disabling the intro video through stconfig.exe might help improve stability.

DemoPatcher::DemoPatcher()
{

}

DemoPatcher::~DemoPatcher()
{

}

void DemoPatcher::Configure()
{
	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x006D1714;
	sleepWellPatch->FrameLimitMemoryAddress = 0x007EBF1C;
	sleepWellPatch->DisableOriginalLimiterSleepAddress = 0x006D173B;
	_patches.push_back(sleepWellPatch);

	Global::RenderWidth = GetConfiguration()->GetInt32(L"SETTING", L"Width", 0);
	if (Global::RenderWidth == 0)
		Global::RenderWidth = Global::MonitorWidth;

	Global::RenderHeight = GetConfiguration()->GetInt32(L"SETTING", L"Height", 0);
	if (Global::RenderHeight == 0)
		Global::RenderHeight = Global::MonitorHeight;

	bool isWindows7 = IsWindows7OrGreater() && !IsWindows8OrGreater();
	auto renderingBackend = GetConfiguration()->GetInt32(L"FEATURE", L"Renderer", Global::RenderingBackend::Automatic);

	auto ddrawReplacementPatch = new DDrawReplacementPatch();
	ddrawReplacementPatch->DDrawDetourAddress = 0x006A6B81;
	ddrawReplacementPatch->DInputDetourAddress = 0x00701C9E;
	ddrawReplacementPatch->WindowRegisterClassDetourAddress = 0x00561847;
	ddrawReplacementPatch->WindowCreateDetourAddress = 0x00561893;
	ddrawReplacementPatch->DInputAbsolutePositioningDetourAddress = 0x007020F0;
	ddrawReplacementPatch->ForceSoftwareRendering = renderingBackend == Global::RenderingBackend::Software
		|| (isWindows7 && renderingBackend == Global::RenderingBackend::Automatic); // Prefer software rendering on windows 7
	ddrawReplacementPatch->DInputReplacement = false; // Doesn't work that well with the demo
	_patches.push_back(ddrawReplacementPatch);

	Global::RetroShader = GetConfiguration()->GetBoolean(L"FEATURE", L"RetroShader", false);
}