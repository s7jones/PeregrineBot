#include "OrderManager.h"

using namespace BWAPI;

OrderManager::OrderManager()
{
}

OrderManager& OrderManager::Instance()
{
	static OrderManager instance;
	return instance;
}

void OrderManager::Update()
{
	UpdateUnitsWaitingSinceLastOrder();
}

void OrderManager::UpdateUnitsWaitingSinceLastOrder()
{
	for (auto unitIterator = unitsToWaitAfterOrder.begin(); unitIterator != unitsToWaitAfterOrder.end(); unitIterator++) {
		++(unitIterator->second); // increment counter
		if (unitIterator->second >= 8) {
			unitsToWaitAfterOrder.erase(unitIterator->first);
		}
	}
}

bool OrderManager::DoesUnitHasOrder(BWAPI::Unit unit)
{
	auto it = unitsToWaitAfterOrder.find(unit);
	return (it != unitsToWaitAfterOrder.end());
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

void OrderManager::Stop(BWAPI::Unit stopper)
{
	unitsToWaitAfterOrder.insert({ stopper, 0 });
	stopper->stop();
}
