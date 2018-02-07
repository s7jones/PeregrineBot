#include "BaseManager.h"

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
		hatcheries.erase(unit);
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

void BaseManager::ManageBases(Unit u)
{
	for (auto trainee : workersTraining) {
		if (!trainee->exists()) {
			if (!trainee->isMorphing()) {
				if (trainee->getType() == UnitTypes::Zerg_Drone) {
					workersTraining.erase(trainee);
					workers.insert(trainee);
				} else if (trainee->getType() == UnitTypes::Zerg_Larva) {
					workersTraining.erase(trainee);
					Broodwar << "ERR: training worker is larva" << std::endl;
				}
			} else {
				workersTraining.erase(trainee);
				Broodwar << "ERR: training worker isn't morphing" << std::endl;
			}
		}
	}

	hatcheries.insert(u);
	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice()) && (WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Drone)) {
		if (!u->getLarva().empty()) {
			u->train(UnitTypes::Zerg_Drone);
			WorkerManager::Instance().indx++;
		}
	}

	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice()) && ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Overlord) || ((WorkerManager::Instance().indx >= WorkerManager::Instance().bo.size()) && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1)))) {
		if (!u->getLarva().empty()) {
			u->train(UnitTypes::Zerg_Overlord);
			WorkerManager::Instance().indx++;
		}
	}

	for (auto& u2 : Broodwar->self()->getUnits()) {
		if ((IsWorker)(u2))
			workers.insert(u2);
	}

	if ((workers.size() + workersTraining.size() < (hatcheries.size() * 3)) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())) {
		if (!u->getLarva().empty()) {
			u->train(UnitTypes::Zerg_Drone);
			workersTraining.insert(u);
			DebugMessenger::Instance() << "droning up from " << workers.size() + workersTraining.size() - 1 << " to " << (hatcheries.size() * 3) << std::endl;
		}
	}

	if ((WorkerManager::Instance().poolready) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice())
	    && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0)
	    && ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Zergling)
	        || (WorkerManager::Instance().indx >= WorkerManager::Instance().bo.size()))) {
		if (!u->getLarva().empty()) {
			u->train(UnitTypes::Zerg_Zergling);
			WorkerManager::Instance().indx++;
		}
	}
}
