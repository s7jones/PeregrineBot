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

void OrderManager::update() // run()
{
	// https://github.com/dgant/PurpleWave/blob/master/src/Micro/Agency/Commander.scala
	//nextOrderFrame.keys.filterNot(_.alive).foreach(nextOrderFrame.remove)
	//nextOrderFrame.keys.foreach(unit = > nextOrderFrame(unit) = Math.max(nextOrderFrame(unit), AttackDelay.nextSafeOrderFrame(unit)))
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

bool OrderManager::DoesUnitHasOrder(BWAPI::Unit unit) // ready()
{
	auto it = unitsToWaitAfterOrder.find(unit);
	return (it != unitsToWaitAfterOrder.end());
}

void OrderManager::Attack(BWAPI::Unit attacker, BWAPI::Position p)
{
	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(p);
}

void OrderManager::Attack(BWAPI::Unit attacker, BWAPI::Unit target)
{
	if (!DoesUnitHasOrder(attacker)) return;

	//ready for attack order?

	if (attacker->getType() == UnitTypes::Protoss_Photon_Cannon) return;

	auto interceptors = attacker->getInterceptors();
	if (attacker->getInterceptorCount()) {
		attacker->attack(target.getPosition());
	} else if (target->isVisible()) {
		auto moving          = attacker->isMoving();
		auto alreadyInRange  = attacker->isInWeaponRange(target);
		auto overdueToAttack = attacker->cooldown

		                       auto shouldOrder
	}

	unitsToWaitAfterOrder.insert({ attacker, 0 });
	attacker->attack(target);
}

void OrderManager::Attack(BWAPI::Unit attacker, UnitInfo u)
{
	Attack(attacker, u.getPosition());
}

void OrderManager::Move(BWAPI::Unit mover, BWAPI::Position p, bool shiftClick)
{
	unitsToWaitAfterOrder.insert({ mover, 0 });
	mover->move(p, shiftClick);
}

void OrderManager::Move(BWAPI::Unit mover, UnitInfo u, bool shiftClick)
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
