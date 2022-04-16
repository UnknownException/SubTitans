#include "subtitans.h"
#include "disabledrawstackingpatch.h"

// Deprecated in favor of the OpenGL/Software rendering implementation (See DDrawReplacementPatch)

DisableDrawStackingPatch::DisableDrawStackingPatch()
{
	GetLogger()->Informational("Constructing %s\n", __func__);

	Address = 0;
}

DisableDrawStackingPatch::~DisableDrawStackingPatch()
{
	GetLogger()->Informational("Destructing %s\n", __func__);
}

bool DisableDrawStackingPatch::Validate()
{
	return Address != 0;
}

bool DisableDrawStackingPatch::Apply()
{
	GetLogger()->Informational("%s\n", __FUNCTION__);

	// This old optimization tanks the FPS
	// Disabling it by overwriting the limit (100) with -1 (Jump Greater will ALWAYS execute)
	unsigned char minusOne[] = { 0xFF };
	if (!MemoryWriter::Write(Address, minusOne, 1))
		return false;

	return true;
}
