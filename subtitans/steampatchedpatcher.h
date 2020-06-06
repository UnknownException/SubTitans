#pragma once
#include "steampatcher.h"

class SteamPatchedPatcher : public SteamPatcher {
public:
	SteamPatchedPatcher();
	virtual ~SteamPatchedPatcher();

protected:
	virtual void Configure() override;
};