#pragma once
#include "patcher.h"

class GOGPatcher : public Patcher {
	uint32_t _baseOffset;

public:
	GOGPatcher(uint32_t baseOffset);
	virtual ~GOGPatcher();

protected:
	void Configure() override;
};