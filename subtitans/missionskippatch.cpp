#include "subtitans.h"
#include "missionskippatch.h"

/*
	Issue: Some missions don't always trigger the mission end event
	Solution: Add a cheat for mission skipping
*/

namespace MissionSkip {
	// Runtime variables
	static char* CurrentMapNamePointer = 0;

	// Function specific variables
	const char MissionSkipCheatString[] = { 'O', 'R', 'B', 'I', 'T', 'O', 'N', 0x00 };
	const char MissionValidationString[] = { 'M', 'I', 'S', 'S', 'I', 'O', 'N', 'S', '\\', 'M', 'I', 'S', 'S', 0x00 };

	// For Detours
	static char OrbitonMissionPathPattern[] = { '%', 's', '%', 's', '%', 's', '0', '0', '0', 0x00 };
	static char DefaultMissionPathPattern[] = { '%', 's', '%', 's', '%', 's', '%', 'd', '0', '1', 0x00 };
	static bool OrbitonActivated = false;

	void ActivateOrbiton()
	{
		GetLogger()->Informational("Orbiton is activated for %s\n", CurrentMapNamePointer);

		// Copy map name and convert to uppercase
		char cheatActivatedMapName[32] = { 0x00, };
		strncpy_s(cheatActivatedMapName, sizeof(cheatActivatedMapName), CurrentMapNamePointer, strlen(CurrentMapNamePointer));
		if (_strupr_s(cheatActivatedMapName, sizeof(cheatActivatedMapName)) != 0)
		{
			GetLogger()->Error("Failed to convert map name to uppercase\n");
			GetLogger()->Informational("Orbiton has been disabled\n");
			return;
		}

		constexpr int missionValidationStringLength = sizeof(MissionValidationString) - 1;
		constexpr int missionNumberStringLength = 3;
		
		// Validate if the string has the expected length
		if(strlen(cheatActivatedMapName) < missionValidationStringLength + missionNumberStringLength)
		{
			GetLogger()->Warning("Current map has an unexpected length, are you sure this is a mission?\n");
			GetLogger()->Informational("Orbiton has been disabled\n");
			return;
		}
		
		// Check if the current map is a mission
		for (unsigned int i = 0; i < missionValidationStringLength; ++i)
		{	
			if (MissionValidationString[i] != cheatActivatedMapName[i])
			{
				GetLogger()->Warning("You're only allowed to skip mission maps!\n");
				GetLogger()->Informational("Orbiton has been disabled\n");
				return;
			}
		}
		
		char missionNumberAsString[missionNumberStringLength + 1] = { 0x00, };
		memcpy_s(missionNumberAsString, sizeof(missionNumberAsString), cheatActivatedMapName + missionValidationStringLength, missionNumberStringLength);
		int missionNumber = atoi(missionNumberAsString + 1); // Skip team
		if (missionNumber < 10)
			missionNumber++;

		sprintf_s(missionNumberAsString + 1, sizeof(missionNumberAsString) - 1, "%02d", missionNumber);
		memcpy_s(cheatActivatedMapName + missionValidationStringLength, missionNumberStringLength, missionNumberAsString, missionNumberStringLength);

		memcpy_s(OrbitonMissionPathPattern + (sizeof(OrbitonMissionPathPattern) - 1) - missionNumberStringLength, missionNumberStringLength, missionNumberAsString, missionNumberStringLength);

		GetLogger()->Informational("Next mission will be forced to: %s\nExit to main menu and start a new campaign.\n", cheatActivatedMapName);

		OrbitonActivated = true;
	}

	namespace AddCheatCode {
		// Detour variables
		constexpr unsigned long DetourSize = 7;
		static unsigned long JmpFromAddress = 0; // Inside of TECH cheat
		static unsigned long JmpBackAddress = 0; // Execute TECH cheat
		static unsigned long JmpBackAlternativeAddress = 0; // Jump to begin of FOW cheat
		static unsigned long CheatValidationFunctionAddress = 0;
	
		__declspec(naked) void Implementation()
		{
			// Restore code
			__asm add esp, 0x0C;
			__asm test eax, eax;
			__asm jnz missionSkipCheat;
			__asm jmp [JmpBackAddress];

		missionSkipCheat:
			// New cheat
			__asm push 0x07;
			__asm push offset [MissionSkipCheatString];
			__asm push ebx;
			__asm call [CheatValidationFunctionAddress];
			__asm test eax, eax;
			__asm jnz missionSkipCheatEpilogue;

			__asm pushfd;
			__asm pushad;

			ActivateOrbiton();

			__asm popad;
			__asm popfd;

		missionSkipCheatEpilogue:
			__asm jmp [JmpBackAlternativeAddress];
		}
	}

	namespace FullPathOverride
	{
		// Detour variables
		constexpr unsigned long DetourSize = 5;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = 0;

		__declspec(naked) void Implementation()
		{
			__asm cmp [OrbitonActivated], 0x00;
			__asm je fullPathOrbitonDeactivated;

			__asm cmp ecx, 0x01;
			__asm je disableOrbitonForTutorial;

			__asm push offset [OrbitonMissionPathPattern];
			__asm jmp [JmpBackAddress];

		disableOrbitonForTutorial:
			__asm mov [OrbitonActivated], 0x00;
		
		fullPathOrbitonDeactivated:
			__asm push offset [DefaultMissionPathPattern];
			__asm jmp [JmpBackAddress];
		}
	}

	namespace RelativePathOverride
	{
		// Detour variables
		constexpr unsigned long DetourSize = 5;
		static unsigned long JmpFromAddress = 0;
		static unsigned long JmpBackAddress = 0;

		__declspec(naked) void Implementation()
		{
			__asm cmp [OrbitonActivated], 0x00;
			__asm je fullPathOrbitonDeactivated;

			__asm push offset [OrbitonMissionPathPattern + 0x02];
			__asm mov [OrbitonActivated], 0x00;
			__asm jmp [JmpBackAddress];

		fullPathOrbitonDeactivated:
			__asm push offset [DefaultMissionPathPattern + 0x02];
			__asm jmp [JmpBackAddress];
		}
	}
}

MissionSkipPatch::MissionSkipPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	AddCheatCodeDetourAddress = 0;
	AddCheatCodeAlternativeReturnAddress = 0;
	FullMapPathDetourAddress = 0;
	RelativeMapPathDetourAddress = 0;

	CurrentMapNameVariable = 0;
	CheatValidationFunctionAddress = 0;
}

MissionSkipPatch::~MissionSkipPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool MissionSkipPatch::Validate()
{
	return AddCheatCodeDetourAddress &&
		AddCheatCodeAlternativeReturnAddress &&
		FullMapPathDetourAddress &&
		RelativeMapPathDetourAddress &&
		CurrentMapNameVariable &&
		CheatValidationFunctionAddress;
}

bool MissionSkipPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	MissionSkip::CurrentMapNamePointer = (char*)CurrentMapNameVariable;

	MissionSkip::AddCheatCode::JmpFromAddress = AddCheatCodeDetourAddress;
	MissionSkip::AddCheatCode::JmpBackAddress = MissionSkip::AddCheatCode::JmpFromAddress + MissionSkip::AddCheatCode::DetourSize;
	MissionSkip::AddCheatCode::JmpBackAlternativeAddress = AddCheatCodeAlternativeReturnAddress;
	MissionSkip::AddCheatCode::CheatValidationFunctionAddress = CheatValidationFunctionAddress;
	if (!Detour::Create(MissionSkip::AddCheatCode::JmpFromAddress, MissionSkip::AddCheatCode::DetourSize, (unsigned long)MissionSkip::AddCheatCode::Implementation))
		return false;

	MissionSkip::FullPathOverride::JmpFromAddress = FullMapPathDetourAddress;
	MissionSkip::FullPathOverride::JmpBackAddress = MissionSkip::FullPathOverride::JmpFromAddress + MissionSkip::FullPathOverride::DetourSize;
	if (!Detour::Create(MissionSkip::FullPathOverride::JmpFromAddress, MissionSkip::FullPathOverride::DetourSize, (unsigned long)MissionSkip::FullPathOverride::Implementation))
		return false;

	MissionSkip::RelativePathOverride::JmpFromAddress = RelativeMapPathDetourAddress;
	MissionSkip::RelativePathOverride::JmpBackAddress = MissionSkip::RelativePathOverride::JmpFromAddress + MissionSkip::RelativePathOverride::DetourSize;
	if (!Detour::Create(MissionSkip::RelativePathOverride::JmpFromAddress, MissionSkip::RelativePathOverride::DetourSize, (unsigned long)MissionSkip::RelativePathOverride::Implementation))
		return false;

	return true;
}

const wchar_t* MissionSkipPatch::ErrorMessage()
{
	return L"Failed to apply the mission skip patch";
}
