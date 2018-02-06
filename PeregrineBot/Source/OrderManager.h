#pragma once
#include "Common.h"

class OrderManager {
	OrderManager();

public:
	static OrderManager& Instance();
	void Update();
	void UpdateUnitsWaitingSinceLastOrder();
	bool DoesUnitHasOrder(BWAPI::Unit unit);
	void Attack(BWAPI::Unit attacker, BWAPI::PositionOrUnit u);
	void Move(BWAPI::Unit mover, BWAPI::Position p);
	void Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick);
	void Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void Stop(BWAPI::Unit stopper);

private:
	std::map<BWAPI::Unit, int> unitsToWaitAfterOrder;
};