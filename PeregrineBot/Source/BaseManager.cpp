#include "BaseManager.h"

#include "Utility.h"
#include "WorkerManager.h"

using namespace BWAPI;
using namespace Filter;

BaseManager::BaseManager()
{
}

BaseManager& BaseManager::Instance()
{
	static BaseManager instance;
	return instance;
}

void BaseManager::ManageBases(Unit base)
{
	auto result = hatcheries.emplace(base);

	if ((*result.first).borderRadius == 0) {
		(*result.first).calculateBorder();
	}

	//auto invaders = (*result.first).checkForInvaders();

	auto it = workersTraining.begin();
	while (it != workersTraining.end()) {
		auto trainee = *it;
		bool erase   = false;
		if (!trainee->exists()) {
			if (!trainee->isMorphing()) {
				if (trainee->getType() == UnitTypes::Zerg_Drone) {
					erase = true;
					workers.insert(trainee);
				} else if (trainee->getType() == UnitTypes::Zerg_Larva) {
					erase = true;
					errorMessage("training worker is larva");
				}
			} else {
				erase = true;
				errorMessage("training worker isn't morphing");
			}
		}

		if (erase) {
			it = workersTraining.erase(it);
		} else {
			it++;
		}
	}

	if (!WorkerManager::Instance().buildOrderComplete) {
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())
		    && (WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Drone)) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Drone);
				WorkerManager::Instance().incrementBuildOrder();
			}
		}

		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Overlord))) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Overlord);
				WorkerManager::Instance().incrementBuildOrder();
			}
		}

		auto poolready     = WorkerManager::Instance().poolready;
		auto gotZergMoney  = Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice();
		auto gotZergSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0;
		auto zergBO        = WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Zergling;

		if (poolready && gotZergMoney
		    && gotZergSupply
		    && zergBO) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Zergling);
				WorkerManager::Instance().incrementBuildOrder();
			}
		}

	} else {
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1)) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Overlord);
			}
		}

		auto poolready     = WorkerManager::Instance().poolready;
		auto gotZergMoney  = Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice();
		auto gotZergSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0;

		if (poolready && gotZergMoney
		    && gotZergSupply) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Zergling);
			}
		}
	}

	for (auto& unit : Broodwar->self()->getUnits()) {
		if ((IsWorker)(unit))
			workers.insert(unit);
	}

	if ((workers.size() + workersTraining.size() < (hatcheries.size() * 3)) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())) {
		if (!base->getLarva().empty()) {
			base->train(UnitTypes::Zerg_Drone);
			workersTraining.insert(base);
			DebugMessenger::Instance() << "droning up from " << workers.size() + workersTraining.size() - 1 << " to " << (hatcheries.size() * 3) << std::endl;
		}
	}
}

void BaseManager::onUnitShow(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		workers.insert(unit);
	}
}

void BaseManager::onUnitCreate(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		workers.insert(unit);
	}
}

void BaseManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isResourceDepot() && unit->getPlayer() == Broodwar->self()) {
		hatcheries.erase(Base(unit));
	}

	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		workers.erase(unit);
	}
}

void BaseManager::onUnitMorph(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		workers.insert(unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == Broodwar->self() && unit->getPlayer()->getRace() == Races::Zerg) {
		workers.erase(unit);
	}
}

void BaseManager::onUnitRenegade(BWAPI::Unit unit)
{
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		workers.erase(unit);
	}
}

Base::Base(BWAPI::Unit u)
    : base(u)
{
}

BWAPI::Unitset Base::checkForInvaders() const
{
	auto units = base->getUnitsInRadius(borderRadius, IsEnemy && !IsFlying);
	auto it    = units.begin();
	while (it != units.end()) {
		auto unit = *it;
		if (BWTA::getRegion(unit->getPosition()) != BWTA::getRegion(base->getPosition())) {
			it = units.erase(it);
		} else {
			it++;
		}
	}
	return units;
}

void Base::calculateBorder() const
{
	auto region = BWTA::getRegion(base->getPosition());
	auto& poly  = region->getPolygon();

	region->getChokepoints();

	double maxDist = 0;
	for (auto p : poly) {
		auto dist = DistanceAir(base->getPosition(), p);
		if (maxDist < dist) {
			maxDist = dist;
		}
	}

	borderRadius = maxDist;
}

bool Base::operator<(const Base& other) const
{
	return base < other.base;
}
