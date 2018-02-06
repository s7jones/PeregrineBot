#pragma once
#include "Common.h"

class BaseManager {
	BaseManager();

public:
	static BaseManager& Instance();
	void ManageBases(BWAPI::Unit u);
	std::set<BWAPI::Unit> hatcheries;
	std::set<BWAPI::Unit> workers;

private:
	std::set<BWAPI::Unit> workersTraining;
};
