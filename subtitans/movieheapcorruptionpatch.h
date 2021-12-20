#pragma once
#include "patch.h"

class MovieHeapCorruptionPatch : public Patch {
public:
	MovieHeapCorruptionPatch();
	virtual ~MovieHeapCorruptionPatch();

	bool Validate() override;
	bool Apply() override;
	const wchar_t* ErrorMessage() override;

	unsigned long AllocatedMemoryOffset;
	unsigned long StructurePointer;
	unsigned long StructureOffsets[26];
	unsigned long DetourAddress;
};