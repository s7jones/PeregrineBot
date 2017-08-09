#include "OrderManager.h"

using namespace BWAPI;

std::map<Unit, int> unitsToWaitAfterOrder;

OrderManager::OrderManager()
{
}

OrderManager& OrderManager::Instance()
{
	static OrderManager instance;
	return instance;
}

void OrderManager::Attack(BWAPI::Unit attacker, BWAPI::PositionOrUnit u)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(u);
}

void OrderManager::Move(BWAPI::Unit mover, BWAPI::Position p)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, false);
}

void OrderManager::Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::Build(BWAPI::Unit builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition)
{
	unitsToWaitAfterOrder.insert({ builder, 0 });
	builder->build(buildingType, buildPosition);
}

void OrderManager::Gather(BWAPI::Unit worker, BWAPI::Unit mineral)
{
	unitsToWaitAfterOrder.insert({ worker, 0 });
	worker->gather(mineral);
}

void OrderManager::Stop(BWAPI::Unit stopper)
{
	unitsToWaitAfterOrder.insert({ stopper, 0 });
	stopper->stop();
}

bool OrderManager::UpdateUnitsWaitingSinceLastOrder(Unit u)
{
	bool unitNeedsToWait = false;
	// If unit has been given an order in the last 8 frames
	auto unitIterator = unitsToWaitAfterOrder.find(u);
	if (unitIterator != unitsToWaitAfterOrder.end()) {
		++(unitIterator->second);
		if (unitIterator->second >= 8) {
			unitsToWaitAfterOrder.erase(u);
		} else {
			unitNeedsToWait = true;
		}
	}
	return unitNeedsToWait;
}