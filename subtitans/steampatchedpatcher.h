#pragma once
#include "steampatcher.h"

class SteamPatchedPatcher : public SteamPatcher {
	uint32_t _baseOffset;

public:
	SteamPatchedPatcher(uint32_t baseOffset);
	virtual ~SteamPatchedPatcher();

protected:
	virtual void Configure() override;
};