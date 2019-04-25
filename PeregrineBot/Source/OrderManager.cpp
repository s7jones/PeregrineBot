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
	while (it != unitsToWaitAfterOrder.end())
	{
		++(it->second); // increment counter
		if (it->second >= 8)
		{
			it = unitsToWaitAfterOrder.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool OrderManager::DoesUnitHasOrder(BWAPI::Unit unit)
{
	auto it = unitsToWaitAfterOrder.find(unit);
	return (it != unitsToWaitAfterOrder.end());
}

void OrderManager::Attack(BWAPI::Unit attacker, BWAPI::Position p)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(p);
}

void OrderManager::Attack(BWAPI::Unit attacker, BWAPI::Unit u)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(u);
}

void OrderManager::Attack(BWAPI::Unit attacker, const EnemyUnitInfo& enemy)
{
	if (Broodwar->isVisible((TilePosition)enemy.getPosition()))
	{
		if (enemy.m_unit)
		{
			Attack(attacker, enemy.m_unit);
		}
		else
		{
			Attack(attacker, enemy.getPosition());
		}
	}
	else
	{
		Move(attacker, enemy.getPosition());
	}
}

void OrderManager::Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::Move(BWAPI::Unit mover, const EnemyUnitInfo& u, bool shiftClick)
{
	Move(mover, u.getPosition(), shiftClick);
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
