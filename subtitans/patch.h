#pragma once

class Patch {
public:
	Patch() {}
	virtual ~Patch() {}

	virtual bool Validate() = 0;
	virtual bool Apply() = 0;
	virtual const wchar_t* ErrorMessage() = 0;
};
