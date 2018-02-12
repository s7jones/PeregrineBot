#pragma once
#include "Common.h"

#include "UnitInfo.h"

class OrderManager {
	OrderManager();

public:
	static OrderManager& Instance();
	void Update();
	void UpdateUnitsWaitingSinceLastOrder();
	bool DoesUnitHasOrder(const BWAPI::Unit& unit);
	void Attack(const BWAPI::Unit& attacker, BWAPI::Position p);
	void Attack(const BWAPI::Unit& attacker, const BWAPI::Unit& u);
	void Attack(const BWAPI::Unit& attacker, UnitInfo u);
	void Move(const BWAPI::Unit& mover, BWAPI::Position p, bool shiftClick = false);
	void Move(const BWAPI::Unit& mover, UnitInfo u, bool shiftClick = false);
	void Build(const BWAPI::Unit& builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void Stop(const BWAPI::Unit& stopper);

private:
	std::map<BWAPI::Unit, int> unitsToWaitAfterOrder;
};
