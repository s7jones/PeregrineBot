#include "BuildOrderManager.h"

#include "DebugMessenger.h"

using namespace BWAPI;
using namespace Filter;

BuildOrderManager::BuildOrderManager()
{
	for (auto ut : bo) {
		auto result = boMap.insert({ ut, 1 });
		if (!result.second) {
			result.first->second++;
		}
	}
}

BuildOrderManager& BuildOrderManager::Instance()
{
	static BuildOrderManager instance;
	return instance;
}

BWAPI::UnitType BuildOrderManager::rebuildBuilding()
{
	for (auto ut : boMap) {
		if (!ut.first.isBuilding()) continue;
		auto utBoPassedCount = std::count(bo.begin(), boIndex, ut.first);

		int count = 0;
		for (auto unit : Broodwar->self()->getUnits()) {
			if ((!IsBuilding)(unit)) continue;
			if (unit->getType() == ut.first) count++;
			if (count >= utBoPassedCount) break;
		}
		if (count < utBoPassedCount) {
			// shouldn't get here without needing to rebuild
			DebugMessenger::Instance() << "rebuilding " << ut.first << std::endl;
			return ut.first;
		}
	}

	return BWAPI::UnitTypes::Unknown;
}

void BuildOrderManager::incrementBuildOrder()
{
	boIndex++;
	if (boIndex == bo.end()) {
		buildOrderComplete = true;
	}
}
