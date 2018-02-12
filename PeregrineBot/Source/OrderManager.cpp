#include "OrderManager.h"

#include "UnitInfo.h"

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

bool OrderManager::DoesUnitHasOrder(const BWAPI::Unit& unit)
{
	auto it = unitsToWaitAfterOrder.find(unit);
	return (it != unitsToWaitAfterOrder.end());
}

void OrderManager::Attack(const BWAPI::Unit& attacker, BWAPI::Position p)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(p);
}

void OrderManager::Attack(const BWAPI::Unit& attacker, const BWAPI::Unit& u)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(u);
}

void OrderManager::Attack(const BWAPI::Unit& attacker, UnitInfo u)
{
	Attack(attacker, u.getPosition());
}

void OrderManager::Move(const BWAPI::Unit& mover, BWAPI::Position p, bool shiftClick)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::Move(const BWAPI::Unit& mover, UnitInfo u, bool shiftClick)
{
	Move(mover, u.getPosition(), shiftClick);
}

void OrderManager::Build(const BWAPI::Unit& builder, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPosition)
{
	unitsToWaitAfterOrder.insert({ builder, 0 });
	builder->build(buildingType, buildPosition);
}

void OrderManager::Stop(const BWAPI::Unit& stopper)
{
	unitsToWaitAfterOrder.insert({ stopper, 0 });
	stopper->stop();
}
