# Linux & Proton (Steam) -> Upgrade from v1.0 to v1.1
If you are running the game on Linux via Steam/Proton, the upgrade to v1.1 requires a workaround.

### How to Run the Patch via Steam/Proton
Because the patcher needs to run within your game's specific Wine context, use this renaming workaround to launch it through Steam:

0. **Download:** Ignore the guide and just download the patch file from https://steamcommunity.com/sharedfiles/filedetails/?id=2129291420
1. **Register fix:** Before attempting to patch, **launch the game at least once** with `subtitans.dll` already placed in your game folder. This will fix the version registry key.
2. **Create a Backup:** Navigate to your game folder and find `STEditor.exe`. Create a backup copy of it.
3. **Rename the Patcher:** Copy the patch file `st1_mj0to0_1_usa.exe` to the game folder and rename it to `STEditor.exe`.
4. **Launch the Patcher:** Launch the game and select the **"Game Editor"** option in Steam. This tricks Proton into executing the patcher with the correct context.
5. **Restore Original Files:** Once patching is finished you must delete the patch file `STEditor.exe`. Don't forget to restore your backup copy of `STEditor.exe`
