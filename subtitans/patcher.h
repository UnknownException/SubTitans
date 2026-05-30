#pragma once
#include "patch.h"

class Patcher {

public:
	Patcher();
	virtual ~Patcher();

	// Optional
	virtual bool Initialize() { return true; }
	// Set addresses
	virtual void Configure() = 0;
	bool Apply();

protected:
	std::vector<Patch*> _patches;

	static bool RestoreVersion(uint32_t expectedVersionValue);
};
