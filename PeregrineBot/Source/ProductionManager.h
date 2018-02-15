#pragma once
#include "Common.h"

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
