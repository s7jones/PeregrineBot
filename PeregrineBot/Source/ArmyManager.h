#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"
#include <deque>
#include <set>

class ArmyManager {
private:
	ArmyManager() = default;

public:
	static ArmyManager& Instance()
	{
		static ArmyManager instance;
		return instance;
	}
	void SquadsRegroup();
	void SquadsAttack();
	void ZerglingAttack(BWAPI::Unit u);
	std::set<EnemyUnitInfo> GetZerglingAccessibleBuildings(BWAPI::Unit u);
	void ZerglingAttackKnownBuildings(BWAPI::Unit u);
	void ZerglingScoutingBeforeBaseFound(BWAPI::Unit u);
	void ZerglingScoutSpreadOut(BWAPI::Unit u);

private:
	void incrementScoutLocationZerglingIndex();

	using Squad = BWAPI::Unitset;
	std::set<Squad> squads;
	const int SQUAD_RADIUS;

	std::deque<BWAPI::Position> scoutLocationsZergling;
	std::deque<BWAPI::Position>::iterator scoutLocationIndex;
};
