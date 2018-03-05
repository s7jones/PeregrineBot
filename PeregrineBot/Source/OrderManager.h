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
	bool doesUnitHasOrder(BWAPI::Unit unit);
	bool doesSquadHaveOrder(Squad& squad);
	void attack(BWAPI::Unit attacker, BWAPI::Position p);
	void attack(BWAPI::Unit attacker, BWAPI::Unit unit);
	void attack(BWAPI::Unit attacker, EnemyUnitInfo enemy);
	void attack(Squad& squad, BWAPI::Unit unit);
	void attack(Squad& squad, EnemyUnitInfo enemy);
	void move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick = false);
	void move(BWAPI::Unit mover, const EnemyUnitInfo& unit, bool shiftClick = false);
	void move(Squad& squad, BWAPI::Position p, bool shiftClick = false);
	void move(Squad& squad, const EnemyUnitInfo& unit, bool shiftClick = false);
	void build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition);
	void stop(BWAPI::Unit stopper);
	void stop(Squad& squad);

private:
	void updateUnitsWaitingSinceLastOrder();
	void updateSquadsWaitingSinceLastOrder();

	std::map<BWAPI::Unit, int> unitsWaitingAfterOrder;
	std::map<unsigned int, int> squadsWaitingAfterOrder;
};
