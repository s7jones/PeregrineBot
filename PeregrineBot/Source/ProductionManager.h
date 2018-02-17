#pragma once
#include "BWAPI.h"

class ProductionManager {
private:
	ProductionManager() {}

public:
	static ProductionManager& ProductionManager::Instance()
	{
		static ProductionManager instance;
		return instance;
	}
};
