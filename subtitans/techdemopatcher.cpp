#include "subtitans.h"
#include "sleepwellpatch.h"
#include "ddrawreplacementpatch.h"
#include "registrypatch.h"
#include "shlobj.h"
#include "techdemopatcher.h"

// The bare minimum to get the Submarine Titans technology demo running.
// DirectDraw support is dropped, OpenGL or Software rendering is forced.
// No support for custom resolutions/widescreen is provided.
// If the game keeps crashing you must delete the savegame folder and create a profile in-game.
// Disabling the intro video through stconfig.exe might help improve stability.
// Doesn't work well without Administrator privileges.

TechDemoPatcher::TechDemoPatcher()
{
}

TechDemoPatcher::~TechDemoPatcher()
{

}

bool TechDemoPatcher::Initialize()
{
	if (!IsUserAnAdmin())
	{
		MessageBox(NULL
			, L"The Technology Demo has quite a few compatibility issues\n\n"
				L"Without Administrator privileges you'll most likely encounter registry errors."
			, L"Compatibility warning"
			, MB_OK | MB_ICONWARNING);
	}

	return true;
}

void TechDemoPatcher::Configure()
{
	auto sleepWellPatch = new SleepWellPatch();
	sleepWellPatch->DetourAddress = 0x00592FDA;
	sleepWellPatch->FrameLimitMemoryAddress = 0x006B92EC;
	sleepWellPatch->DisableOriginalLimiterSleepAddress = 0x00593001;
	_patches.push_back(sleepWellPatch);

	Global::RenderWidth = GetConfiguration()->GetInt32(L"SETTING", L"Width", 0);
	if (Global::RenderWidth == 0)
		Global::RenderWidth = Global::MonitorWidth;

	Global::RenderHeight = GetConfiguration()->GetInt32(L"SETTING", L"Height", 0);
	if (Global::RenderHeight == 0)
		Global::RenderHeight = Global::MonitorHeight;

	auto renderingBackend = GetConfiguration()->GetInt32(L"FEATURE", L"Renderer", Global::RenderingBackend::Automatic);

	auto ddrawReplacementPatch = new DDrawReplacementPatch();
	ddrawReplacementPatch->DDrawDetourAddress = 0x005778A1;
	ddrawReplacementPatch->DInputDetourAddress = 0x005B254E;
	ddrawReplacementPatch->WindowRegisterClassDetourAddress = 0x00561895;
	ddrawReplacementPatch->WindowCreateDetourAddress = 0x005618E1;
	ddrawReplacementPatch->VideoFormatCheckDetourAddress = 0;
	ddrawReplacementPatch->VideoScalingAddress = 0x005842B7;
	ddrawReplacementPatch->DInputAbsolutePositioningDetourAddress = 0x005B29A3;
	ddrawReplacementPatch->ForceSoftwareRendering = renderingBackend == Global::RenderingBackend::Software;
	ddrawReplacementPatch->DInputReplacement = false; // Doesn't work that well with the demo
	_patches.push_back(ddrawReplacementPatch);

	auto registryPatch = new RegistryPatch();
	registryPatch->DetourAddress = 0x00565E1E;
	_patches.push_back(registryPatch);

	Global::RetroShader = GetConfiguration()->GetBoolean(L"FEATURE", L"RetroShader", false);
}