#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"

class OrderManager
{
private:
	OrderManager() = default;

public:
	static OrderManager& Instance()
	{
		static OrderManager instance;
		return instance;
	}
	void update();
	void UpdateUnitsWaitingSinceLastOrder();
	bool DoesUnitHasOrder(BWAPI::Unit unit);
	void Attack(BWAPI::Unit attacker, BWAPI::Position p);
	void Attack(BWAPI::Unit attacker, BWAPI::Unit u);
	void Attack(BWAPI::Unit attacker, const EnemyUnitInfo& enemy);
	void Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick = false);
	void Move(BWAPI::Unit mover, const EnemyUnitInfo& u, bool shiftClick = false);
	void Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void Stop(BWAPI::Unit stopper);

private:
	std::map<BWAPI::Unit, int> unitsToWaitAfterOrder;
};
