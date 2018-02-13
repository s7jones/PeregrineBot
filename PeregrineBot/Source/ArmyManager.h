#pragma once
#include "Common.h"
#include "UnitInfo.h"

class ArmyManager {
	ArmyManager();

public:
	static ArmyManager& Instance();
	void ZerglingAttack(BWAPI::Unit u);
	std::set<EnemyUnitInfo> GetZerglingAccessibleBuildings(BWAPI::Unit u);
	void ZerglingAttackKnownBuildings(BWAPI::Unit u);
	void ZerglingScoutingBeforeBaseFound(BWAPI::Unit u);
	void ZerglingScoutSpreadOut(BWAPI::Unit u);

private:
	std::deque<BWAPI::Position> scoutLocationsZergling;
};
