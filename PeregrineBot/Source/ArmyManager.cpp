#include "ArmyManager.h"

#include "DebugMessenger.h"
#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "Utility.h"
#include "UtilityManager.h"

using namespace BWAPI;
using namespace Filter;

void ArmyManager::ZerglingAttack(BWAPI::Unit u)
{
	const auto& enemyMain              = InformationManager::Instance().m_enemyMain;
	const auto& enemyRace              = InformationManager::Instance().m_enemyRace;
	const auto& unscoutedPositions     = InformationManager::Instance().unscoutedPositions;
	const bool isEnemyBaseFromSpotting = InformationManager::Instance().m_isEnemyBaseFromSpotting;
	const bool isEnemyBaseDeduced      = InformationManager::Instance().m_isEnemyBaseDeduced;
	const bool isEnemyBaseReached      = InformationManager::Instance().isEnemyBaseReached;
	const bool isEnemyBaseDestroyed    = InformationManager::Instance().isEnemyBaseDestroyed;
	const auto& enemyBaseSpottingGuess = InformationManager::Instance().enemyBaseSpottingGuess;
	const auto& enemyBuildings         = InformationManager::Instance().m_enemyBuildings;
	const auto& enemyArmy              = InformationManager::Instance().m_enemyArmy;

	if (enemyMain.m_unit)
	{
		if (!isEnemyBaseReached)
		{
			auto region = BWTA::getRegion(u->getPosition());
			if (region)
			{
				if (region == BWTA::getRegion(enemyMain.getPosition()))
				{
					InformationManager::Instance().isEnemyBaseReached = true;
					DebugMessenger::Instance() << "reach enemy base: " << Broodwar->getFrameCount() << "F" << std::endl;
				}
			}
		}
	}

	/* ## isIdle calling false after previous attack order in frame
	s7jones / Peregrine - Today at 12:51 AM
	    so basically if I call Unit->attack(something); then next line
	    Unit->isIdle();

	    then isIdle should return false?
	jaj22 - Today at 12:51 AM
	    yeah, I just checked
	++N00byEdge - Today at 12:51 AM
	    yes, if you're using latcom
	s7jones / Peregrine - Today at 12:51 AM
	    that's crazy, I really don't have good intuitions about BWAPI and the
	engine
	*/

	bool priorityTarget = UtilityManager::Instance().getBestActionForZergling(u);

	if (priorityTarget)
	{
		return;
	}

	if (u->isIdle())
	{
		Unit closestGroundEnemy = u->getClosestUnit(IsEnemy && !IsFlying);
		if (closestGroundEnemy)
		{
			OrderManager::Instance().Attack(u, closestGroundEnemy);
		}
		else
		{
			if (enemyMain.m_unit)
			{
				if (!isEnemyBaseDestroyed)
				{
					if (!Broodwar->isVisible(TilePosition(enemyMain.getPosition())))
					{
						OrderManager::Instance().Attack(u, enemyMain);
					}
					else if (!Broodwar->getUnitsOnTile(
					                      TilePosition(enemyMain.getPosition()),
					                      IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
					              .empty())
					{
						OrderManager::Instance().Attack(u, enemyMain);
					}
					else if (enemyBuildings.size() != 0)
					{
						ZerglingAttackKnownBuildings(u);
					}
					else
					{
						ZerglingScoutSpreadOut(u);
					}
				}
				else if (enemyBuildings.size() != 0)
				{
					ZerglingAttackKnownBuildings(u);
				}
				else
				{
					ZerglingScoutSpreadOut(u);
				}
			}
			else
			{
				if (enemyBuildings.size() != 0)
				{
					ZerglingAttackKnownBuildings(u);
				}
				else if (isEnemyBaseFromSpotting)
				{
					OrderManager::Instance().Move(u, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(u, "scout overlord spot", 480);
				}
				else
				{
					ZerglingScoutingBeforeBaseFound(u);
				}
			}
		}
	}
	else if (u->isMoving())
	{ // attack move is most likely not covered here
		UnitCommand lastCmd = u->getLastCommand();
		if (lastCmd.getType() == UnitCommandTypes::Move)
		{
			Position targetPos = lastCmd.getTargetPosition();
			if (!enemyMain.m_unit)
			{
				if (isEnemyBaseFromSpotting
				    && enemyBaseSpottingGuess != targetPos)
				{
					OrderManager::Instance().Move(u, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting (overlord guess)", 480);
				}
				// if moving to somewhere already scouted
				else if (unscoutedPositions.count(targetPos) == 0)
				{
					if (!unscoutedPositions.empty())
					{
						auto p = *unscoutedPositions.begin();
						OrderManager::Instance().Move(u, p);
						GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting 1", 480);
					}
					// if moving to somewhere not in zergScoutLocations and there are places to go
					else if ((std::find(m_scoutLocationsZergling.begin(), m_scoutLocationsZergling.end(), targetPos) == m_scoutLocationsZergling.end())
					         && !m_scoutLocationsZergling.empty())
					{
						auto p = *m_scoutLocationIndex;
						OrderManager::Instance().Move(u, p);
						GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting 2", 480);
						incrementScoutLocationZerglingIndex();
					}
				}
			}
		}
		else if (lastCmd.getType() == UnitCommandTypes::Attack_Unit)
		{
			using EnemyContainer            = decltype(enemyArmy);
			auto lastCommandUnitInContainer = [lastCmd](EnemyContainer container) -> bool {
				auto it = std::find_if(container.begin(), container.end(),
				                       [lastCmd](const EnemyUnitInfo& enemy) -> bool {
					                       return lastCmd.getUnit() == enemy.m_unit;
				                       });
				return it != container.end();
			};

			if (!lastCommandUnitInContainer(enemyBuildings)
			    && !lastCommandUnitInContainer(enemyArmy))
			{
				errorMessage("unit not in enemy containers, stopping.");
				OrderManager::Instance().Stop(u);
			}
		}
	}
}

std::set<EnemyUnitInfo> ArmyManager::GetZerglingAccessibleBuildings(BWAPI::Unit zergling)
{
	auto enemyBuildings = InformationManager::Instance().m_enemyBuildings;
	std::set<EnemyUnitInfo> enemyBuildingsAccessible;

	auto zerglingPos    = zergling->getPosition();
	auto zerglingRegion = BWTA::getRegion(zerglingPos);

	if (zerglingRegion == nullptr)
	{
		// TODO: warning
		return enemyBuildingsAccessible;
	}

	for (auto building : enemyBuildings)
	{
		auto buildingPos    = building.getPosition();
		auto buildingRegion = BWTA::getRegion(buildingPos);

		if (buildingRegion == nullptr)
		{
			continue;
		}

		// if building isn't reachable then skip
		if (!isReachable(zerglingRegion, buildingRegion))
		{
			DebugMessenger::Instance() << "unaccessible building" << std::endl;
			if (!building.exists())
			{
				errorMessage("building doesn't exist");
			}
			continue;
		}
		else
		{
			enemyBuildingsAccessible.insert(building);
			DebugMessenger::Instance() << "scoutable building" << std::endl;
		}
	}
	return enemyBuildingsAccessible;
}

void ArmyManager::ZerglingAttackKnownBuildings(BWAPI::Unit u)
{
	auto enemyBuildingsAccessible = GetZerglingAccessibleBuildings(u);

	if (enemyBuildingsAccessible.size() != 0)
	{

		double distanceEnemyBuildingAccessible = std::numeric_limits<double>::infinity();
		Position buildingAccessiblePos;

		for (auto building : enemyBuildingsAccessible)
		{
			Position buildingPos    = building.getPosition();
			double distanceBuilding = distanceAir(u->getPosition(), buildingPos);
			if (distanceBuilding < distanceEnemyBuildingAccessible)
			{
				distanceEnemyBuildingAccessible = distanceBuilding;
				buildingAccessiblePos           = buildingPos;
			}
		}

		OrderManager::Instance().Attack(u, buildingAccessiblePos);
		GUIManager::Instance().drawTextOnScreen(u, "attacking accessible building", 48);
		// DebugMessenger::Instance() << "attacking accessible building" <<
		// std::endl;
	}
}

void ArmyManager::ZerglingScoutingBeforeBaseFound(BWAPI::Unit u)
{
	auto scoutingOptions    = InformationManager::Instance().scoutingOptions;
	auto scoutedPositions   = InformationManager::Instance().scoutedPositions;
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;

	if (Broodwar->getStartLocations().size() == 4)
	{ // map size is 4, use new scouting
		auto tp1                       = scoutingOptions.begin()->startToP1ToP2[1];
		auto p1                        = getBasePos(tp1);
		const bool firstOptionScouted  = scoutedPositions.find(p1) != scoutedPositions.end();
		auto tp2                       = scoutingOptions.begin()->startToP1ToP2[2];
		auto p2                        = getBasePos(tp2);
		const bool secondOptionScouted = scoutedPositions.find(p2) != scoutedPositions.end();
		std::stringstream ss;
		if (!firstOptionScouted)
		{
			OrderManager::Instance().Move(u, p1);
			ss << " moving to 1: " << tp1.x << "," << tp1.y << "TP";
			// DebugMessenger::Instance() << " moving to 1: " << tp1.x << "," <<
			// tp1.y << "TP" << std::endl;
		}
		else if (!secondOptionScouted)
		{
			OrderManager::Instance().Move(u, p2);
			ss << " moving to 2: " << tp2.x << "," << tp2.y << "TP";
			// DebugMessenger::Instance() << " moving to 2: " << tp2.x << "," <<
			// tp2.y << "TP" << std::endl;
		}
		else
		{
			auto p = *unscoutedPositions.begin();
			OrderManager::Instance().Move(u, p);
			auto tp3 = (TilePosition)p;
			ss << " moving to else: " << tp3.x << "," << tp3.y << "TP";
			// DebugMessenger::Instance() << " moving to else: " << tp3.x << ","
			// << tp3.y << "TP" << std::endl;
		}
		GUIManager::Instance().drawTextOnScreen(u, ss.str(), 480);
	}
	else
	{ // map size isn't 4, so use old scouting
		auto p = *unscoutedPositions.begin();
		OrderManager::Instance().Move(u, p);
	}
}

void ArmyManager::ZerglingScoutSpreadOut(BWAPI::Unit u)
{
	auto enemyBuildings = InformationManager::Instance().m_enemyBuildings;
	if (m_scoutLocationsZergling.empty())
	{
		GenerateZergScoutPositions(u);

		// deque iterators NOT valid after insertion
		m_scoutLocationIndex = m_scoutLocationsZergling.begin();
	}
	else
	{
		GUIManager::Instance().drawTextOnScreen(u, "Zergling Scouting!", 48);
		Position p = *m_scoutLocationIndex;
		OrderManager::Instance().Move(u, p);
		incrementScoutLocationZerglingIndex();
	}
}

void ArmyManager::GenerateZergScoutPositions(BWAPI::Unit u)
{
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;

	auto enemyBuildingsAccessible = GetZerglingAccessibleBuildings(u);

	for (auto building : enemyBuildingsAccessible)
	{
		Position buildingPos = building.getPosition();
		m_scoutLocationsZergling.push_front(buildingPos);
	}

	for (const auto& unscoutedLocation : unscoutedPositions)
	{
		m_scoutLocationsZergling.push_back(unscoutedLocation);
	}

	auto zerglingRegion = BWTA::getRegion(u->getPosition());
	if (zerglingRegion == nullptr)
	{
		return;
	}

	// TODO: don't add duplicate cases
	for (auto base : BWTA::getBaseLocations())
	{
		if (base == nullptr)
		{
			continue;
		}

		auto baseRegion = base->getRegion();
		if (baseRegion == nullptr)
		{
			continue;
		}
		if (!isReachable(zerglingRegion, baseRegion))
		{
			continue;
		}
		m_scoutLocationsZergling.push_back(base->getPosition());
	}

	for (auto region : BWTA::getRegions())
	{
		if (region == nullptr)
		{
			continue;
		}
		if (!isReachable(zerglingRegion, region))
		{
			continue;
		}
		auto& poly = region->getPolygon();
		for (size_t j = 0; j < poly.size(); ++j)
		{
			Position point1 = poly[j];
			if (region == BWTA::getRegion(u->getPosition()))
			{
				m_scoutLocationsZergling.push_front(point1);
			}
			else
			{
				m_scoutLocationsZergling.push_back(point1);
			}
		}
	}
}

void ArmyManager::incrementScoutLocationZerglingIndex()
{
	++m_scoutLocationIndex;
	if (m_scoutLocationIndex == m_scoutLocationsZergling.end())
	{
		m_scoutLocationIndex = m_scoutLocationsZergling.begin();
	}
}
