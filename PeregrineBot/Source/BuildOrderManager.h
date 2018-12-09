#pragma once
#include "BWAPI.h"

class BuildOrderManager
{
private:
	BuildOrderManager() = default;

public:
	static BuildOrderManager& Instance()
	{
		static BuildOrderManager instance;
		return instance;
	}
	void setup();
	BWAPI::UnitType rebuildBuilding();
	void incrementBuildOrder();

	// To workaround Compiler Error C2797 in VS2013 had to do this
	// https://msdn.microsoft.com/en-us/library/dn793970.aspx
	// Seems to be Error C2664 in VS2017?
	using Vector    = std::vector<BWAPI::UnitType>;
	const Vector bo = {
		BWAPI::UnitTypes::Zerg_Drone,
		BWAPI::UnitTypes::Zerg_Spawning_Pool,
		BWAPI::UnitTypes::Zerg_Drone,
		BWAPI::UnitTypes::Zerg_Drone,
		BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Overlord,
		BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Zergling
	};

	Vector::const_iterator boIndex = bo.begin();
	bool pool                      = false;
	bool poolready                 = false;
	bool buildOrderComplete        = false;

private:
	std::map<BWAPI::UnitType, int> boMap;
};
