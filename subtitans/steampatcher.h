#pragma once
#include "patcher.h"

class SteamPatcher : public Patcher {
public:
	SteamPatcher();
	virtual ~SteamPatcher();

protected:
	unsigned long _directDrawId;

	bool Initialize() override;
	virtual void Configure() override;
};