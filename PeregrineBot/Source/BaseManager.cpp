#include "BaseManager.h"

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

void BaseManager::SetupBaseAtStart()
{
	std::set<Unit> bases;
	for (Unit u : Broodwar->self()->getUnits()) {
		if (u->getType().isResourceDepot()) {
			bases.insert(u);
		}
	}

	if (bases.size() == 1) {
		mainBase = *bases.begin();
	} else {
		for (auto base : bases) {
			if ((TilePosition)base->getPosition() == Broodwar->self()->getStartLocation()) {
				mainBase = base;
				break;
			}
		}
		// Should not reach here
		DebugMessenger::Instance() << "Couldn't find main base at start" << std::endl;
	}
}

void BaseManager::ManageBases(Unit u)
{
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
			workerList.insert(u2);
	}

	if ((workerList.size() < (hatcheries.size() * 3)) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())) {
		if (!u->getLarva().empty()) {
			u->train(UnitTypes::Zerg_Drone);
			DebugMessenger::Instance() << "droning up from " << workerList.size() << " to " << (hatcheries.size() * 3) << std::endl;
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