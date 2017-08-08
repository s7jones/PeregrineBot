#pragma once
#include "Common.h"
#include "WorkerManager.h"

class BaseManager {
	BaseManager();

public:
	static BaseManager& Instance();
	void ManageBases(BWAPI::Unit u);
	std::set<BWAPI::Unit> hatcheries;
	std::set<BWAPI::Unit> workerList;
};