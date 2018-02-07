#pragma once
#include "Common.h"

class BaseManager {
	BaseManager();

public:
	static BaseManager& Instance();
	void onUnitShow(BWAPI::Unit unit);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);

	void ManageBases(BWAPI::Unit u);
	std::set<BWAPI::Unit> hatcheries;
	std::set<BWAPI::Unit> workers;


private:
	std::set<BWAPI::Unit> workersTraining;
};
