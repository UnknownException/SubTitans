#pragma once
#include "patcher.h"

class TechDemoPatcher : public Patcher {
public:
	TechDemoPatcher();
	virtual ~TechDemoPatcher();

protected:
	virtual bool Initialize() override;
	virtual void Configure() override;
};