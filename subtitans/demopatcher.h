#include "patcher.h"

class DemoPatcher : public Patcher {
public:
	DemoPatcher();
	virtual ~DemoPatcher();

protected:
	void Configure() override;
};