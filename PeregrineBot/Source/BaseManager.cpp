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

	for (auto trainee : workersTraining) {
		if (!trainee->exists()) {
			if (!trainee->isMorphing()) {
				if (trainee->getType() == UnitTypes::Zerg_Drone) {
					workersTraining.erase(trainee);
					workers.insert(trainee);
				} else if (trainee->getType() == UnitTypes::Zerg_Larva) {
					workersTraining.erase(trainee);
					errorMessage("training worker is larva");
				}
			} else {
				workersTraining.erase(trainee);
				errorMessage("training worker isn't morphing");
			}
		}
	}

	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice()) && (WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Drone)) {
		if (!base->getLarva().empty()) {
			base->train(UnitTypes::Zerg_Drone);
			WorkerManager::Instance().indx++;
		}
	}

	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice()) && ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Overlord) || ((WorkerManager::Instance().indx >= WorkerManager::Instance().bo.size()) && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1)))) {
		if (!base->getLarva().empty()) {
			base->train(UnitTypes::Zerg_Overlord);
			WorkerManager::Instance().indx++;
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

	if ((WorkerManager::Instance().poolready) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice())
	    && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0)
	    && ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Zergling)
	        || (WorkerManager::Instance().indx >= WorkerManager::Instance().bo.size()))) {
		if (!base->getLarva().empty()) {
			base->train(UnitTypes::Zerg_Zergling);
			WorkerManager::Instance().indx++;
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
	for (auto unit : units) {
		if (BWTA::getRegion(unit->getPosition()) != BWTA::getRegion(base->getPosition())) {
			units.erase(unit);
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
