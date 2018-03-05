#include "OrderManager.h"

#include "UnitInfo.h"

using namespace BWAPI;

void OrderManager::update()
{
	updateUnitsWaitingSinceLastOrder();
	updateSquadsWaitingSinceLastOrder();
}

void OrderManager::updateUnitsWaitingSinceLastOrder()
{
	auto it = unitsWaitingAfterOrder.begin();
	while (it != unitsWaitingAfterOrder.end()) {
		++(it->second); // increment counter
		if (it->second >= 8) {
			it = unitsWaitingAfterOrder.erase(it);
		} else {
			++it;
		}
	}
}

void OrderManager::updateSquadsWaitingSinceLastOrder()
{
	auto it = squadsWaitingAfterOrder.begin();
	while (it != squadsWaitingAfterOrder.end()) {
		++(it->second); // increment counter
		if (it->second >= 8) {
			it = squadsWaitingAfterOrder.erase(it);
		} else {
			++it;
		}
	}
}

bool OrderManager::doesUnitHasOrder(BWAPI::Unit unit)
{
	auto it = unitsWaitingAfterOrder.find(unit);
	return (it != unitsWaitingAfterOrder.end());
}

bool OrderManager::doesSquadHaveOrder(Squad& squad)
{
	auto it = squadsWaitingAfterOrder.find(squad.id);
	return (it != squadsWaitingAfterOrder.end());
}

void OrderManager::attack(BWAPI::Unit attacker, BWAPI::Position p)
{
	unitsWaitingAfterOrder.insert({ attacker, 0 });
	attacker->attack(p);
}

void OrderManager::attack(BWAPI::Unit attacker, BWAPI::Unit unit)
{
	unitsWaitingAfterOrder.insert({ attacker, 0 });
	attacker->attack(unit);
}

void OrderManager::attack(BWAPI::Unit attacker, EnemyUnitInfo enemy)
{
	if (Broodwar->isVisible((TilePosition)enemy.getPosition())) {
		if (enemy.unit) {
			attack(attacker, enemy.unit);
		} else {
			attack(attacker, enemy.getPosition());
		}
	} else {
		move(attacker, enemy.getPosition());
	}
}

void OrderManager::attack(Squad& squad, BWAPI::Unit unit)
{
	squadsWaitingAfterOrder.insert({ squad.id, 0 });
	for (const auto attacker : squad) {
		attack(attacker, unit);
	}

	SquadCommand command = { SquadCommandTypes::ATTACK_UNIT, unit };
	squad.setLastCommand(command);
}

void OrderManager::attack(Squad& squad, EnemyUnitInfo enemy)
{
	squadsWaitingAfterOrder.insert({ squad.id, 0 });
	for (const auto attacker : squad) {
		attack(attacker, enemy);
	}

	SquadCommandTypes type;

	if (Broodwar->isVisible((TilePosition)enemy.getPosition())) {
		if (enemy.unit) {
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
	unitsWaitingAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::move(BWAPI::Unit mover, const EnemyUnitInfo& unit, bool shiftClick)
{
	move(mover, unit.getPosition(), shiftClick);
}

void OrderManager::move(Squad& squad, BWAPI::Position p, bool shiftClick)
{
	squadsWaitingAfterOrder.insert({ squad.id, 0 });
	for (const auto mover : squad) {
		move(mover, p, shiftClick);
	}

	SquadCommand command = { SquadCommandTypes::MOVE, p };
	squad.setLastCommand(command);
}

void OrderManager::move(Squad& squad, const EnemyUnitInfo& unit, bool shiftClick)
{
	squadsWaitingAfterOrder.insert({ squad.id, 0 });
	for (const auto mover : squad) {
		move(mover, unit, shiftClick);
	}

	SquadCommand command = { SquadCommandTypes::MOVE, unit };
	squad.setLastCommand(command);
}

void OrderManager::build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition)
{
	unitsWaitingAfterOrder.insert({ builder, 0 });
	builder->build(buildingType, buildPosition);
}

void OrderManager::stop(BWAPI::Unit stopper)
{
	unitsWaitingAfterOrder.insert({ stopper, 0 });
	stopper->stop();
}

void OrderManager::stop(Squad& squad)
{
	squadsWaitingAfterOrder.insert({ squad.id, 0 });
	for (const auto stopper : squad) {
		stop(stopper);
	}
}
