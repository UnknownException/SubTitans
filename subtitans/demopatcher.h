#pragma once
#include "patcher.h"

class DemoPatcher : public Patcher {
	uint32_t _registerGameVersion = 0;

public:
	DemoPatcher();
	virtual ~DemoPatcher();

protected:
	virtual bool Initialize() override;
	virtual void Configure() override;
};