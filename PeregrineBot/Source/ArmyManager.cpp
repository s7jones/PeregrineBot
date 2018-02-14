#include "ArmyManager.h"

#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "Utility.h"
#include "UtilityManager.h"

using namespace BWAPI;
using namespace Filter;

ArmyManager::ArmyManager() {}

ArmyManager& ArmyManager::Instance()
{
	static ArmyManager instance;
	return instance;
}

void ArmyManager::ZerglingAttack(BWAPI::Unit u)
{
	auto enemyMain                       = InformationManager::Instance().enemyMain;
	auto enemyRace                       = InformationManager::Instance().enemyRace;
	auto unscoutedPositions              = InformationManager::Instance().unscoutedPositions;
	auto isEnemyBaseFromOverlordSpotting = InformationManager::Instance().isEnemyBaseFromOverlordSpotting;
	auto isEnemyBaseDeduced              = InformationManager::Instance().isEnemyBaseDeduced;
	auto isEnemyBaseReached              = InformationManager::Instance().isEnemyBaseReached;
	auto isEnemyBaseDestroyed            = InformationManager::Instance().isEnemyBaseDestroyed;
	auto enemyBaseSpottingGuess          = InformationManager::Instance().enemyBaseSpottingGuess;
	auto enemyBuildings                  = InformationManager::Instance().enemyBuildings;

	if (enemyMain) {
		if ((!isEnemyBaseReached) && (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(enemyMain->getPosition()))) {
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

	if (!priorityTarget) {
		if (u->isIdle()) {
			Unit closestGroundEnemy = u->getClosestUnit(IsEnemy && !IsFlying);
			if (closestGroundEnemy) {
				OrderManager::Instance().Attack(u, closestGroundEnemy);
			} else {
				if (enemyMain) {
					if (!isEnemyBaseDestroyed) {
						if (!Broodwar->isVisible(TilePosition(enemyMain->getPosition()))) {
							OrderManager::Instance().Attack(u, *enemyMain);
						} else if (!Broodwar->getUnitsOnTile(
						                        TilePosition(enemyMain->getPosition()),
						                        IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
						                .empty()) {
							OrderManager::Instance().Attack(u, *enemyMain);
						} else if (enemyBuildings.size() != 0) {
							ZerglingAttackKnownBuildings(u);
						} else {
							ZerglingScoutSpreadOut(u);
						}
					} else if (enemyBuildings.size() != 0) {
						ZerglingAttackKnownBuildings(u);
					} else {
						ZerglingScoutSpreadOut(u);
					}
				} else {
					if (enemyBuildings.size() != 0) {
						ZerglingAttackKnownBuildings(u);
					} else if (isEnemyBaseFromOverlordSpotting) {
						OrderManager::Instance().Move(u, enemyBaseSpottingGuess);
						DebugMessenger::Instance() << "scout overlord spot" << std::endl;
					} else {
						ZerglingScoutingBeforeBaseFound(u);
					}
				}
			}
		} else if (u->isMoving()) { // attack move is most likely not covered here
			UnitCommand lastCmd = u->getLastCommand();
			if (lastCmd.getType() == UnitCommandTypes::Move) {
				Position targetPos = lastCmd.getTargetPosition();
				if ((unscoutedPositions.count(targetPos) == 0) && (!unscoutedPositions.empty())
				    && (std::find(scoutLocationsZergling.begin(), scoutLocationsZergling.end(), targetPos) == scoutLocationsZergling.end())) {
					auto p = *unscoutedPositions.begin();
					OrderManager::Instance().Move(u, p);
					GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting", 480);
					// DebugMessenger::Instance() << "recalculate scouting" <<
					// std::endl;
				} else if ((!enemyMain) && (isEnemyBaseFromOverlordSpotting)
				           && enemyBaseSpottingGuess != targetPos) {
					OrderManager::Instance().Move(u, enemyBaseSpottingGuess);
					GUIManager::Instance().drawTextOnScreen(u, "recalculate scouting (overlord guess)", 480);
					// DebugMessenger::Instance() << "recalculate scouting
					// (overlord guess)" << std::endl;
				}
			}
		}
	}
}

std::set<EnemyUnitInfo> ArmyManager::GetZerglingAccessibleBuildings(BWAPI::Unit u)
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

void ArmyManager::ZerglingAttackKnownBuildings(BWAPI::Unit u)
{
	auto enemyBuildingsAccessible = GetZerglingAccessibleBuildings(u);

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

	if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
		auto tp1                       = scoutingOptions.begin()->startToP1ToP2[1];
		auto p1                        = getBasePos(tp1);
		const bool firstOptionScouted  = scoutedPositions.find(p1) != scoutedPositions.end();
		auto tp2                       = scoutingOptions.begin()->startToP1ToP2[2];
		auto p2                        = getBasePos(tp2);
		const bool secondOptionScouted = scoutedPositions.find(p2) != scoutedPositions.end();
		std::stringstream ss;
		if (!firstOptionScouted) {
			OrderManager::Instance().Move(u, p1);
			ss << " moving to 1: " << tp1.x << "," << tp1.y << "TP";
			// DebugMessenger::Instance() << " moving to 1: " << tp1.x << "," <<
			// tp1.y << "TP" << std::endl;
		} else if (!secondOptionScouted) {
			OrderManager::Instance().Move(u, p2);
			ss << " moving to 2: " << tp2.x << "," << tp2.y << "TP";
			// DebugMessenger::Instance() << " moving to 2: " << tp2.x << "," <<
			// tp2.y << "TP" << std::endl;
		} else {
			auto p = *unscoutedPositions.begin();
			OrderManager::Instance().Move(u, p);
			auto tp3 = (TilePosition)p;
			ss << " moving to else: " << tp3.x << "," << tp3.y << "TP";
			// DebugMessenger::Instance() << " moving to else: " << tp3.x << ","
			// << tp3.y << "TP" << std::endl;
		}
		GUIManager::Instance().drawTextOnScreen(u, ss.str(), 480);
	} else { // map size isn't 4, so use old scouting
		auto p = *unscoutedPositions.begin();
		OrderManager::Instance().Move(u, p);
	}
}

void ArmyManager::ZerglingScoutSpreadOut(BWAPI::Unit u)
{
	auto enemyBuildings     = InformationManager::Instance().enemyBuildings;
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;
	if (scoutLocationsZergling.empty()) {
		auto enemyBuildingsAccessible = GetZerglingAccessibleBuildings(u);

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
	} else {
		GUIManager::Instance().drawTextOnScreen(u, "Zergling Scouting!", 48);
		// DebugMessenger::Instance() << "Zergling Scouting!" << std::endl;

		auto it    = scoutLocationsZergling.begin();
		Position p = (*it);
		OrderManager::Instance().Move(u, p);
		scoutLocationsZergling.erase(it);
	}
}
