# SubTitans
Unofficial Submarine Titans patch

**Requirements**
* Windows 10
* Supports Steam/Retail v1.0, Retail v1.1 and GOG v1.1

**Features**
* Support for native resolution ingame (tested up to 3840x2160)
* Improved scrolling (Requires v1.1)
* Added mission skip cheat (Requires v1.1; workaround for progression bugs)
* Performance boost (Steam; not required for GOG)
* Fixes various internal errors (Steam; not required for GOG)
* Support for high DPI display resolutions (Steam; not required for GOG)
* Fixes alt-tab crashes (Steam; not required for GOG)
* Windows 7 color fix (Steam; not required for GOG) - deprecated

**Instructions**
1. Copy & paste d3drm.dll and subtitans.dll in your Submarine Titans folder.
2. Open STConfig.exe and select 1280x1024.
3. Run the game (through ST.exe or Steam).

**Q&A** 
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
Q: Why is the game running so slow? (Steam/Retail only) \
A: Submarine Titan uses DirectDraw, a deprecated piece of technology from a bygone era. \
There are DirectDraw replacements/wrappers available that might improve performance (like DXWrapper, DDrawCompat, DXGL, dgVoodoo 2 and others). \

**Known bugs**
* 1366*768 does not function correctly, reverts to 1280x768 ingame.

**Uninstall**
* Deleting SubTitans.dll will disable the ingame patches.