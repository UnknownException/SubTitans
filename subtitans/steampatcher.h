#pragma once
#include <string>
#include "patcher.h"

class SteamPatcher : public Patcher {
public:
	SteamPatcher();
	virtual ~SteamPatcher();

protected:
	unsigned long _directDrawId;
	std::wstring _windows7CompatibilityKeyPostfix;

	bool Initialize() override;
	virtual void Configure() override;
};