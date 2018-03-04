#include "BaseManager.h"

#include "BuildOrderManager.h"
#include "BuildingManager.h"
#include "DebugMessenger.h"
#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

void BaseManager::ManageBases(BWAPI::Unit base)
{
	auto result = hatcheries.emplace(base);

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

	if (!BuildOrderManager::Instance().buildOrderComplete) {
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())
		    && (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Drone)) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Drone);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}

		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Overlord)) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Overlord);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}

		const auto poolready     = BuildOrderManager::Instance().poolready;
		const auto gotZergMoney  = Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice();
		const auto gotZergSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0;
		const auto zergBO        = *BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Zergling;

		if (poolready && gotZergMoney
		    && gotZergSupply
		    && zergBO) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Zergling);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}

	} else {
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1)) {
			if (!base->getLarva().empty()) {
				base->train(UnitTypes::Zerg_Overlord);
			}
		}

		auto poolready     = BuildOrderManager::Instance().poolready;
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

// Details for mineral lock
//s7jones: @PurpleWaveJadien In plain english, is mineral locking : -spam worker on mineral until it has gathered - return with minerals once it has gathered - each worker assigned to one mineral always
//jaj22 : spam every other frame at most
//s7jones : @PurpleWaveJadien - also have no more than 2 / 2.5 workers to a mineral patch ? -choose minerals based on closest then least amount of workers ?
//Moderator Subscriber Twitch Prime PurpleWaveJadien : spam until it starts gathering, then stop spamming
//bftjoe : you check which mineral its trying to target and only issue command if its wrong
//Moderator Subscriber Twitch Prime PurpleWaveJadien : 2 workers basically saturates a mineral with mineral locking.you get extremely marginal return past that; you'd likely be better off long distance mining
//Moderator Subscriber Twitch Prime PurpleWaveJadien : ^ right, what bftjoe said.spam whenever getTarget is a different mineralwant

void BaseManager::DoAllWorkerTasks(BWAPI::Unit unit)
{
	workers.insert(unit);

	// if a miner
	if (miners.find(unit) != miners.end()) {
		auto invaders = hatcheries.begin()->checkForInvaders();
		for (auto invader : invaders) {
			std::pair<bool, invaderAndDefender> foundDefensePair
			    = { false, { nullptr, nullptr } };
			for (auto defencePair : targetsAndAssignedDefenders) {
				if (invader == defencePair.first) {
					foundDefensePair = { true, defencePair };
					break;
				}
			}
			// invader not been assigned a defender
			if (!foundDefensePair.first) {
				// always keep 1 mining
				if (miners.size() > 1) {
					targetsAndAssignedDefenders.insert({ invader, unit });
					OrderManager::Instance().attack(unit, invader);
					GUIManager::Instance().drawTextOnScreen(unit, "this is SPARTA!");
					defenders.insert(unit);
					miners.erase(unit);
				}
				return;
			} else {
				auto defencePair = foundDefensePair.second;
				// assigneddefender is destroyed
				if (!defencePair.second->exists()) {
					defenders.erase(defencePair.second);
					targetsAndAssignedDefenders.erase(defencePair);
					if (miners.size() > 1) {
						targetsAndAssignedDefenders.insert({ invader, unit });
						OrderManager::Instance().attack(unit, invader);
						GUIManager::Instance().drawTextOnScreen(unit, "prepare for glory (reinforce)");
						defenders.insert(unit);
						miners.erase(unit);
					}
					return;
				}
			}
		}
	}

	if (defenders.find(unit) != defenders.end()) {
		if (distanceAir(unit->getPosition(), hatcheries.begin()->base->getPosition()) > hatcheries.begin()->borderRadius) {
			OrderManager::Instance().stop(unit);
			GUIManager::Instance().drawTextOnScreen(unit, "don't chase");
			defenders.erase(unit);
			for (auto defencePair : targetsAndAssignedDefenders) {
				if (defencePair.second == unit) {
					targetsAndAssignedDefenders.erase(defencePair);
				}
			}
		}
	}

	// if our worker is idle
	if (unit->isIdle()) {
		defenders.erase(unit);

		// Order workers carrying a resource to return them to the center,
		// otherwise find a mineral patch to harvest.
		if (unit->isCarryingGas() || unit->isCarryingMinerals()) {
			unit->returnCargo();
			miners.insert(unit);
		}
		// The worker cannot harvest anything if it
		// is carrying a powerup such as a flag
		else if (!unit->getPowerUp()) {
			// Harvest from the nearest mineral patch or gas refinery
			auto success = unit->gather(unit->getClosestUnit(IsMineralField || IsRefinery));
			if (!success) {
				// If the call fails, then print the last error message
				DebugMessenger::Instance() << Broodwar->getLastError() << std::endl;
				DebugMessenger::Instance() << "Worker couldn't gather mineral or from refinery" << std::endl;
				const auto closestKnownMineral = InformationManager::Instance().getClosestMineral(unit);
				if (closestKnownMineral) {
					OrderManager::Instance().move(unit, closestKnownMineral->getPosition());
				}
			} else {
				miners.insert(unit);
			}

		} // closure: has no powerup
		else {
			DebugMessenger::Instance() << "is idle and has power up?" << std::endl;
		}
	}

	// if it isn't a defender
	if (defenders.find(unit) == defenders.end()) {
		BuildingManager::Instance().isAnythingToBuild(unit);
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
		auto it = hatcheries.find(unit);
		if (it != hatcheries.end()) {
			it = hatcheries.erase(it);
		}
	}

	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		workers.erase(unit);
		miners.erase(unit);
		defenders.erase(unit);
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
		miners.erase(unit);
		defenders.erase(unit);
	}
}

void BaseManager::onUnitRenegade(BWAPI::Unit unit)
{
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		workers.erase(unit);
		miners.erase(unit);
		defenders.erase(unit);
	}
}

Base::Base(BWAPI::Unit unit)
    : base(unit)
{
}

BWAPI::Unitset Base::checkForInvaders() const
{
	auto units = Unitset::none;
	if (base) {
		units   = base->getUnitsInRadius((int)floor(borderRadius), IsEnemy && !IsFlying);
		auto it = units.begin();
		while (it != units.end()) {
			auto unit = *it;
			if (BWTA::getRegion(unit->getPosition()) != BWTA::getRegion(base->getPosition())) {
				it = units.erase(it);
			} else {
				it++;
			}
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
		auto dist = distanceAir(base->getPosition(), p);
		if (maxDist < dist) {
			maxDist = dist;
		}
	}

	borderRadius = maxDist;
}
