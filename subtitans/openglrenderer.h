#pragma once
#include "irenderer.h"

class OpenGLRenderer : public IRenderer {
public:
	OpenGLRenderer();
	virtual ~OpenGLRenderer();

	virtual bool Create() override;
	virtual void OnCreatePrimarySurface(Surface* primarySurface) override;
	virtual void OnDestroyPrimarySurface() override;

protected:
	virtual void Run() override;
};