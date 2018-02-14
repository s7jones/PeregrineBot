#pragma once
#include "Common.h"

class BuildingManager {
public:
	static BuildingManager& Instance();
	bool isAnythingToBuild(BWAPI::Unit builder);
	bool tryToBuild(BWAPI::Unit builder, BWAPI::UnitType);

private:
	BuildingManager();
	int lastChecked     = 0;
	int poolLastChecked = 0;
};
