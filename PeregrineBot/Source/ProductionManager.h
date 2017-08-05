#pragma once
#include "Common.h"

struct BuildQueue {
	BWAPI::Unit Worker;
	BWAPI::UnitType type;
	BWAPI::TilePosition buildLocation;
	BWAPI::Unit building;
	bool Started;
	int FrameStarted;
};

struct IndividualBuildOrder {
	BWAPI::UnitType type;
	int supply;
};

class ProductionManager {
public:
	int reservedMinerals;
	int reservedGas;

	//std::vector<BWAPI::UnitType> BuildingOrder;
	std::vector<IndividualBuildOrder> BuildingAtSupplyOrder;
	std::vector<BuildQueue> BuildingsQueue;
	std::vector<BWAPI::TechType> Research;
	std::vector<BWAPI::UnitType> BuildingsQueueType; //record the type of buildings in the queue

	ProductionManager();
	void onFrame();
	void onFrameMain();
	void onFrameMacro();

	void returnBuilderWorker(BWAPI::Unit Worker);
	BWAPI::Unit getWorkerBuilder(BWAPI::UnitType type, BWAPI::TilePosition position);
	void checkWorkerbuild();
	void onUnitComplete(BWAPI::Unit unit);
	int TotalTypeInQueue(BWAPI::UnitType type);
	int TotalUnitInProduction(BWAPI::UnitType type);

	void addToQueue(BWAPI::UnitType type);
	void addToQueueTile(BWAPI::UnitType type, BWAPI::TilePosition tile);
	void addToQueueClose(BWAPI::UnitType type, BWAPI::TilePosition tile, bool inBase = true);
	void addToQueueClose(BWAPI::UnitType type, BWAPI::TilePosition tile, BWTA::Region* region);
	void addToQueueBegin(BWAPI::UnitType type, BWAPI::TilePosition tileClose = BWAPI::TilePositions::Unknown);
	void removeFromQueue(BWAPI::UnitType type);

	void addToBuildOrder(BWAPI::UnitType type, int supply = 0);
	void addToBuildOrderBegin(BWAPI::UnitType type, int supply = 0);

	void onUnitDestroy(BWAPI::Unit unit);

	int getAvailableMinerals();
	int getAvailableGas();
};