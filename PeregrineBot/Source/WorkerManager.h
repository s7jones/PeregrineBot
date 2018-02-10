#pragma once
#include "Common.h"

typedef std::vector<BWAPI::UnitType> Vector;

class WorkerManager {
	WorkerManager();

public:
	static WorkerManager& Instance();
	void DoAllWorkerTasks(BWAPI::Unit u);

	// To workaround Compiler Error C2797 in VS2013 had to do this
	// https://msdn.microsoft.com/en-us/library/dn793970.aspx
	// Seems to be Error C2664 in VS2017?
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

	int indx                = 0;
	bool pool               = false;
	bool poolready          = false;
	int lastChecked         = 0;
	int poolLastChecked     = 0;
	bool buildOrderComplete = false;
};
