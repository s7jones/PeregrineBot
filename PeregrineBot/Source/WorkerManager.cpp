#include "WorkerManager.h"

using namespace BWAPI;
using namespace Filter;

WorkerManager::WorkerManager()
{
}

WorkerManager& WorkerManager::Instance()
{
	static WorkerManager instance;
	return instance;
}

void WorkerManager::DoAllWorkerTasks(Unit u)
{
	// if our worker is idle
	if (u->isIdle()) {
		// Order workers carrying a resource to return them to the center,
		// otherwise find a mineral patch to harvest.
		if (u->isCarryingGas() || u->isCarryingMinerals()) {
			u->returnCargo();
		}
		// The worker cannot harvest anything if it
		// is carrying a powerup such as a flag
		else if (!u->getPowerUp()) {
			// Harvest from the nearest mineral patch or gas refinery
			if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery))) {
				// If the call fails, then print the last error message
				DebugMessenger::Instance() << Broodwar->getLastError() << std::endl;
			}

		} // closure: has no powerup
		else {
			DebugMessenger::Instance() << "is idle and has power up?" << std::endl;
		}
	}
	if (bo[indx] == UnitTypes::Zerg_Spawning_Pool) {
		if ((!pool) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Spawning_Pool.mineralPrice())) {
			if ((poolLastChecked + 115) < Broodwar->getFrameCount()) {
				//find a location for spawning pool and construct it
				TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Spawning_Pool, u->getTilePosition());
				OrderManager::Instance().Build(u, UnitTypes::Zerg_Spawning_Pool, buildPosition);
				poolLastChecked = Broodwar->getFrameCount();
				return;
			}
		}
	}
	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Hatchery.mineralPrice()) && (indx >= bo.size())) {
		if ((lastChecked + 400) < Broodwar->getFrameCount()) {
			TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Hatchery, u->getTilePosition());
			OrderManager::Instance().Build(u, UnitTypes::Zerg_Hatchery, buildPosition);
			lastChecked = Broodwar->getFrameCount();
			return;
		}
	}
}

void WorkerManager::DoAllWorkerTasks2(Unit u)
{
	if (bo[indx] == UnitTypes::Zerg_Spawning_Pool) {
		if ((!pool) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Spawning_Pool.mineralPrice())) {
			if ((poolLastChecked + 115) < Broodwar->getFrameCount()) {
				//find a location for spawning pool and construct it
				TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Spawning_Pool, u->getTilePosition());
				OrderManager::Instance().Build(u, UnitTypes::Zerg_Spawning_Pool, buildPosition);
				poolLastChecked = Broodwar->getFrameCount();
				UnassignWorkerFromMineral(u);
				builderWorkers.insert(u);
				return;
			}
		}
	}

	if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Hatchery.mineralPrice()) && (indx >= bo.size())) {
		if ((lastChecked + 400) < Broodwar->getFrameCount()) {
			TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Hatchery, u->getTilePosition());
			OrderManager::Instance().Build(u, UnitTypes::Zerg_Hatchery, buildPosition);
			lastChecked = Broodwar->getFrameCount();
			UnassignWorkerFromMineral(u);
			builderWorkers.insert(u);
			return;
		}
	}

	// if worker isn't a builder then do worker lock gathering
	if (builderWorkers.find(u) == builderWorkers.end()) {
		WorkerLockGathering(u);
	}
	// otherwise if it's idle then assign it to mining
	else if (u->isIdle()) {
		builderWorkers.erase(u);
		WorkerLockGathering(u);
	}
}

//s7jones: @PurpleWaveJadien In plain english, is mineral locking : -spam worker on mineral until it has gathered - return with minerals once it has gathered - each worker assigned to one mineral always
//jaj22 : spam every other frame at most
//s7jones : @PurpleWaveJadien - also have no more than 2 / 2.5 workers to a mineral patch ? -choose minerals based on closest then least amount of workers ?
//Moderator Subscriber Twitch Prime PurpleWaveJadien : spam until it starts gathering, then stop spamming
//bftjoe : you check which mineral its trying to target and only issue command if its wrong
//Moderator Subscriber Twitch Prime PurpleWaveJadien : 2 workers basically saturates a mineral with mineral locking.you get extremely marginal return past that; you'd likely be better off long distance mining
//Moderator Subscriber Twitch Prime PurpleWaveJadien : ^ right, what bftjoe said.spam whenever getTarget is a different mineralwant

void WorkerManager::WorkerLockGathering(Unit u)
{
	Unit mineralLocked = MineralAssociatedWithWorker(u);

	// if our worker is idle
	if (u->isIdle()) {
		// Order workers carrying a resource to return them to the center,
		// otherwise find a mineral patch to harvest.
		if (u->isCarryingGas() || u->isCarryingMinerals()) {
			u->returnCargo();
		}
		// The worker cannot harvest anything if it
		// is carrying a powerup such as a flag
		else if (!u->getPowerUp()) {
			Unit bestMineral = GetMineralPatch(u);
			// If mineral patch is found then
			if (bestMineral != NULL) {
				//u->gather(bestMineral);
				OrderManager::Instance().Gather(u, bestMineral);
			} else {
				DebugMessenger::Instance() << "couldn't find mineral patch" << std::endl;
			}
		} // closure: has no powerup
		else {
			DebugMessenger::Instance() << "is idle and has power up?" << std::endl;
		}
	} 
	// if our worker is moving to a resource depot with resources then continue
	else if (u->isMoving()) {
		if ((u->isCarryingGas() || u->isCarryingMinerals())) {
			//Unit target = u->getTarget();
			//if (target == NULL) {
			//	Broodwar << "null pointer!" << std::endl;
			//	u->returnCargo();
			//}
			//else if (target->getType().isResourceDepot()) {
			//	return;
			//}
			//else {
			//	u->returnCargo();
			//}
			return;
		}
	}
	// if our locked mineral is not null
	else if (mineralLocked != NULL) {
		if (u->getTarget() != mineralLocked) {
			//u->gather(mineralLocked);
			OrderManager::Instance().Gather(u, mineralLocked);
		}
	} else {
		DebugMessenger::Instance() << "mineral is null" << std::endl;
		Broodwar << "mineral is null" << std::endl;
		//u->stop();
		OrderManager::Instance().Stop(u);
	}
}

Unit WorkerManager::GetMineralPatch(Unit u)
{
	UnassignWorkerFromMineral(u);

	auto base                = BaseManager::Instance().mainBase;
	Unitset mineralsAtBase   = base->getUnitsInRadius(15 * 32, IsMineralField);
	Unitset startingMinerals = InformationManager::Instance().initialMinerals;

	int workerOptimalLevel    = 2;
	int workerSaturationLevel = 3;
	std::map<Unit, Unitset>::iterator bestChoice = assignedWorkersAllMinerals.end();
	int leastWorkersOnAMineral = std::numeric_limits<int>::max(); // largest int possible

	for (auto mineral : mineralsAtBase) {
		int currentWorkersOnMineral = 0;
		auto iter                   = assignedWorkersAllMinerals.find(mineral);
		// if mineral is in the map then check how many workers are assigned
		if (iter != assignedWorkersAllMinerals.end()) {
			currentWorkersOnMineral = iter->second.size();
		}
		// otherwise add it to be assigned and update the iterator
		else {
			auto result = assignedWorkersAllMinerals.insert(std::make_pair(mineral, Unitset::none));
			iter        = result.first;
		}
		//for (auto assignedWorkersOneMineral : assignedWorkersAllMinerals) {
		//	// if we have a worker assigned to the mineral in the loop we're on
		//	// then count the worker
		//	if (assignedWorkersOneMineral.mineral->getID() == mineral->getID()) {
		//		currentWorkersOnMineral = assignedWorkersOneMineral.workers.size();
		//	}
		//}
		if (currentWorkersOnMineral < leastWorkersOnAMineral) {
			leastWorkersOnAMineral = currentWorkersOnMineral;
		}
		// if the amount of workers on the mineral is less than saturation amount
		if (currentWorkersOnMineral < workerSaturationLevel) {
			// if we don't have a mineral candidate then take it
			if (bestChoice == assignedWorkersAllMinerals.end()) {
				bestChoice = iter;
			}
			// otherwise take it if it has less workers than candidate
			else if (iter->second.size() < bestChoice->second.size()) {
				bestChoice = iter;
			}
			// otherwise take it if it is closer than candidate
			else if (base->getDistance(mineral) < base->getDistance(bestChoice->first)) {
				bestChoice = iter;
			}
		} else {
			DebugMessenger::Instance() << "more workers than saturation on mineral" << std::endl;
		}
	}
	// if we got a best option from base minerals then add worker to map and return mineral
	if (bestChoice != assignedWorkersAllMinerals.end()) {
		bestChoice->second.insert(u);
		return bestChoice->first;
	} else {
		Unitset allUnits = Broodwar->getAllUnits();
		Unitset allCurrentMinerals;
		Unit closestMineral = NULL;
		for (auto oneUnit : allUnits) {
			if (oneUnit->getType().isMineralField()) {
				allCurrentMinerals.insert(oneUnit);
				if (closestMineral == NULL) {
					closestMineral = oneUnit;
				} else if (base->getDistance(oneUnit) < base->getDistance(closestMineral)) {
					closestMineral = oneUnit;
				}
			}
		}
		return closestMineral;
	}
}

void WorkerManager::UnassignWorkerFromMineral(Unit u)
{
	for (auto assignedWorkersOneMineral : assignedWorkersAllMinerals) {
		// if worker successfully removed then return
		Unitset workerSet = assignedWorkersOneMineral.second;
		if (workerSet.erase(u)) {
			return;
		}
	}
}

Unit WorkerManager::MineralAssociatedWithWorker(Unit u)
{
	for (auto assignedWorkersOneMineral : assignedWorkersAllMinerals) {
		// if worker successfully found then return mineral otherwise null
		Unitset workerSet = assignedWorkersOneMineral.second;
		if (workerSet.find(u) != workerSet.end()) {
			return assignedWorkersOneMineral.first;
		}
	}
	// it should only reach here if worker not found
	return NULL;
}

void WorkerManager::AddMinerals(Unitset minerals)
{
	for (auto mineral : minerals) {
		assignedWorkersAllMinerals.insert(std::make_pair(mineral, Unitset::none));
	}
}