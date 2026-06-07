# SubTitans ![GitHub all releases](https://img.shields.io/github/downloads/UnknownException/SubTitans/total)
## Unofficial patch for Submarine Titans

### Requirements
* Windows 11 or a Linux distribution with Wine or Proton
* Retail v1.1 or GOG v1.1

> Support for v1.0, Demo, and the Technology Demo is limited.
> These versions are not actively tested, and some features may be missing.

### Instructions
1. Copy & paste d3drm.dll, subtitans.dll and subtitans.ini into your Submarine Titans folder.
2. Open STConfig.exe and select 1280x1024.
3. Run the game.

> **For Steam users**: At least start the game once before applying this patch. steam_installscript.vdf will not be applied if you skip this step.

### Features
* Support for any resolution in-game (tested up to 3840x2160)
* OpenGL and Software rendering replacement for DirectDraw to improve compatibility and performance
* DInput replacement
* Improved scrolling
* Added mission skip cheat (Retail/GOG v1.1; workaround for progression bugs)
* Fixes various internal errors
* Fixes alt-tab crashes
* Fixes video issues
* Support for display scaling
* Palette color fix

### Known bugs
* Regions next to the in-game command panel aren't selectable/clickable.

## Question and Answers
### How to update Submarine Titans to version 1.1?
> Windows: https://steamcommunity.com/sharedfiles/filedetails/?id=2129291420 \
> Linux: See [PROTON-v1_0-v_1_1.md](PROTON-v1_0-v1_1.md)

### Why am I getting MSVCP140.dll errors?
> You're missing some libraries on your computer, you'll need to install the Visual C++ redistributable provided by Microsoft. \
> vc_redist.x86.exe ( https://aka.ms/vc14/vc_redist.x86.exe )

### I've got a problem in-game with my mouse and/or keyboard.
> Open SubTitans.ini and set CustomInput to *false*. \
> This will turn off the DInput reimplementation. 

### How can I use a custom or GOG's DDraw wrapper?
> Open SubTitans.ini and set Renderer to **1**.
> 
> Only the OpenGL renderer and Software renderer are tested while developing this patch. \
> Custom wrappers might introduce new issues, please validate if issues you observe also occur under the OpenGL/Software renderer.

### The Submarine Titans (technology) demo crashes with or without this patch.
> The support for the demo versions of Submarine Titans is bare minimum. Please don't report issues regarding the demo, it will be ignored.

### Why would I want to enable the experimental ASLR/DEP option?
> Many (older) applications have inherent vulnerabilities, and exposing them over the internet (such as in multiplayer modes) increases the attack vector by a huge margin.
> 
> By enabling **ASLR (Address Space Layout Randomization)** and **DEP (Data Execution Prevention)**, you tighten security by making the application's memory space less predictable. While this doesn't patch the vulnerabilities within the game itself, it makes them significantly harder for an attacker to successfully exploit

### How do I use the mission skip cheat?
> Load the mission you want to skip and write 'orbiton' without quotes in the chatbox. \
> Exit to the menu and select a random campaign to start the next mission.

### How do I uninstall the patch?
> Deleting SubTitans.dll will disable the in-game patches.
> 
> Do **NOT** delete d3drm.dll, the game will stop working without it.
