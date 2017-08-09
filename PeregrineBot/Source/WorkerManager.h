#pragma once
#include "Common.h"
#include "BaseManager.h"
#include "OrderManager.h"
#include "InformationManager.h"

typedef std::vector<BWAPI::UnitType> Vector;

//struct WorkersAssignedToMineral {
//	Unit mineral;
//	Unitset workers;
//};

//struct sortByLeastWorkersThenClosest {
//	bool operator()(const BWAPI::Unit& lhs, BWAPI::Unit& rhs)
//	{
//		if (lhs.)
//	}
//};

class WorkerManager {
	WorkerManager();

public:
	static WorkerManager& Instance();
	void DoAllWorkerTasks(BWAPI::Unit u);
	void DoAllWorkerTasks2(BWAPI::Unit u);
	void WorkerLockGathering(BWAPI::Unit u);
	BWAPI::Unit GetMineralPatch(BWAPI::Unit u);
	void UnassignWorkerFromMineral(BWAPI::Unit u);
	BWAPI::Unit MineralAssociatedWithWorker(BWAPI::Unit u);
	void AddMinerals(BWAPI::Unitset minerals);

	//const std::vector<BWAPI::UnitType> bo = ( BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Spawning_Pool),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Drone),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Overlord),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling),
	//	                                      BWAPI::UnitType(BWAPI::UnitTypes::Zerg_Zergling) );

	// To workaround Compiler Error C2797 in VS2013 had to do this
	// https://msdn.microsoft.com/en-us/library/dn793970.aspx
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

	int indx            = 0;
	bool pool           = false;
	bool poolready      = false;
	int lastChecked     = 0;
	int poolLastChecked = 0;
	//std::vector<WorkersAssignedToMineral> assignedWorkersAllMinerals;
	std::map<BWAPI::Unit, BWAPI::Unitset> assignedWorkersAllMinerals;
	BWAPI::Unitset builderWorkers;
	BWAPI::Unitset mineralWorkers;
};