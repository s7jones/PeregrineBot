#pragma once
#include "Common.h"

#include "UnitInfo.h"

class OrderManager {
	OrderManager();

public:
	static OrderManager& Instance();
	void Update();
	void UpdateUnitsWaitingSinceLastOrder();
	bool DoesUnitHasOrder(BWAPI::Unit unit);
	void Attack(BWAPI::Unit attacker, BWAPI::Position p);
	void Attack(BWAPI::Unit attacker, BWAPI::Unit u);
	void Attack(BWAPI::Unit attacker, EnemyUnitInfo u);
	void Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick = false);
	void Move(BWAPI::Unit mover, EnemyUnitInfo u, bool shiftClick = false);
	void Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void Stop(BWAPI::Unit stopper);

private:
	std::map<BWAPI::Unit, int> unitsToWaitAfterOrder;
};
