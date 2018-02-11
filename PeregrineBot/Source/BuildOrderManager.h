#pragma once
#include "Common.h"

class BuildOrderManager {
public:
	static BuildOrderManager& Instance();
	void incrementBuildOrder();

	// To workaround Compiler Error C2797 in VS2013 had to do this
	// https://msdn.microsoft.com/en-us/library/dn793970.aspx
	// Seems to be Error C2664 in VS2017?
	typedef std::vector<BWAPI::UnitType> Vector;
	const Vector bo = Vector{ BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Spawning_Pool),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Overlord),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
		BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling) };

	Vector::const_iterator boIndex = bo.begin();
	bool pool = false;
	bool poolready = false;
	bool buildOrderComplete = false;

private:
	BuildOrderManager();
};
