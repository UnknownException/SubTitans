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
	unsigned long StructurePatches[27];
	unsigned long DetourAddress;
};