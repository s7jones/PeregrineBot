#include "ArmyManager.h"

#include "DebugMessenger.h"
#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "Utility.h"
#include "UtilityManager.h"

using namespace BWAPI;
using namespace Filter;

void ArmyManager::update()
{
	putUnassignedInSquads();
	handleIdleUnits();

	for (auto squad : squads) {
		attackWithSquad(squad);
	}
}

void ArmyManager::putUnassignedInSquads()
{
	for (const auto friendly : InformationManager::Instance().friendlyUnits) {
		if (friendly.getType() != UnitTypes::Zerg_Zergling) {
			continue;
		}

		bool isUnitAssigned = false;

		Squad closestSquad;
		double closestSquadDistance = std::numeric_limits<double>::infinity();

		for (const auto squad : squads) {

			auto distance = distanceGround(friendly.getPosition(), squad.getPosition());
			if (distance < closestSquadDistance) {
				closestSquadDistance = distance;
				closestSquad         = squad;
			}

			if (squad.count(friendly.u)) {
				isUnitAssigned = true;
				break;
			}
		}

		if (!isUnitAssigned) {
			if (closestSquadDistance < SQUAD_RADIUS) {
				closestSquad.insert(friendly.u);
			} else {
				Squad newSquad;
				newSquad.insert(friendly.u);
				squads.insert(newSquad);
			}
		}
	}
}

void ArmyManager::attackWithSquad(Squad& squad)
{
	// for now let's try and leverage existing individual attack
	const auto enemyMain               = InformationManager::Instance().enemyMain;
	const auto enemyRace               = InformationManager::Instance().enemyRace;
	const auto unscoutedPositions      = InformationManager::Instance().unscoutedPositions;
	const auto isEnemyBaseFromSpotting = InformationManager::Instance().isEnemyBaseFromSpotting;
	const auto isEnemyBaseDeduced      = InformationManager::Instance().isEnemyBaseDeduced;
	const auto isEnemyBaseReached      = InformationManager::Instance().isEnemyBaseReached;
	const auto isEnemyBaseDestroyed    = InformationManager::Instance().isEnemyBaseDestroyed;
	const auto enemyBaseSpottingGuess  = InformationManager::Instance().enemyBaseSpottingGuess;
	const auto enemyBuildings          = InformationManager::Instance().enemyBuildings;
	const auto enemyArmy               = InformationManager::Instance().enemyArmy;

	for (const auto unit : squad) {
		if (enemyMain.u) {
			if ((!isEnemyBaseReached) && (BWTA::getRegion(unit->getPosition()) == BWTA::getRegion(enemyMain.getPosition()))) {
				InformationManager::Instance().isEnemyBaseReached = true;
				DebugMessenger::Instance()
				    << "reach enemy base: " << Broodwar->getFrameCount() << "F"
				    << std::endl;
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

	bool priorityTarget = UtilityManager::Instance().getBestActionForSquad(squad);

	if (priorityTarget) {
		return;
	}

	bool allIdle = true, someIdle = false;

	for (auto unit : squad) {
		if (!unit->isIdle()) {
			allIdle = false;
			break;
		} else {
			if (!someIdle) {
				someIdle = true;
			}
		}
	}

	auto idleState   = squad.isIdle();
	auto movingState = squad.isMoving();

	if (idleState == Squad::ALLIDLE) {
		Unit closestGroundEnemy = squad.getClosestUnit(IsEnemy && !IsFlying);
		if (closestGroundEnemy) {
			OrderManager::Instance().attack(squad, closestGroundEnemy);
		} else {
			if (enemyMain.u) {
				if (!isEnemyBaseDestroyed) {
					if (!Broodwar->isVisible(TilePosition(enemyMain.getPosition()))) {
						OrderManager::Instance().attack(squad, enemyMain);
					} else if (!Broodwar->getUnitsOnTile(
					                        TilePosition(enemyMain.getPosition()),
					                        IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
					                .empty()) {
						OrderManager::Instance().attack(squad, enemyMain);
					} else if (enemyBuildings.size() != 0) {
						zerglingAttackKnownBuildings(squad);
					} else {
						zerglingScoutSpreadOut(squad);
					}
				} else if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(squad);
				} else {
					zerglingScoutSpreadOut(squad);
				}
			} else {
				if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(squad);
				} else if (isEnemyBaseFromSpotting) {
					OrderManager::Instance().move(squad, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(*squad.begin(), "scout overlord spot", 480);
				} else {
					zerglingScoutingBeforeBaseFound(squad);
				}
			}
		}
	} else {
		if (Squad::SOMEIDLE) {
			// reassign idle members
		} else {
			if (Squad::ALLMOVING) { // attack move is most likely not covered here
				UnitCommand lastCmd = squad.begin()->getLastCommand();
				if (lastCmd.getType() == UnitCommandTypes::Move) {
					Position targetPos = lastCmd.getTargetPosition();
					if (!enemyMain.u) {
						if (isEnemyBaseFromSpotting
						    && enemyBaseSpottingGuess != targetPos) {
							OrderManager::Instance().move(squad, enemyBaseSpottingGuess);
							GUIManager::Instance().drawTextOnScreen(*squad.begin(), "recalculate scouting (overlord guess)", 480);
						}
						// if moving to somewhere already scouted
						else if (unscoutedPositions.count(targetPos) == 0) {
							if (!unscoutedPositions.empty()) {
								auto p = *unscoutedPositions.begin();
								OrderManager::Instance().move(squad, p);
								GUIManager::Instance().drawTextOnScreen(*squad.begin(), "recalculate scouting 1", 480);
							}
							// if moving to somewhere not in zergScoutLocations and there are places to go
							else if ((std::find(scoutLocationsZergling.begin(), scoutLocationsZergling.end(), targetPos)
							          == scoutLocationsZergling.end())
							         && !scoutLocationsZergling.empty()) {
								auto p = *scoutLocationIndex;
								OrderManager::Instance().move(squad, p);
								GUIManager::Instance().drawTextOnScreen(*squad.begin(), "recalculate scouting 2", 480);
								incrementScoutLocationZerglingIndex();
							}
						}
					}
				} else if (lastCmd.getType() == UnitCommandTypes::Attack_Unit) {
					using EnemyContainer            = std::set<EnemyUnitInfo>;
					auto lastCommandUnitInContainer = [lastCmd](EnemyContainer container) -> bool {
						auto it = std::find_if(container.begin(), container.end(),
						                       [lastCmd](const EnemyUnitInfo& enemy) -> bool {
							                       return lastCmd.getUnit() == enemy.u;
						                       });
						return it != container.end();
					};

					if (!lastCommandUnitInContainer(enemyBuildings)
					    && !lastCommandUnitInContainer(enemyArmy)) {
						errorMessage("unit not in enemy containers, stopping.");
						OrderManager::Instance().stop(squad);
					}
				}
			} else {
				if (Squad::SOMEMOVING) {
					// reassign non moving
				} else {
					// none are moving - do nothing
				}
			}
		}
	}
}

void ArmyManager::handleIdleUnits()
{
	std::set<Squad> idleSquadList, combinedSet;

	for (auto squad : squads) {
		auto it = squad.begin();
		while (squad.size() > 1 && it != squad.end()) {
			auto unit = (*it);
			if (unit->isIdle()) {
				Squad idleSquad;
				idleSquad.insert(unit);
				idleSquadList.insert(idleSquad);
				it = squad.erase(it);
			} else {
				it++;
			}
		}
	}

	// combine idle squads into squads
	set_union(squads.begin(), squads.end(),
	          idleSquadList.begin(), idleSquadList.end(),
	          inserter(combinedSet, combinedSet.begin()));
	squads = combinedSet;
}

void ArmyManager::zerglingAttack(BWAPI::Unit u)
{
	const auto enemyMain               = InformationManager::Instance().enemyMain;
	const auto enemyRace               = InformationManager::Instance().enemyRace;
	const auto unscoutedPositions      = InformationManager::Instance().unscoutedPositions;
	const auto isEnemyBaseFromSpotting = InformationManager::Instance().isEnemyBaseFromSpotting;
	const auto isEnemyBaseDeduced      = InformationManager::Instance().isEnemyBaseDeduced;
	const auto isEnemyBaseReached      = InformationManager::Instance().isEnemyBaseReached;
	const auto isEnemyBaseDestroyed    = InformationManager::Instance().isEnemyBaseDestroyed;
	const auto enemyBaseSpottingGuess  = InformationManager::Instance().enemyBaseSpottingGuess;
	const auto enemyBuildings          = InformationManager::Instance().enemyBuildings;
	const auto enemyArmy               = InformationManager::Instance().enemyArmy;

	if (enemyMain.u) {
		if ((!isEnemyBaseReached) && (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(enemyMain.getPosition()))) {
			InformationManager::Instance().isEnemyBaseReached = true;
			DebugMessenger::Instance()
			    << "reach enemy base: " << Broodwar->getFrameCount() << "F"
			    << std::endl;
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

	if (priorityTarget) {
		return;
	}

	if (u->isIdle()) {
		Unit closestGroundEnemy = u->getClosestUnit(IsEnemy && !IsFlying);
		if (closestGroundEnemy) {
			OrderManager::Instance().attack(u, closestGroundEnemy);
		} else {
			if (enemyMain.u) {
				if (!isEnemyBaseDestroyed) {
					if (!Broodwar->isVisible(TilePosition(enemyMain.getPosition()))) {
						OrderManager::Instance().attack(u, enemyMain);
					} else if (!Broodwar->getUnitsOnTile(
					                        TilePosition(enemyMain.getPosition()),
					                        IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
					                .empty()) {
						OrderManager::Instance().attack(u, enemyMain);
					} else if (enemyBuildings.size() != 0) {
						zerglingAttackKnownBuildings(u);
					} else {
						zerglingScoutSpreadOut(u);
					}
				} else if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(u);
				} else {
					zerglingScoutSpreadOut(u);
				}
			} else {
				if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(u);
				} else if (isEnemyBaseFromSpotting) {
					OrderManager::Instance().move(u, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(u, "scout overlord spot", 480);
				} else {
					zerglingScoutingBeforeBaseFound(u);
				}
			}
		}
	} else if (u->isMoving()) { // attack move is most likely not covered here
		UnitCommand lastCmd = u->getLastCommand();
		if (lastCmd.getType() == UnitCommandTypes::Move) {
			Position targetPos = lastCmd.getTargetPosition();
			if (!enemyMain.u) {
				if (isEnemyBaseFromSpotting
				    && enemyBaseSpottingGuess != targetPos) {
					OrderManager::Instance().move(u, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting (overlord guess)", 480);
				}
				// if moving to somewhere already scouted
				else if (unscoutedPositions.count(targetPos) == 0) {
					if (!unscoutedPositions.empty()) {
						auto p = *unscoutedPositions.begin();
						OrderManager::Instance().move(u, p);
						GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting 1", 480);
					}
					// if moving to somewhere not in zergScoutLocations and there are places to go
					else if ((std::find(scoutLocationsZergling.begin(), scoutLocationsZergling.end(), targetPos)
					          == scoutLocationsZergling.end())
					         && !scoutLocationsZergling.empty()) {
						auto p = *scoutLocationIndex;
						OrderManager::Instance().move(u, p);
						GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting 2", 480);
						incrementScoutLocationZerglingIndex();
					}
				}
			}
		} else if (lastCmd.getType() == UnitCommandTypes::Attack_Unit) {
			using EnemyContainer            = std::set<EnemyUnitInfo>;
			auto lastCommandUnitInContainer = [lastCmd](EnemyContainer container) -> bool {
				auto it = std::find_if(container.begin(), container.end(),
				                       [lastCmd](const EnemyUnitInfo& enemy) -> bool {
					                       return lastCmd.getUnit() == enemy.u;
				                       });
				return it != container.end();
			};

			if (!lastCommandUnitInContainer(enemyBuildings)
			    && !lastCommandUnitInContainer(enemyArmy)) {
				errorMessage("unit not in enemy containers, stopping.");
				OrderManager::Instance().stop(u);
			}
		}
	}
}

std::set<EnemyUnitInfo> ArmyManager::getZerglingAccessibleBuildings(BWAPI::Unit u)
{
	auto enemyBuildings = InformationManager::Instance().enemyBuildings;
	std::set<EnemyUnitInfo> enemyBuildingsAccessible;
	for (auto iter = enemyBuildings.begin(); iter != enemyBuildings.end(); iter++) {
		auto building    = *iter;
		auto buildingPos = building.getPosition();
		auto zerglingPos = u->getPosition();

		auto zerglingRegion = BWTA::getRegion(zerglingPos);
		auto buildingRegion = BWTA::getRegion(buildingPos);

		// if building isn't reachable then skip
		if (!isReachable(zerglingRegion, buildingRegion)) {
			DebugMessenger::Instance() << "unaccessible building" << std::endl;
			if (!building.exists()) {
				errorMessage("building doesn't exist");
			}
			continue;
		} else {
			enemyBuildingsAccessible.insert(building);
			DebugMessenger::Instance() << "scoutable building" << std::endl;
		}
	}
	return enemyBuildingsAccessible;
}

void ArmyManager::zerglingAttackKnownBuildings(BWAPI::Unit u)
{
	auto enemyBuildingsAccessible = getZerglingAccessibleBuildings(u);

	if (enemyBuildingsAccessible.size() != 0) {

		double distanceEnemyBuildingAccessible = std::numeric_limits<double>::infinity();
		Position buildingAccessiblePos;

		for (auto building : enemyBuildingsAccessible) {
			Position buildingPos    = building.getPosition();
			double distanceBuilding = distanceAir(u->getPosition(), buildingPos);
			if (distanceBuilding < distanceEnemyBuildingAccessible) {
				distanceEnemyBuildingAccessible = distanceBuilding;
				buildingAccessiblePos           = buildingPos;
			}
		}

		OrderManager::Instance().attack(u, buildingAccessiblePos);
		GUIManager::Instance().drawTextOnScreen(u, "attacking accessible building", 48);
		// DebugMessenger::Instance() << "attacking accessible building" <<
		// std::endl;
	}
}

void ArmyManager::zerglingScoutingBeforeBaseFound(BWAPI::Unit u)
{
	auto scoutingOptions    = InformationManager::Instance().scoutingOptions;
	auto scoutedPositions   = InformationManager::Instance().scoutedPositions;
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;

	if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
		auto tp1                       = scoutingOptions.begin()->startToP1ToP2[1];
		auto p1                        = getBasePos(tp1);
		const bool firstOptionScouted  = scoutedPositions.find(p1) != scoutedPositions.end();
		auto tp2                       = scoutingOptions.begin()->startToP1ToP2[2];
		auto p2                        = getBasePos(tp2);
		const bool secondOptionScouted = scoutedPositions.find(p2) != scoutedPositions.end();
		std::stringstream ss;
		if (!firstOptionScouted) {
			OrderManager::Instance().move(u, p1);
			ss << " moving to 1: " << tp1.x << "," << tp1.y << "TP";
			// DebugMessenger::Instance() << " moving to 1: " << tp1.x << "," <<
			// tp1.y << "TP" << std::endl;
		} else if (!secondOptionScouted) {
			OrderManager::Instance().move(u, p2);
			ss << " moving to 2: " << tp2.x << "," << tp2.y << "TP";
			// DebugMessenger::Instance() << " moving to 2: " << tp2.x << "," <<
			// tp2.y << "TP" << std::endl;
		} else {
			auto p = *unscoutedPositions.begin();
			OrderManager::Instance().move(u, p);
			auto tp3 = (TilePosition)p;
			ss << " moving to else: " << tp3.x << "," << tp3.y << "TP";
			// DebugMessenger::Instance() << " moving to else: " << tp3.x << ","
			// << tp3.y << "TP" << std::endl;
		}
		GUIManager::Instance().drawTextOnScreen(u, ss.str(), 480);
	} else { // map size isn't 4, so use old scouting
		auto p = *unscoutedPositions.begin();
		OrderManager::Instance().move(u, p);
	}
}

void ArmyManager::zerglingScoutSpreadOut(BWAPI::Unit u)
{
	auto enemyBuildings     = InformationManager::Instance().enemyBuildings;
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;
	if (scoutLocationsZergling.empty()) {
		auto enemyBuildingsAccessible = getZerglingAccessibleBuildings(u);

		for (auto building : enemyBuildingsAccessible) {
			Position buildingPos = building.getPosition();
			scoutLocationsZergling.push_front(buildingPos);
		}

		for (const auto& unscoutedLocation : unscoutedPositions) {
			scoutLocationsZergling.push_back(unscoutedLocation);
		}

		auto zerglingRegion = BWTA::getRegion(u->getPosition());
		// TODO: don't add duplicate cases
		for (const auto& base : BWTA::getBaseLocations()) {
			auto region = base->getRegion();
			if (!isReachable(zerglingRegion, region)) {
				continue;
			}
			scoutLocationsZergling.push_back(base->getPosition());
		}

		for (const auto& region : BWTA::getRegions()) {
			if (!isReachable(zerglingRegion, region)) {
				continue;
			}
			auto& poly = region->getPolygon();
			for (size_t j = 0; j < poly.size(); ++j) {
				Position point1 = poly[j];
				if (region == BWTA::getRegion(u->getPosition())) {
					scoutLocationsZergling.push_front(point1);
				} else {
					scoutLocationsZergling.push_back(point1);
				}
			}
		}

		// deque iterators NOT valid after insertion
		scoutLocationIndex = scoutLocationsZergling.begin();
	} else {
		GUIManager::Instance().drawTextOnScreen(u, "Zergling Scouting!", 48);
		Position p = *scoutLocationIndex;
		OrderManager::Instance().move(u, p);
		incrementScoutLocationZerglingIndex();
	}
}

void ArmyManager::incrementScoutLocationZerglingIndex()
{
	scoutLocationIndex++;
	if (scoutLocationIndex == scoutLocationsZergling.end()) {
		scoutLocationIndex = scoutLocationsZergling.begin();
	}
}
