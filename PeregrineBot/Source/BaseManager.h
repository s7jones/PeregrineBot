#pragma once
#include "Common.h"

struct WorkerAndMineral {
	BWAPI::Unit worker;
	BWAPI::Unit mineralPatch;

	bool useTrick;            // whether or not this Worker is using mineral trick
	BWAPI::Unit mineralTrick; // mineral field used for mineral trick

	bool operator<(const WorkerAndMineral& rhs) const
	{
		return worker->getID() < rhs.worker->getID();
	}
};

struct MineralPatch {
	bool trickPossible;       // whether or not this mineral field is available for mineral trick
	BWAPI::Unit mineralTrick; // mineral field used for mineral trick
	BWAPI::Unit mineralPatch;

	bool operator<(const MineralPatch& rhs) const
	{
		return mineralPatch->getID() < rhs.mineralPatch->getID();
	}
};

/*
enum FloatState{
Liftoff,

};
*/

class BaseManager {
public:
	BaseManager(BWAPI::Unit CC, BWAPI::TilePosition land);
	int cycleMineral;
	void addWorker(BWAPI::Unit worker);
	void addMineralField(BWAPI::Unit mineral);
	//void CalculateTrickChoice(); // calculate which mineral fields can use coop path trick
	//void CalculateTrick();       // calculate which mineral fields can use coop path trick
	void buildWorker();
	void onFrame();
	void toGas(int ToSend = 3);

	void MineralGatherChoice();
	//void MineralGather();        // mineral gathering with techniques
	void MineralGatherLock(); // mineral gathering using only lock
	//void MineralGatherNoTrick(); // mineral gathering without special techniques

	//void LiftToNat();

	BWAPI::Unit getBuilder();   // remove a worker form minerals to build
	BWAPI::Unit getFullHPWorker(); // get full hp Worker for defence

	int WorkerSaturation;
	BWAPI::Unit CommandCenter;
	BWAPI::TilePosition LandLocation; // in case the CC has to fly
	bool BaseReady;                   // base is ready when CC has landed
	bool buildComsat;
	int WorkersonGas;
	int InitialMinerals; // how many minerals this base initiall has

	std::set<MineralPatch> Minerals;
	std::set<BWAPI::Unit> Geysers;
	std::set<BWAPI::Unit> Refinerys;

	std::set<WorkerAndMineral> WorkerAndMineralList;
	//std::set<BWAPI::Unit> GasWorker;
};