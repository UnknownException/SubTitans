# SubTitans
Unofficial Submarine Titans patch \
![GitHub all releases](https://img.shields.io/github/downloads/UnknownException/SubTitans/total)

**Requirements**
* Windows 7, Windows 10, Windows 11 or Wine (+ Ubuntu)
* Supports Steam/Retail v1.0, Retail v1.1 and GOG v1.1

**Features**
* Support for any resolution in-game (tested up to 3840x2160)
* OpenGL and Software rendering replacement for DirectDraw to improve compatibility and performance
* DInput replacement
* Improved scrolling
* Added mission skip cheat (Retail/GOG v1.1; workaround for progression bugs)
* Fixes various internal errors
* Fixes alt-tab crashes
* Support for display scaling
* Windows 7 palette color fix

**Instructions**
1. Copy & paste d3drm.dll, subtitans.dll and subtitans.ini into your Submarine Titans folder.
2. Open STConfig.exe and select 1280x1024.
3. Run the game.

**Q&A** \
Q: How do I use the mission skip cheat? \
A: Load the mission you want to skip and write 'orbiton' without quotes in the chatbox. \
Exit to the menu and select a random campaign to start the next mission. \
 \
Q: Help, I'm getting MSVCP140.dll errors! \
A: Install vc_redist.x86.exe ( https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads ). \
 \
Q: How to update Submarine Titans to version 1.1? \
A: https://steamcommunity.com/sharedfiles/filedetails/?id=2129291420 \
 \
Q: I've got a problem in-game with my mouse and/or keyboard. \
A: Open SubTitans.ini and set CustomInput to false, this should revert the input handling changes. \
 \
Q: I've got a problem with the OpenGL/Software renderer and want to use a custom DDraw wrapper. \
A: Open SubTitans.ini and set Renderer to 1.
 \
Q: The Submarine Titans demo crashes with or without this patch. \
A: The support for the demo version of Submarine Titans is bare minimum. Please don't report issues regarding the demo, it will be ignored.

**Known bugs**
* Regions next to the in-game command panel aren't selectable/clickable.

**Uninstall**
* Deleting SubTitans.dll will disable the in-game patches.