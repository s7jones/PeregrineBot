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
	auto result = m_hatcheries.emplace(base);

	//auto invaders = (*result.first).checkForInvaders();

	auto it = workersTraining.begin();
	while (it != workersTraining.end())
	{
		auto trainee = *it;
		bool erase   = false;
		if (!trainee->exists())
		{
			if (!trainee->isMorphing())
			{
				if (trainee->getType() == UnitTypes::Zerg_Drone)
				{
					erase = true;
					m_workers.insert(trainee);
				}
				else if (trainee->getType() == UnitTypes::Zerg_Larva)
				{
					erase = true;
					errorMessage("training worker is larva");
				}
			}
			else
			{
				erase = true;
				errorMessage("training worker isn't morphing");
			}
		}

		if (erase)
		{
			it = workersTraining.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (!BuildOrderManager::Instance().buildOrderComplete)
	{
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice())
		    && (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Drone))
		{
			if (!base->getLarva().empty())
			{
				base->train(UnitTypes::Zerg_Drone);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}

		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Overlord))
		{
			if (!base->getLarva().empty())
			{
				base->train(UnitTypes::Zerg_Overlord);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}

		auto poolready     = BuildOrderManager::Instance().poolready;
		auto gotZergMoney  = Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice();
		auto gotZergSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0;
		auto zergBO        = *BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Zergling;

		if (poolready && gotZergMoney
		    && gotZergSupply
		    && zergBO)
		{
			if (!base->getLarva().empty())
			{
				base->train(UnitTypes::Zerg_Zergling);
				BuildOrderManager::Instance().incrementBuildOrder();
			}
		}
	}
	else
	{
		if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice())
		    && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1))
		{
			if (!base->getLarva().empty())
			{
				base->train(UnitTypes::Zerg_Overlord);
			}
		}

		auto poolready     = BuildOrderManager::Instance().poolready;
		auto gotZergMoney  = Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice();
		auto gotZergSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0;

		if (poolready && gotZergMoney
		    && gotZergSupply)
		{
			if (!base->getLarva().empty())
			{
				base->train(UnitTypes::Zerg_Zergling);
			}
		}
	}

	for (auto& unit : Broodwar->self()->getUnits())
	{
		if ((IsWorker)(unit))
			m_workers.insert(unit);
	}

	if ((m_workers.size() + workersTraining.size() < (m_hatcheries.size() * 3)) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice()))
	{
		if (!base->getLarva().empty())
		{
			base->train(UnitTypes::Zerg_Drone);
			workersTraining.insert(base);
			DebugMessenger::Instance() << "droning up from " << m_workers.size() + workersTraining.size() - 1 << " to " << (m_hatcheries.size() * 3) << std::endl;
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

void BaseManager::DoAllWorkerTasks(BWAPI::Unit u)
{
	m_workers.insert(u);

	DefendWithMiner(u);

	if (m_defenders.find(u) != m_defenders.end())
	{
		if (distanceAir(u->getPosition(), m_hatcheries.begin()->base->getPosition()) > m_hatcheries.begin()->m_borderRadius)
		{
			OrderManager::Instance().Stop(u);
			GUIManager::Instance().drawTextOnScreen(u, "don't chase");
			m_defenders.erase(u);
			for (auto defencePair : m_targetsAndAssignedDefenders)
			{
				if (defencePair.second == u)
				{
					m_targetsAndAssignedDefenders.erase(defencePair);
				}
			}
		}
	}

	// if our worker is idle
	if (u->isIdle())
	{
		m_defenders.erase(u);

		// Order workers carrying a resource to return them to the center,
		// otherwise find a mineral patch to harvest.
		if (u->isCarryingGas() || u->isCarryingMinerals())
		{
			u->returnCargo();
			m_miners.insert(u);
		}
		// The worker cannot harvest anything if it
		// is carrying a powerup such as a flag
		else if (!u->getPowerUp())
		{
			// Harvest from the nearest mineral patch or gas refinery
			auto success = u->gather(u->getClosestUnit(IsMineralField || IsRefinery));
			if (!success)
			{
				// If the call fails, then print the last error message
				DebugMessenger::Instance() << Broodwar->getLastError() << std::endl;
				DebugMessenger::Instance() << "Worker couldn't gather mineral or from refinery" << std::endl;
				const auto closestKnownMineral = InformationManager::Instance().getClosestMineral(u);
				if (closestKnownMineral)
				{
					OrderManager::Instance().Move(u, closestKnownMineral->getPosition());
				}
			}
			else
			{
				m_miners.insert(u);
			}

		} // closure: has no powerup
		else
		{
			DebugMessenger::Instance() << "is idle and has power up?" << std::endl;
		}
	}

	// if it isn't a defender
	if (m_defenders.find(u) == m_defenders.end())
	{
		BuildingManager::Instance().isAnythingToBuild(u);
	}
}

void BaseManager::DefendWithMiner(BWAPI::Unit u)
{
	// if a miner
	if (m_miners.find(u) == m_miners.end())
	{
		return;
	}

	if (m_hatcheries.size() == 0)
	{
		return;
	}

	auto invaders = m_hatcheries.begin()->checkForInvaders();
	for (auto invader : invaders)
	{
		std::pair<bool, invaderAndDefender> foundDefensePair
		    = { false, { nullptr, nullptr } };
		for (auto defencePair : m_targetsAndAssignedDefenders)
		{
			if (invader == defencePair.first)
			{
				foundDefensePair = { true, defencePair };
				break;
			}
		}

		// invader not been assigned a defender
		if (!foundDefensePair.first)
		{
			// always keep 1 mining
			if (m_miners.size() == 1)
			{
				return;
			}
			m_targetsAndAssignedDefenders.insert({ invader, u });
			OrderManager::Instance().Attack(u, invader);
			GUIManager::Instance().drawTextOnScreen(u, "this is SPARTA!");
			m_defenders.insert(u);
			m_miners.erase(u);
		}
		else
		{
			auto defencePair = foundDefensePair.second;

			if (defencePair.second->exists())
			{
				return;
			}

			// assigneddefender is destroyed
			m_defenders.erase(defencePair.second);
			m_targetsAndAssignedDefenders.erase(defencePair);

			// always keep 1 mining
			if (m_miners.size() == 1)
			{
				return;
			}

			m_targetsAndAssignedDefenders.insert({ invader, u });
			OrderManager::Instance().Attack(u, invader);
			GUIManager::Instance().drawTextOnScreen(u, "prepare for glory (reinforce)");
			m_defenders.insert(u);
			m_miners.erase(u);
		}
	}
}

void BaseManager::onUnitShow(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0)
	{
		m_workers.insert(unit);
	}
}

void BaseManager::onUnitCreate(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0)
	{
		m_workers.insert(unit);
	}
}

void BaseManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isResourceDepot() && unit->getPlayer() == Broodwar->self())
	{
		auto it = m_hatcheries.find(unit);
		if (it != m_hatcheries.end())
		{
			it = m_hatcheries.erase(it);
		}
	}

	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self())
	{
		m_workers.erase(unit);
		m_miners.erase(unit);
		m_defenders.erase(unit);
	}
}

void BaseManager::onUnitMorph(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0)
	{
		m_workers.insert(unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == Broodwar->self() && unit->getPlayer()->getRace() == Races::Zerg)
	{
		m_workers.erase(unit);
		m_miners.erase(unit);
		m_defenders.erase(unit);
	}
}

void BaseManager::onUnitRenegade(BWAPI::Unit unit)
{
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self())
	{
		m_workers.erase(unit);
		m_miners.erase(unit);
		m_defenders.erase(unit);
	}
}

Base::Base(BWAPI::Unit u)
    : base(u)
{
}

BWAPI::Unitset Base::checkForInvaders() const
{
	auto units = Unitset::none;
	if (base)
	{
		units   = base->getUnitsInRadius((int)floor(m_borderRadius), IsEnemy && !IsFlying);
		auto it = units.begin();
		while (it != units.end())
		{
			auto unit = *it;
			if (BWTA::getRegion(unit->getPosition()) != BWTA::getRegion(base->getPosition()))
			{
				it = units.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	return units;
}

void Base::calculateBorder()
{
	auto region = BWTA::getRegion(base->getPosition());
	if (region == nullptr)
	{
		return;
	}

	auto& poly = region->getPolygon();

	double maxDist = 0;
	for (auto p : poly)
	{
		auto dist = distanceAir(base->getPosition(), p);
		if (maxDist < dist)
		{
			maxDist = dist;
		}
	}

	m_borderRadius = maxDist;
}
