#include "ProductionManager.h"

using namespace BWAPI;

ProductionManager::ProductionManager()
{
}

ProductionManager& ProductionManager::Instance()
{
	static ProductionManager instance;
	return instance;
}