#pragma once
#include "Common.h"
#include "UnitInfo.h"

class ArmyManager {
private:
	ArmyManager() {}

public:
	static ArmyManager& Instance()
	{
		static ArmyManager instance;
		return instance;
	}
	void ZerglingAttack(BWAPI::Unit u);
	std::set<EnemyUnitInfo> GetZerglingAccessibleBuildings(BWAPI::Unit u);
	void ZerglingAttackKnownBuildings(BWAPI::Unit u);
	void ZerglingScoutingBeforeBaseFound(BWAPI::Unit u);
	void ZerglingScoutSpreadOut(BWAPI::Unit u);

private:
	void incrementScoutLocationZerglingIndex();

	std::deque<BWAPI::Position> scoutLocationsZergling;
	std::deque<BWAPI::Position>::iterator scoutLocationIndex;
};
