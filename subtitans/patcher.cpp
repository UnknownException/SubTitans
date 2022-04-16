#include "subtitans.h"
#include "patcher.h"

Patcher::Patcher()
{
}

Patcher::~Patcher()
{
	for (auto it = _patches.begin(); it != _patches.end(); ++it)
	{
		delete *it;
	}
}

bool Patcher::Apply()
{
	for (auto it = _patches.begin(); it != _patches.end(); ++it)
	{
		if (!(*it)->Validate())
		{
			MessageBox(NULL, (*it)->ErrorMessage(), L"Patch validation error", MB_ICONERROR);
			return false;
		}

		if (!(*it)->Apply())
		{
			MessageBox(NULL, (*it)->ErrorMessage(), L"Patching error", MB_ICONERROR);
			return false;
		}
	}

	return true;
}