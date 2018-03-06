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
	removeEmptySquads();

	for (auto squad : squads) {
		const bool squadNeedsToWait = OrderManager::Instance().doesSquadHaveOrder(squad);
		if (squadNeedsToWait) {
			continue;
		}
		attackWithSquad(squad);
	}
}

void ArmyManager::putUnassignedInSquads()
{
	for (const auto& friendly : InformationManager::Instance().friendlyUnits) {
		if (friendly.getType() != UnitTypes::Zerg_Zergling) {
			continue;
		}

		bool isUnitAssigned = false;

		Squad closestSquad;
		double closestSquadDistance = std::numeric_limits<double>::infinity();

		for (const auto& squad : squads) {
			if (squad.count(friendly.unit)) {
				isUnitAssigned = true;
				break;
			}
		}

		if (!isUnitAssigned) {
			for (const auto& squad : squads) {
				auto distance = distanceAir(friendly.getPosition(), squad.getPosition());
				if (distance < closestSquadDistance) {
					closestSquadDistance = distance;
					closestSquad         = squad;
				}
			}

			if (closestSquadDistance < SQUAD_RADIUS) {
				closestSquad.insert(friendly.unit);
			} else {
				Squad newSquad;
				newSquad.insert(friendly.unit);
				squads.push_back(newSquad);
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
	const auto isEnemyBaseReached      = InformationManager::Instance().isEnemyBaseReached;
	const auto isEnemyBaseDestroyed    = InformationManager::Instance().isEnemyBaseDestroyed;
	const auto enemyBaseSpottingGuess  = InformationManager::Instance().enemyBaseSpottingGuess;
	const auto enemyBuildings          = InformationManager::Instance().enemyBuildings;
	const auto enemyArmy               = InformationManager::Instance().enemyArmy;

	for (const auto unit : squad) {
		if (enemyMain.unit) {
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

	const bool priorityTarget = UtilityManager::Instance().getBestActionForSquad(squad);

	if (priorityTarget) {
		return;
	}

	auto idleState   = squad.isIdle();
	auto movingState = squad.isMoving();

	if (idleState == Squad::ALLIDLE) {
		Unit closestGroundEnemy = squad.getClosestUnit(IsEnemy && !IsFlying);
		if (closestGroundEnemy) {
			OrderManager::Instance().attack(squad, closestGroundEnemy);
		} else {
			if (enemyMain.unit) {
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
		if (idleState == Squad::SOMEIDLE) {
			// reassign idle members
			errorMessage("some (but not all or none) in squad idle");
		} else {
			if (movingState == Squad::ALLMOVING) { // attack move is most likely not covered here
				UnitCommand lastCmd = (*squad.begin())->getLastCommand();
				if (lastCmd.getType() == UnitCommandTypes::Move) {
					Position targetPos = lastCmd.getTargetPosition();
					if (!enemyMain.unit) {
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
							                       return lastCmd.getUnit() == enemy.unit;
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
				if (movingState == Squad::SOMEMOVING) {
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
	std::vector<Squad> idleSquadList;

	for (auto squad : squads) {
		auto it = squad.begin();
		while (squad.size() > 1 && it != squad.end()) {
			auto unit = (*it);
			if (unit->isIdle()) {
				Squad idleSquad;
				idleSquad.insert(unit);
				idleSquadList.push_back(idleSquad);
				it = squad.erase(it);
			} else {
				it++;
			}
		}
	}

	squads.insert(squads.end(), idleSquadList.begin(), idleSquadList.end());
}

void ArmyManager::removeEmptySquads()
{
	squads.erase(std::remove_if(squads.begin(),
	                            squads.end(),
	                            [](Squad& squad) -> bool {
		                            return squad.empty();
	                            }),
	             squads.end());
}

void ArmyManager::zerglingAttack(BWAPI::Unit unit)
{
	const auto enemyMain               = InformationManager::Instance().enemyMain;
	const auto enemyRace               = InformationManager::Instance().enemyRace;
	const auto unscoutedPositions      = InformationManager::Instance().unscoutedPositions;
	const auto isEnemyBaseFromSpotting = InformationManager::Instance().isEnemyBaseFromSpotting;
	const auto isEnemyBaseReached      = InformationManager::Instance().isEnemyBaseReached;
	const auto isEnemyBaseDestroyed    = InformationManager::Instance().isEnemyBaseDestroyed;
	const auto enemyBaseSpottingGuess  = InformationManager::Instance().enemyBaseSpottingGuess;
	const auto enemyBuildings          = InformationManager::Instance().enemyBuildings;
	const auto enemyArmy               = InformationManager::Instance().enemyArmy;

	if (enemyMain.unit) {
		if ((!isEnemyBaseReached) && (BWTA::getRegion(unit->getPosition()) == BWTA::getRegion(enemyMain.getPosition()))) {
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

	const bool priorityTarget = UtilityManager::Instance().getBestActionForZergling(unit);

	if (priorityTarget) {
		return;
	}

	if (unit->isIdle()) {
		Unit closestGroundEnemy = unit->getClosestUnit(IsEnemy && !IsFlying);
		if (closestGroundEnemy) {
			OrderManager::Instance().attack(unit, closestGroundEnemy);
		} else {
			if (enemyMain.unit) {
				if (!isEnemyBaseDestroyed) {
					if (!Broodwar->isVisible(TilePosition(enemyMain.getPosition()))) {
						OrderManager::Instance().attack(unit, enemyMain);
					} else if (!Broodwar->getUnitsOnTile(
					                        TilePosition(enemyMain.getPosition()),
					                        IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
					                .empty()) {
						OrderManager::Instance().attack(unit, enemyMain);
					} else if (enemyBuildings.size() != 0) {
						zerglingAttackKnownBuildings(unit);
					} else {
						zerglingScoutSpreadOut(unit);
					}
				} else if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(unit);
				} else {
					zerglingScoutSpreadOut(unit);
				}
			} else {
				if (enemyBuildings.size() != 0) {
					zerglingAttackKnownBuildings(unit);
				} else if (isEnemyBaseFromSpotting) {
					OrderManager::Instance().move(unit, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(unit, "scout overlord spot", 480);
				} else {
					zerglingScoutingBeforeBaseFound(unit);
				}
			}
		}
	} else if (unit->isMoving()) { // attack move is most likely not covered here
		UnitCommand lastCmd = unit->getLastCommand();
		if (lastCmd.getType() == UnitCommandTypes::Move) {
			Position targetPos = lastCmd.getTargetPosition();
			if (!enemyMain.unit) {
				if (isEnemyBaseFromSpotting
				    && enemyBaseSpottingGuess != targetPos) {
					OrderManager::Instance().move(unit, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(unit, "recalculate scouting (overlord guess)", 480);
				}
				// if moving to somewhere already scouted
				else if (unscoutedPositions.count(targetPos) == 0) {
					if (!unscoutedPositions.empty()) {
						auto p = *unscoutedPositions.begin();
						OrderManager::Instance().move(unit, p);
						GUIManager::Instance().drawTextOnScreen(unit, "recalculate scouting 1", 480);
					}
					// if moving to somewhere not in zergScoutLocations and there are places to go
					else if ((std::find(scoutLocationsZergling.begin(), scoutLocationsZergling.end(), targetPos)
					          == scoutLocationsZergling.end())
					         && !scoutLocationsZergling.empty()) {
						auto p = *scoutLocationIndex;
						OrderManager::Instance().move(unit, p);
						GUIManager::Instance().drawTextOnScreen(unit, "recalculate scouting 2", 480);
						incrementScoutLocationZerglingIndex();
					}
				}
			}
		} else if (lastCmd.getType() == UnitCommandTypes::Attack_Unit) {
			using EnemyContainer            = std::set<EnemyUnitInfo>;
			auto lastCommandUnitInContainer = [lastCmd](EnemyContainer container) -> bool {
				auto it = std::find_if(container.begin(), container.end(),
				                       [lastCmd](const EnemyUnitInfo& enemy) -> bool {
					                       return lastCmd.getUnit() == enemy.unit;
				                       });
				return it != container.end();
			};

			if (!lastCommandUnitInContainer(enemyBuildings)
			    && !lastCommandUnitInContainer(enemyArmy)) {
				errorMessage("unit not in enemy containers, stopping.");
				OrderManager::Instance().stop(unit);
			}
		}
	}
}

std::set<EnemyUnitInfo> ArmyManager::getZerglingAccessibleBuildings(BWAPI::Unit unit)
{
	const auto enemyBuildings = InformationManager::Instance().enemyBuildings;
	std::set<EnemyUnitInfo> enemyBuildingsAccessible;
	for (const auto& building : enemyBuildings) {
		auto buildingPos = building.getPosition();
		auto zerglingPos = unit->getPosition();

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

void ArmyManager::zerglingAttackKnownBuildings(BWAPI::Unit unit)
{
	auto enemyBuildingsAccessible = getZerglingAccessibleBuildings(unit);

	if (enemyBuildingsAccessible.size() != 0) {

		double distanceEnemyBuildingAccessible = std::numeric_limits<double>::infinity();
		Position buildingAccessiblePos;

		for (const auto& building : enemyBuildingsAccessible) {
			Position buildingPos    = building.getPosition();
			double distanceBuilding = distanceAir(unit->getPosition(), buildingPos);
			if (distanceBuilding < distanceEnemyBuildingAccessible) {
				distanceEnemyBuildingAccessible = distanceBuilding;
				buildingAccessiblePos           = buildingPos;
			}
		}

		OrderManager::Instance().attack(unit, buildingAccessiblePos);
		GUIManager::Instance().drawTextOnScreen(unit, "attacking accessible building", 48);
		// DebugMessenger::Instance() << "attacking accessible building" <<
		// std::endl;
	}
}

void ArmyManager::zerglingScoutingBeforeBaseFound(BWAPI::Unit unit)
{
	const auto scoutingOptions    = InformationManager::Instance().scoutingOptions;
	const auto scoutedPositions   = InformationManager::Instance().scoutedPositions;
	const auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;

	if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
		auto tp1                       = scoutingOptions.begin()->startToP1ToP2[1];
		auto p1                        = getBasePos(tp1);
		const bool firstOptionScouted  = scoutedPositions.find(p1) != scoutedPositions.end();
		auto tp2                       = scoutingOptions.begin()->startToP1ToP2[2];
		auto p2                        = getBasePos(tp2);
		const bool secondOptionScouted = scoutedPositions.find(p2) != scoutedPositions.end();
		std::stringstream ss;
		if (!firstOptionScouted) {
			OrderManager::Instance().move(unit, p1);
			ss << " moving to 1: " << tp1.x << "," << tp1.y << "TP";
			// DebugMessenger::Instance() << " moving to 1: " << tp1.x << "," <<
			// tp1.y << "TP" << std::endl;
		} else if (!secondOptionScouted) {
			OrderManager::Instance().move(unit, p2);
			ss << " moving to 2: " << tp2.x << "," << tp2.y << "TP";
			// DebugMessenger::Instance() << " moving to 2: " << tp2.x << "," <<
			// tp2.y << "TP" << std::endl;
		} else {
			auto p = *unscoutedPositions.begin();
			OrderManager::Instance().move(unit, p);
			auto tp3 = (TilePosition)p;
			ss << " moving to else: " << tp3.x << "," << tp3.y << "TP";
			// DebugMessenger::Instance() << " moving to else: " << tp3.x << ","
			// << tp3.y << "TP" << std::endl;
		}
		GUIManager::Instance().drawTextOnScreen(unit, ss.str(), 480);
	} else { // map size isn't 4, so use old scouting
		auto p = *unscoutedPositions.begin();
		OrderManager::Instance().move(unit, p);
	}
}

void ArmyManager::zerglingScoutSpreadOut(BWAPI::Unit unit)
{
	const auto enemyBuildings     = InformationManager::Instance().enemyBuildings;
	const auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;
	if (scoutLocationsZergling.empty()) {
		auto enemyBuildingsAccessible = getZerglingAccessibleBuildings(unit);

		for (const auto& building : enemyBuildingsAccessible) {
			Position buildingPos = building.getPosition();
			scoutLocationsZergling.push_front(buildingPos);
		}

		for (const auto& unscoutedLocation : unscoutedPositions) {
			scoutLocationsZergling.push_back(unscoutedLocation);
		}

		auto zerglingRegion = BWTA::getRegion(unit->getPosition());
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
				if (region == BWTA::getRegion(unit->getPosition())) {
					scoutLocationsZergling.push_front(point1);
				} else {
					scoutLocationsZergling.push_back(point1);
				}
			}
		}

		// deque iterators NOT valid after insertion
		scoutLocationIndex = scoutLocationsZergling.begin();
	} else {
		GUIManager::Instance().drawTextOnScreen(unit, "Zergling Scouting!", 48);
		Position p = *scoutLocationIndex;
		OrderManager::Instance().move(unit, p);
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

void ArmyManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Zerg_Zergling && unit->getPlayer() == Broodwar->self()) {
		for (auto squad : squads) {
			squad.erase(unit);
		}
	}

	removeEmptySquads();
}
