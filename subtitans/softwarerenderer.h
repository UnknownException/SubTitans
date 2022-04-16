#pragma once
#include "irenderer.h"

class SoftwareRenderer : public IRenderer {
public:
	SoftwareRenderer();
	virtual ~SoftwareRenderer();

	virtual bool Create() override;
	virtual void OnCreatePrimarySurface(Surface* primarySurface) override;
	virtual void OnDestroyPrimarySurface() override;

protected:
	virtual void Run() override;
};