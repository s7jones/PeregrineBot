#pragma once
#include "BWAPI.h"

class BuildingManager {
private:
	BuildingManager() = default;

public:
	static BuildingManager& Instance()
	{
		static BuildingManager instance;
		return instance;
	}
	bool isAnythingToBuild(BWAPI::Unit builder);
	bool tryToBuild(BWAPI::Unit builder, BWAPI::UnitType);

private:
	int lastChecked     = 0;
	int poolLastChecked = 0;
};
