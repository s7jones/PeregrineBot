#pragma once
#include "Common.h"
#include "UnitInfo.h"

class ArmyManager {
	ArmyManager();

public:
	static ArmyManager& Instance();
	void ZerglingAttack(const BWAPI::Unit u);
	std::set<UnitInfo> GetZerglingAccessibleBuildings(const BWAPI::Unit u);
	void ZerglingAttackKnownBuildings(const BWAPI::Unit u);
	void ZerglingScoutingBeforeBaseFound(const BWAPI::Unit u);
	void ZerglingScoutSpreadOut(const BWAPI::Unit u);

private:
	std::deque<BWAPI::Position> scoutLocationsZergling;
};
