#pragma once
#include "BWAPI.h"

class ProductionManager {
private:
	ProductionManager() = default;

public:
	static ProductionManager& Instance()
	{
		static ProductionManager instance;
		return instance;
	}
};
