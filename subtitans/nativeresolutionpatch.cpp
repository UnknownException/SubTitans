#include "subtitans.h"
#include "nativeresolutionpatch.h"

namespace NativeResolution{
	static unsigned long ControlPanelMarginLeft = 0;

	namespace RepositionBottomMenu{
		// Detour variables
		constexpr unsigned long DetourSize = 26;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = JmpFromAddress + DetourSize;

		__declspec(naked) void Implementation()
		{
			__asm mov ecx, 0x0A;
			__asm mov dword ptr ds:[esi + 0x60], edx;

			__asm mov edx, dword ptr ds:[esi + 0x8C]; 
			__asm cmp edx, 0xF0;
			__asm jne ASM_NR_RBM_LOOP;
			__asm add ecx, 0x01; // + 1 for Left TV panel repositioning (orig 0a)

		ASM_NR_RBM_LOOP:
			__asm mov edx, dword ptr ds:[esi + 0x8C];
			__asm cmp edx, 0xF0; // Only do this for 1280x1024 resolution modification
			__asm jne ASM_NR_RBM_NOTNATIVERES;
			__asm cmp ecx, 0x01; // Check if last (0x0B: Left TV panel) <-- Don't add default value if match
			__asm jne ASM_NR_RBM_DONTOVERRIDEDEFAULT;
			__asm mov edx, 0x00;

		ASM_NR_RBM_DONTOVERRIDEDEFAULT:
			__asm add edx, [ControlPanelMarginLeft]; // Widescreen reposition value

		ASM_NR_RBM_NOTNATIVERES:
			// Restore
			__asm mov ebx, dword ptr ds:[eax];
			__asm add ebx, edx;
			__asm mov dword ptr ds:[eax], ebx;
			__asm add eax, 0x04;
			__asm dec ecx;
			__asm jnz ASM_NR_RBM_LOOP;

			__asm jmp [JmpBackAddress];
		}
	}

	namespace RenameSetting{
		// Detour variables
		constexpr unsigned long DetourSize = 6;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = JmpFromAddress + DetourSize;
		static unsigned long FunctionAddress = 0;

		// Function specific variables
		static char* CurrentStringPtr = 0;
		static char TargetString[] = { '1', '2', '8', '0', 'x', '1', '0', '2', '4', 0x00 };
		static char NewString[] = { 'N', 'A', 'T', 'I', 'V', 'E', ' ', 'R', 'E', 'S', 'O', 'L', 'U', 'T', 'I', 'O', 'N', 0x00 };

		__declspec(naked) void Implementation()
		{
			__asm mov [CurrentStringPtr], eax;

			__asm pushad;
			__asm pushfd;
				if (strcmp(TargetString, CurrentStringPtr) == 0)
					CurrentStringPtr = NewString;
			__asm popfd;
			__asm popad;

			__asm push [CurrentStringPtr];
			__asm call [FunctionAddress];
			__asm jmp [JmpBackAddress];
		}
	}

	namespace RedesignFrame{
		// Detour variables
		constexpr unsigned long DetourSize = 18;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = JmpFromAddress + DetourSize;
		static unsigned long TeamIdMemoryAddress = 0;
		static unsigned long DrawFunctionAddress = 0;

		// Function specific variables
		static unsigned char* ImagePtr;
		static unsigned char LastTeamId = 0; // Required for check if frame has to be remade
		const int HeaderAndColorTableSize = 40 /* 0x28 */ + 1024;
		static unsigned char* FrameBuffer = nullptr;
		static int Width = 0;
		static int Height = 0;

		void FillRect(unsigned int sourceX, unsigned int sourceY, unsigned int targetX, unsigned int targetY, unsigned int width, unsigned int height)
		{
			int sourceWidth = 0;
			memcpy(&sourceWidth, ImagePtr + 0x04, sizeof(int)); // Always 1280

			int sourceHeight = 0;
			memcpy(&sourceHeight, ImagePtr + 0x08, sizeof(int)); // Always 1024

			int targetWidth = 0;
			memcpy(&targetWidth, FrameBuffer + 0x04, sizeof(int));

			int targetHeight = 0;
			memcpy(&targetHeight, FrameBuffer + 0x08, sizeof(int));

			// Lower height is ok (720 is lowest)
			if (sourceWidth != 1280 || sourceHeight != 1024 || targetWidth < sourceWidth)
				return;

			sourceY = sourceHeight - height - sourceY;
			targetY = targetHeight - height - targetY;

			unsigned long sourceBoundary = (unsigned long)ImagePtr + HeaderAndColorTableSize + sourceWidth * sourceHeight;
			unsigned long targetBoundary = (unsigned long)FrameBuffer + HeaderAndColorTableSize + targetWidth * targetHeight;

			for (unsigned int line = 0; line < height; ++line)
			{
				bool outOfBounds = false;

				unsigned char* sourcePtr = ImagePtr + HeaderAndColorTableSize; // Set pointer past color table
				sourcePtr += (line + sourceY) * sourceWidth + sourceX;

				unsigned long sourceReadEndPosition = (unsigned long)sourcePtr + width;
				if (sourceReadEndPosition > sourceBoundary)
					outOfBounds = true;

				unsigned char* targetPtr = FrameBuffer + HeaderAndColorTableSize; // Set pointer past color table
				targetPtr += (line + targetY) * targetWidth + targetX;

				unsigned long targetReadEndPosition = (unsigned long)targetPtr + width;
				if (targetReadEndPosition > targetBoundary)
					outOfBounds = true;

				if (!outOfBounds)
					memcpy(targetPtr, sourcePtr, width);
			}
		}

		void Build()
		{
			int imageSize = Width * Height;
			memset(FrameBuffer, 0x00, HeaderAndColorTableSize + imageSize);

			memcpy(FrameBuffer, ImagePtr, HeaderAndColorTableSize);
			memcpy(FrameBuffer + 0x04, &Width, sizeof(int));
			memcpy(FrameBuffer + 0x08, &Height, sizeof(int));
			memcpy(FrameBuffer + 0x14, &imageSize, sizeof(int));

			// Top bar
			const int topLeftWidth = 426;
			const int topMidWidth = 448;
			const int topRightWidth = 406;
			const int topHeight = 30;

			FillRect(0, 0, 0, 0, topLeftWidth, topHeight); // Left
			const int topMidRequiredWidth = Width - topRightWidth - topLeftWidth;
			for (int i = 0; i < topMidRequiredWidth;)
			{
				int drawWidth = topMidWidth;
				if (i + drawWidth > topMidRequiredWidth)
					drawWidth -= i + drawWidth - topMidRequiredWidth;

				FillRect(topLeftWidth, 0, topLeftWidth + i, 0, drawWidth, topHeight); // Middle

				i += drawWidth;
			}
			FillRect(1280 - topRightWidth, 0, Width - topRightWidth, 0, topRightWidth, topHeight); // Right
	
			// Left bar
			const int leftWidth = 12;
			const int leftHeight = 772; // Not precise, doesn't matter; looks good
			const int leftBottomHeight = 42;

			const int leftRequiredHeight = Height - topHeight - leftBottomHeight;
			for (int i = 0; i < leftRequiredHeight;)
			{
				int drawHeight = leftHeight;
				if (i + drawHeight > leftRequiredHeight)
					drawHeight -= i + drawHeight - leftRequiredHeight;

				FillRect(0, topHeight, 0, topHeight + i, leftWidth, drawHeight); // Top

				i += drawHeight;
			}
			FillRect(0, 1024 - leftBottomHeight, 0, Height - leftBottomHeight, leftWidth, leftBottomHeight); // Bottom

			// Bottom bar
			const int bottomHeight = 32;
			const int bottomMidWidth = 692;
			const int bottomRightWidth = 133;

			const int bottomRequiredWidth = Width - leftWidth - bottomRightWidth;
			for (int i = 0; i < bottomRequiredWidth;)
			{
				int drawWidth = bottomMidWidth;
				if (i + drawWidth > bottomRequiredWidth)
					drawWidth -= i + drawWidth - bottomRequiredWidth;

				FillRect(306, 1024 - bottomHeight, leftWidth + i, Height - bottomHeight, drawWidth, bottomHeight); // Left

				i += drawWidth;
			}
			FillRect(1280 - bottomRightWidth, 1024 - bottomHeight, Width - bottomRightWidth, Height - bottomHeight, bottomRightWidth, bottomHeight); // Right

			// Right bar
			const int rightWidth = 28;
			const int rightTopHeight = 336;
			const int rightMidHeight = 422;
			const int rightBottomHeight = 204;

			FillRect(1280 - rightWidth, topHeight, Width - rightWidth, topHeight, rightWidth, rightTopHeight); // Top
			const int rightMidRequiredHeight = Height - rightTopHeight - rightBottomHeight - topHeight - bottomHeight;
			for (int i = 0; i < rightMidRequiredHeight;)
			{
				int drawHeight = rightMidHeight;
				if (i + drawHeight > rightMidRequiredHeight)
					drawHeight -= i + drawHeight - rightMidRequiredHeight;

				FillRect(1280 - rightWidth, topHeight + rightTopHeight, Width - rightWidth, topHeight + rightTopHeight + i, rightWidth, drawHeight); // Mid

				i += drawHeight;
			}
			FillRect(1280 - rightWidth, 1024-bottomHeight-rightBottomHeight, Width - rightWidth, Height - bottomHeight - rightBottomHeight, rightWidth, rightBottomHeight); // Bottom
		}

		unsigned long _teamIdMemoryPtr = 0;
		__declspec(naked) void Implementation()
		{
			__asm mov dword ptr ss:[ebp - 0x08], eax;

			// Check image properties
			__asm cmp dword ptr ds:[eax], 0x28; // Header size (bmp (v4))
			__asm jne ASM_NR_RF_RENDERFRAME;
			__asm cmp dword ptr ds:[eax + 0x04], 0x500; // Width
			__asm jne ASM_NR_RF_RENDERFRAME;
			__asm cmp dword ptr ds:[eax + 0x08], 0x400; // Height
			__asm jne ASM_NR_RF_RENDERFRAME;
			__asm cmp word ptr ds:[eax + 0x0C], 0x01; // Layers
			__asm jne ASM_NR_RF_RENDERFRAME;
			__asm cmp word ptr ds:[eax + 0x0E], 0x08; // Bits
			__asm jne ASM_NR_RF_RENDERFRAME;
			__asm cmp dword ptr ds:[eax + 0x20], 0x100 // Color Table items 
			__asm jne ASM_NR_RF_RENDERFRAME;

			// Get original image
			__asm mov [ImagePtr], eax; 
			
			// Get team id address
			__asm mov eax, [TeamIdMemoryAddress];
			__asm mov eax, [eax];
			__asm mov [_teamIdMemoryPtr], eax;

			// Restore eax
			__asm mov eax, [ImagePtr]
			
			// Uninitialized teamid?			
			__asm cmp byte ptr ds:[_teamIdMemoryPtr], 0x00;
			__asm je ASM_NR_RF_RENDERFRAME;

			// Check if teams have changes
			__asm xor eax, eax;
			__asm mov al, [LastTeamId];
			__asm cmp byte ptr ds:[_teamIdMemoryPtr], al;
			__asm je ASM_NR_RF_OVERRIDEFRAMEPTR;

			// Store team id
			__asm mov al, byte ptr ds:[_teamIdMemoryPtr];
			__asm mov [LastTeamId], al;

			// Create new frame
			__asm pushad;
			__asm pushfd;
				Build();
			__asm popfd;
			__asm popad;

		ASM_NR_RF_OVERRIDEFRAMEPTR:
			__asm mov eax, [FrameBuffer];

		ASM_NR_RF_RENDERFRAME:
			__asm push eax;
			__asm push 1;
			__asm push 0;
			__asm push 0;
			__asm call [DrawFunctionAddress];
			__asm add esp, 0x10;

			__asm jmp [JmpBackAddress];
		}
	}

	namespace RepositionBriefing {
		// Detour variables
		constexpr unsigned long DetourSize = 6;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = JmpFromAddress + DetourSize;

		__declspec(naked) void Implementation()
		{
			__asm sub ecx, [ControlPanelMarginLeft];
			__asm mov dword ptr ds:[esi + 0x10C], ecx;
			__asm jmp [JmpBackAddress];
		}
	}
}

NativeResolutionPatch::NativeResolutionPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);
	
	GuiRescalerAddress = 0;

	QueueScreenAddress = 0;

	ScreenInitialResizeWidthAddress = 0;
	ScreenInitialResizeHeightAddress = 0;

	ScreenResizeWidthCompareAddress = 0;
	ScreenResizeWidthAddress = 0;
	ScreenResizeHeightAddress = 0;

	GamefieldPresetWidthAddress = 0;
	GamefieldPresetHeightAddress = 0;

	GamefieldHeightReducingAddress = 0;
	GamefieldHeightRestorationAddress = 0;

	MovieWidthAddress = 0;
	MovieHeightAddress = 0;

	RepositionBottomMenuDetourAddress = 0;
	RenameSettingsDetourAddress = 0;
	RenameSettingsFunctionAddress = 0;
	RedesignFrameDetourAddress = 0;
	RedesignFrameTeamIdMemoryAddress = 0;
	RedesignFrameDrawFunctionAddress = 0;
	RepositionBriefingDetourAddress = 0;
}

NativeResolutionPatch::~NativeResolutionPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);

	if (NativeResolution::RedesignFrame::FrameBuffer != nullptr)
		delete[] NativeResolution::RedesignFrame::FrameBuffer;
}

bool NativeResolutionPatch::Validate()
{
	if(!GuiRescalerAddress ||
		!QueueScreenAddress ||
		!ScreenInitialResizeWidthAddress ||
		!ScreenInitialResizeHeightAddress ||
		!ScreenResizeWidthCompareAddress ||
		!ScreenResizeWidthAddress ||
		!ScreenResizeHeightAddress ||
		!GamefieldPresetWidthAddress ||
		!GamefieldPresetHeightAddress ||
		!GamefieldHeightReducingAddress ||
		!GamefieldHeightRestorationAddress ||
		!MovieWidthAddress ||
		!MovieHeightAddress ||
		!RepositionBottomMenuDetourAddress ||
		!RenameSettingsDetourAddress ||
		!RenameSettingsFunctionAddress ||
		!RedesignFrameDetourAddress ||
		!RedesignFrameTeamIdMemoryAddress ||
		!RedesignFrameDrawFunctionAddress ||
		!RepositionBriefingDetourAddress)
		return false;

	return true;
}

// Overwrites 1280x1024
bool NativeResolutionPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	GetLogger()->Informational("Resolution %ix%i\n", screenWidth, screenHeight);

	if (screenWidth < 1280 || screenHeight < 720) // Don't patch; user should use 800x600/1024x768
		return true;

	if (screenWidth == 1366 && screenHeight == 768)
	{
		GetLogger()->Warning("1366x768 causes rendering issues; trying 1280x768\n");
		screenWidth = 1280;
	}

	unsigned char buffer[4];

	/*
	// Create window Width (store @ 807100): Default 800
	/* memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6C5, buffer, sizeof(int)))
		return false;

	// Create window Height (store @ 807104): Default 600
	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(0x0056C6CF, buffer, sizeof(int)))
		return false;
	*/
	
	// GUI Rescaler?
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(GuiRescalerAddress, buffer, sizeof(int)))
		return false;

	// Queuescreen bottom @ ingame patch (0: 800, 2:1024, 6: 1280)
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(QueueScreenAddress, buffer, sizeof(int)))
		return false;

	// Initial screen width/height
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(ScreenInitialResizeWidthAddress, buffer, sizeof(int)))
		return false;

	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(ScreenInitialResizeHeightAddress, buffer, sizeof(int)))
		return false;

	// Screen resize compare value
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(ScreenResizeWidthCompareAddress, buffer, sizeof(int)))
		return false;

	// Screen resize values (width, height)
	memcpy(buffer, &screenWidth, sizeof(int));
	if (!MemoryWriter::Write(ScreenResizeWidthAddress, buffer, sizeof(int)))
		return false;

	memcpy(buffer, &screenHeight, sizeof(int));
	if (!MemoryWriter::Write(ScreenResizeHeightAddress, buffer, sizeof(int)))
		return false;

	// Gamefield preset values (width, height)
	int fieldWidth = screenWidth - 40;
	memcpy(buffer, &fieldWidth, sizeof(int));
	if (!MemoryWriter::Write(GamefieldPresetWidthAddress, buffer, sizeof(int)))
		return false;

	int fieldHeight = screenHeight - 62;
	memcpy(buffer, &fieldHeight, sizeof(int));
	if (!MemoryWriter::Write(GamefieldPresetHeightAddress, buffer, sizeof(int)))
		return false;

	unsigned char nopGamefieldResizeArray[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	if (!MemoryWriter::Write(GamefieldHeightReducingAddress, nopGamefieldResizeArray, 6))
		return false;

	if (!MemoryWriter::Write(GamefieldHeightRestorationAddress, nopGamefieldResizeArray, 5))
		return false;

	// The movie resolution patch does NOT scale the movie
	// Don't use native resolution until menu renders at native resolution (if)

	// Movie resolution patch (Width)
	constexpr int movieWidth = 800;
	memcpy(buffer, &movieWidth, sizeof(int));
	if (!MemoryWriter::Write(MovieWidthAddress, buffer, sizeof(int)))
		return false;

	// Movie resolution patch (Height)
	constexpr int movieHeight = 600;
	memcpy(buffer, &movieHeight, sizeof(int));
	if (!MemoryWriter::Write(MovieHeightAddress, buffer, sizeof(int)))
		return false;

	// Patch variables
	NativeResolution::ControlPanelMarginLeft = (screenWidth - 1280) / 2;

	NativeResolution::RedesignFrame::Width = screenWidth;
	NativeResolution::RedesignFrame::Height = screenHeight;
	NativeResolution::RedesignFrame::FrameBuffer = new unsigned char[NativeResolution::RedesignFrame::HeaderAndColorTableSize + screenWidth * screenHeight];

	NativeResolution::RepositionBottomMenu::JmpFromAddress = RepositionBottomMenuDetourAddress;
	NativeResolution::RepositionBottomMenu::JmpBackAddress = NativeResolution::RepositionBottomMenu::JmpFromAddress + NativeResolution::RepositionBottomMenu::DetourSize;
	if (!Detour::Create(NativeResolution::RepositionBottomMenu::JmpFromAddress, NativeResolution::RepositionBottomMenu::DetourSize, (unsigned long)NativeResolution::RepositionBottomMenu::Implementation))
		return false;

	NativeResolution::RenameSetting::JmpFromAddress = RenameSettingsDetourAddress;
	NativeResolution::RenameSetting::JmpBackAddress = NativeResolution::RenameSetting::JmpFromAddress + NativeResolution::RenameSetting::DetourSize;
	NativeResolution::RenameSetting::FunctionAddress = RenameSettingsFunctionAddress;
	if (!Detour::Create(NativeResolution::RenameSetting::JmpFromAddress, NativeResolution::RenameSetting::DetourSize, (unsigned long)NativeResolution::RenameSetting::Implementation))
		return false;

	NativeResolution::RedesignFrame::JmpFromAddress = RedesignFrameDetourAddress;
	NativeResolution::RedesignFrame::JmpBackAddress = NativeResolution::RedesignFrame::JmpFromAddress + NativeResolution::RedesignFrame::DetourSize;
	NativeResolution::RedesignFrame::TeamIdMemoryAddress = RedesignFrameTeamIdMemoryAddress;
	NativeResolution::RedesignFrame::DrawFunctionAddress = RedesignFrameDrawFunctionAddress;
	if (!Detour::Create(NativeResolution::RedesignFrame::JmpFromAddress, NativeResolution::RedesignFrame::DetourSize, (unsigned long)NativeResolution::RedesignFrame::Implementation))
		return false;

	NativeResolution::RepositionBriefing::JmpFromAddress = RepositionBriefingDetourAddress;
	NativeResolution::RepositionBriefing::JmpBackAddress = NativeResolution::RepositionBriefing::JmpFromAddress + NativeResolution::RepositionBriefing::DetourSize;
	if (!Detour::Create(NativeResolution::RepositionBriefing::JmpFromAddress, NativeResolution::RepositionBriefing::DetourSize, (unsigned long)NativeResolution::RepositionBriefing::Implementation))
		return false;

	return true;
}

const wchar_t* NativeResolutionPatch::ErrorMessage()
{
	return L"Failed to apply Native Resolution patch";
}