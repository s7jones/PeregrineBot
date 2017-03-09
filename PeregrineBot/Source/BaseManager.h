#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <vector>

struct SCVmineral {
	BWAPI::Unit scv;
	BWAPI::Unit mineralPatch;

	bool useTrick;            // whether or not this SCV is using mineral trick
	BWAPI::Unit mineralTrick; // mineral field used for mineral trick
};

struct MineralPatch {
	bool trickPossible;       // whether or not this mineral field is available for mineral trick
	BWAPI::Unit mineralTrick; // mineral field used for mineral trick
	BWAPI::Unit mineralPatch;
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
	void addSCV(BWAPI::Unit scv);
	void addMineralField(BWAPI::Unit mineral);
	void CalculateTrickChoice(); // calcualte which mineral fields can use coop path trick
	void CalculateTrick();       // calcualte which mineral fields can use coop path trick
	void buildSCV();
	void onFrame();
	void toGas(int ToSend = 3);

	void MineralGatherChoice();
	void MineralGather();        // mineral gathering with techniques
	void MineralGatherLock();    // mineral gathering using only lock
	void MineralGatherNoTrick(); // mineral gathering without special techniques

	void LiftToNat();

	BWAPI::Unit getBuilder();   // remove a scv form minerals to build
	BWAPI::Unit getFullHPSCV(); // get full hp SCV for defence

	int SCVsaturation;
	BWAPI::Unit CommandCenter;
	BWAPI::TilePosition LandLocation; // in case the CC has to fly
	bool BaseReady;                   // base is ready when CC has landed
	bool buildComsat;
	int SCVsonGas;
	int InitialMinerals; // how many minerals this base initiall has

	std::vector<MineralPatch> Minerals;
	std::vector<BWAPI::Unit> Geysers;
	std::vector<BWAPI::Unit> Refinerys;

	std::vector<SCVmineral> MineralSCV;
	std::vector<BWAPI::Unit> GasSCV;
};