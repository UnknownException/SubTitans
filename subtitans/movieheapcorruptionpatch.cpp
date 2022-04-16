#include "subtitans.h"
#include "movieheapcorruptionpatch.h"

/*
	Issue: Trying to free (CoTaskMemFree) a static structure, which hasn't been created with a CoTaskMemAlloc call.
	Solution: The structure should be created with CoTaskMemAlloc
*/

namespace MovieHeapCorruption {
	// Function specific variables
	static unsigned long CurrentAllocatedMemoryAddress = 0;
	static void* NewAllocatedMemoryAddress = 0;

	void RecalculateStructPointer(unsigned long address)
	{
		unsigned long* addressPointer = (unsigned long*)address;
		unsigned long addition = 0;

		memcpy(&addition, addressPointer, sizeof(int));
		addition -= CurrentAllocatedMemoryAddress; // Get relative position in structure

		unsigned long newAddress = (unsigned long)NewAllocatedMemoryAddress;
		newAddress += addition;

		if (!MemoryWriter::Write(address, (unsigned char*)&newAddress, sizeof(int), false))
		{
			GetLogger()->Critical("Failed to update an offset for the movie player! (Address 0x%08x)\n", address);
			MessageBox(NULL, L"Unexpected failure while patching the movie player", L"SubTitans", MB_ICONERROR);
			ExitProcess(-1);
		}
	}

	// Functions specific variables
	static unsigned long StructurePatches[26];
	void __stdcall AllocateMemoryAndRedirectReferences()
	{
		NewAllocatedMemoryAddress = CoTaskMemAlloc(0x11A * 4);
		memset(NewAllocatedMemoryAddress, 0, 0x11A * 4);

		for (int i = 0; i < sizeof(StructurePatches) / sizeof(unsigned long); ++i)
		{
			RecalculateStructPointer(StructurePatches[i]);
		}

		CurrentAllocatedMemoryAddress = (unsigned long)NewAllocatedMemoryAddress;
	}

	// Detour variables
	constexpr unsigned long DetourSize = 5;
	static unsigned long JmpFromAddress = 0;
	static unsigned long JmpBackAddress = 0;
	__declspec(naked) void Implementation()
	{
		__asm pushad;
		__asm pushfd;

		__asm call [AllocateMemoryAndRedirectReferences];

		__asm popfd;
		__asm popad;

		__asm mov ecx, [NewAllocatedMemoryAddress];
		__asm mov dword ptr ss:[ebp - 0x10], ecx;

		// Restore code
		__asm mov ecx, 0x12;

		__asm jmp [JmpBackAddress];
	}
}

MovieHeapCorruptionPatch::MovieHeapCorruptionPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	AllocatedMemoryOffset = 0;
	StructurePointer = 0;
	memset(StructureOffsets, 0, sizeof(StructureOffsets));
	DetourAddress = 0;
}

MovieHeapCorruptionPatch::~MovieHeapCorruptionPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool MovieHeapCorruptionPatch::Validate()
{
	// Sanity check
	if (sizeof(StructureOffsets) != sizeof(MovieHeapCorruption::StructurePatches))
		return false;

	for (int i = 0; i < sizeof(StructureOffsets) / sizeof(unsigned long); ++i)
	{
		if (StructureOffsets[i] == 0)
			return false;
	}

	return AllocatedMemoryOffset != 0 && StructurePointer != 0 && DetourAddress != 0;
}

bool MovieHeapCorruptionPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	unsigned char zeroArray[] = { 0x00, 0x00, 0x00, 0x00 };
	if (!MemoryWriter::Write(StructurePointer, zeroArray, sizeof(zeroArray)))
		return false;

	MovieHeapCorruption::CurrentAllocatedMemoryAddress = AllocatedMemoryOffset;

	// Skip address at first position
	memcpy(MovieHeapCorruption::StructurePatches, StructureOffsets, sizeof(MovieHeapCorruption::StructurePatches));

	MovieHeapCorruption::JmpFromAddress = DetourAddress;
	MovieHeapCorruption::JmpBackAddress = MovieHeapCorruption::JmpFromAddress + MovieHeapCorruption::DetourSize;
	if (!Detour::Create(MovieHeapCorruption::JmpFromAddress, MovieHeapCorruption::DetourSize, (unsigned long)MovieHeapCorruption::Implementation))
		return false;

	return true;
}