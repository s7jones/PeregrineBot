#include "OrderManager.h"

#include "UnitInfo.h"

using namespace BWAPI;

void OrderManager::update()
{
	UpdateUnitsWaitingSinceLastOrder();
}

void OrderManager::UpdateUnitsWaitingSinceLastOrder()
{
	auto it = unitsToWaitAfterOrder.begin();
	while (it != unitsToWaitAfterOrder.end()) {
		++(it->second); // increment counter
		if (it->second >= 8) {
			it = unitsToWaitAfterOrder.erase(it);
		} else {
			++it;
		}
	}
}

bool OrderManager::DoesUnitHasOrder(BWAPI::Unit unit)
{
	auto it = unitsToWaitAfterOrder.find(unit);
	return (it != unitsToWaitAfterOrder.end());
}

void OrderManager::attack(BWAPI::Unit attacker, BWAPI::Position p)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(p);
}

void OrderManager::attack(BWAPI::Unit attacker, BWAPI::Unit u)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(u);
}

void OrderManager::attack(BWAPI::Unit attacker, EnemyUnitInfo enemy)
{
	if (Broodwar->isVisible((TilePosition)enemy.getPosition())) {
		if (enemy.u) {
			attack(attacker, enemy.u);
		} else {
			attack(attacker, enemy.getPosition());
		}
	} else {
		move(attacker, enemy.getPosition());
	}
}

void OrderManager::attack(Squad& squad, BWAPI::Unit u)
{
	for (const auto attacker : squad) {
		attack(attacker, u);
	}

	SquadCommand command = { SquadCommandTypes::ATTACK_UNIT, u };
	squad.setLastCommand(command);
}

void OrderManager::attack(Squad& squad, EnemyUnitInfo enemy)
{
	for (const auto attacker : squad) {
		attack(attacker, enemy);
	}

	SquadCommandTypes type;

	if (Broodwar->isVisible((TilePosition)enemy.getPosition())) {
		if (enemy.u) {
			type = SquadCommandTypes::ATTACK_UNIT;
		} else {
			type = SquadCommandTypes::ATTACK_MOVE;
		}
	} else {
		type = SquadCommandTypes::MOVE;
	}

	SquadCommand command = { type, enemy };
	squad.setLastCommand(command);
}

void OrderManager::move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::move(BWAPI::Unit mover, EnemyUnitInfo u, bool shiftClick)
{
	move(mover, u.getPosition(), shiftClick);
}

void OrderManager::move(Squad& squad, BWAPI::Position p, bool shiftClick)
{
	for (const auto mover : squad) {
		move(mover, p, shiftClick);
	}

	SquadCommand command = { SquadCommandTypes::MOVE, p };
	squad.setLastCommand(command);
}

void OrderManager::move(Squad& squad, EnemyUnitInfo u, bool shiftClick)
{
	for (const auto mover : squad) {
		move(mover, u, shiftClick);
	}

	SquadCommand command = { SquadCommandTypes::MOVE, u };
	squad.setLastCommand(command);
}

void OrderManager::Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition)
{
	unitsToWaitAfterOrder.insert({ builder, 0 });
	builder->build(buildingType, buildPosition);
}

void OrderManager::stop(BWAPI::Unit stopper)
{
	unitsToWaitAfterOrder.insert({ stopper, 0 });
	stopper->stop();
}

void OrderManager::stop(Squad& squad)
{
	for (const auto stopper : squad) {
		stop(stopper);
	}
}
