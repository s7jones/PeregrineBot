#pragma once
#include "BWAPI.h"
#include "Squad.h"
#include "UnitInfo.h"

class OrderManager {
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
	void attack(BWAPI::Unit attacker, BWAPI::Position p);
	void attack(BWAPI::Unit attacker, BWAPI::Unit u);
	void attack(BWAPI::Unit attacker, EnemyUnitInfo enemy);
	void attack(Squad& squad, BWAPI::Unit u);
	void attack(Squad& squad, EnemyUnitInfo enemy);
	void move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick = false);
	void move(BWAPI::Unit mover, EnemyUnitInfo u, bool shiftClick = false);
	void move(Squad& squad, BWAPI::Position p, bool shiftClick = false);
	void move(Squad& squad, EnemyUnitInfo u, bool shiftClick = false);
	void Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void stop(BWAPI::Unit stopper);
	void stop(Squad& squad);

private:
	std::map<BWAPI::Unit, int> unitsToWaitAfterOrder;
};
