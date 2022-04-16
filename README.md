# SubTitans
Unofficial Submarine Titans patch
![GitHub all releases](https://img.shields.io/github/downloads/UnknownException/SubTitans/total)

**Requirements**
* Windows 10 or Windows 11
* Supports Steam/Retail v1.0, Retail v1.1 and GOG v1.1

**Features**
* Support for any resolution in-game (All; tested up to 3840x2160)
* OpenGL and Software rendering replacement for DirectDraw to improve compatibility and performance (Retail v1.1)
* DInput replacement (Retail v1.1)
* Improved scrolling (Retail/GOG v1.1)
* Added mission skip cheat (Retail/GOG v1.1; workaround for progression bugs)
* Fixes various internal errors (All)
* Support for display scaling (Retail v1.0 & Retail v1.1)
* Fixes alt-tab crashes (Retail v1.0 & Retail v1.1)
* Windows 7 palette color fix (Retail v1.1)

**Instructions**
1. Copy & paste d3drm.dll and subtitans.dll in your Submarine Titans folder.
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
A: Open SubTitans.ini and set Renderer to 1. \

**Known bugs**
* None (yet)

**Uninstall**
* Deleting SubTitans.dll will disable the ingame patches.