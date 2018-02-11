#include "BuildOrderManager.h"

using namespace BWAPI;

BuildOrderManager::BuildOrderManager()
{
}

BuildOrderManager& BuildOrderManager::Instance()
{
	static BuildOrderManager instance;
	return instance;
}

void BuildOrderManager::incrementBuildOrder()
{
	boIndex++;
	if (boIndex == bo.end()) {
		buildOrderComplete = true;
	}
}
