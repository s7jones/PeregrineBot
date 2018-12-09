#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"
#include <deque>
#include <set>

class ArmyManager
{
private:
	ArmyManager() = default;

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
	void GenerateZergScoutPositions(BWAPI::Unit u);
	void incrementScoutLocationZerglingIndex();

	std::deque<BWAPI::Position> m_scoutLocationsZergling;
	std::deque<BWAPI::Position>::iterator m_scoutLocationIndex;
};
