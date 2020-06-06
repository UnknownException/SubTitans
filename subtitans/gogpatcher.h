#include "patcher.h"

class GOGPatcher : public Patcher {
public:
	GOGPatcher();
	virtual ~GOGPatcher();

protected:
	void Configure() override;
};