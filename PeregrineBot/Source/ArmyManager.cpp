#include "ArmyManager.h"

using namespace BWAPI;
using namespace Filter;

ArmyManager::ArmyManager()
{
}

ArmyManager& ArmyManager::Instance()
{
	static ArmyManager instance;
	return instance;
}

void ArmyManager::ZerglingScout(Unit u)
{
	auto enemyBuildings = InformationManager::Instance().enemyBuildings;
	static std::deque<Position> ZerglingScoutLocations;
	if (ZerglingScoutLocations.empty()) {
		for (const auto& region : BWTA::getRegions()) {
			// if region isn't reachable then skip
			if (!BWTA::getRegion(u->getPosition())->isReachable(region)) {
				continue;
			}
			BWTA::Polygon poly = region->getPolygon();
			auto it            = ZerglingScoutLocations.begin();
			for (size_t j = 0; j < poly.size(); ++j) {
				Position point1 = poly[j];
				if (region == BWTA::getRegion(u->getPosition())) {
					it = ZerglingScoutLocations.insert(it, point1);
					//it++;
				} else {
					ZerglingScoutLocations.push_back(point1);
				}
			}
		}
		for (const auto& building : enemyBuildings) {
			Position buildingPos = building->getPosition();
			// if building isn't reachable then skip
			if (!BWTA::getRegion(u->getPosition())->isReachable(BWTA::getRegion(buildingPos))) {
				DebugMessenger::Instance() << "unaccessible building" << std::endl;
				continue;
			}
			DebugMessenger::Instance() << "scoutable building" << std::endl;
			auto it = ZerglingScoutLocations.begin();
			it      = ZerglingScoutLocations.insert(it, buildingPos);
		}
	} else {
		DebugMessenger::Instance() << "Zergling Scouting!" << std::endl;

		auto it                 = ZerglingScoutLocations.begin();
		Position perimeterPoint = (*it);
		OrderManager::Instance().Move(u, perimeterPoint);
		ZerglingScoutLocations.erase(it);
	}
}

void ArmyManager::ZerglingAttack(Unit u)
{
	auto enemyBase          = InformationManager::Instance().enemyBase;
	auto reachEnemyBase     = InformationManager::Instance().reachEnemyBase;
	auto enemyRace          = InformationManager::Instance().enemyRace;
	auto destroyEnemyBase   = InformationManager::Instance().destroyEnemyBase;
	auto scoutingOptions    = InformationManager::Instance().scoutingOptions;
	auto scoutedPositions   = InformationManager::Instance().scoutedPositions;
	auto unscoutedPositions = InformationManager::Instance().unscoutedPositions;
	if ((enemyBase.x != 0) && (enemyBase.y != 0)) {
		if ((!reachEnemyBase) && (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(enemyBase))) {
			InformationManager::Instance().reachEnemyBase = true;
			DebugMessenger::Instance() << "reach enemy base: " << Broodwar->getFrameCount() << std::endl;
		}
	}
	//Broodwar->setLocalSpeed(23);
	if (enemyRace == Races::Protoss) { // if protoss - enemy then pylon then worker
		Unit enemy = u->getClosestUnit(
		    IsEnemy && (GetType == UnitTypes::Zerg_Zergling || GetType == UnitTypes::Terran_Marine || GetType == UnitTypes::Protoss_Zealot || GetType == UnitTypes::Zerg_Sunken_Colony || GetType == UnitTypes::Terran_Bunker || GetType == UnitTypes::Protoss_Photon_Cannon));
		Unit supply = u->getClosestUnit(
		    IsEnemy && (GetType == UnitTypes::Protoss_Pylon || GetType == UnitTypes::Terran_Supply_Depot));
		Unit worker      = u->getClosestUnit(IsEnemy && IsWorker);
		Unit enemy_atall = u->getClosestUnit(IsEnemy);
		Unit target      = NULL;
		if (enemy) {
			target = enemy;
		} else if (supply) {
			target = supply;
		} else if (worker) {
			target = worker;
		} else if (enemy_atall) {
			target = enemy_atall;
		}
		if (target) {
			OrderManager::Instance().Attack(u, target);
		}
	}      // end if protoss
	else { // not protoss - enemy and worker then supply
		Unit enemy = u->getClosestUnit(
		    IsEnemy && (GetType == UnitTypes::Zerg_Zergling || GetType == UnitTypes::Terran_Marine || GetType == UnitTypes::Protoss_Zealot || GetType == UnitTypes::Zerg_Sunken_Colony || GetType == UnitTypes::Terran_Bunker || GetType == UnitTypes::Protoss_Photon_Cannon || GetType == UnitTypes::Terran_Firebat || IsWorker));
		Unit supply = u->getClosestUnit(
		    IsEnemy && (GetType == UnitTypes::Protoss_Pylon || GetType == UnitTypes::Terran_Supply_Depot));
		Unit enemy_atall = u->getClosestUnit(IsEnemy);
		Unit target      = NULL;
		if (enemy) {
			target = enemy;
		} else if (supply) {
			target = supply;
		} else if (enemy_atall) {
			target = enemy_atall;
		}
		if (target) {
			OrderManager::Instance().Attack(u, target);
		}
	}

	//Unit neutral = u->getClosestUnit(IsNeutral && IsBuilding, 5);
	//if (neutral) {		// remove potential blockages
	//	u->attack(PositionOrUnit(neutral));
	//	Broodwar << "attacking neutral" << std::endl;
	//}

	if (u->isIdle()) {
		Unit enemy_atall = u->getClosestUnit(IsEnemy);
		if (enemy_atall) {
			OrderManager::Instance().Attack(u, enemy_atall);
		} else {
			if ((enemyBase.x != 0) && (enemyBase.y != 0)) {
				if (!destroyEnemyBase) {
					if (!Broodwar->isVisible(TilePosition(enemyBase))) {
						OrderManager::Instance().Attack(u, enemyBase);
					} else if (!Broodwar->getUnitsOnTile(TilePosition(enemyBase), IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted).empty()) {
						OrderManager::Instance().Attack(u, enemyBase);
					} else {
						ZerglingScout(u);
					}
				} // havent destroyed enemy base
				else {
					ZerglingScout(u);
				}
			} // no enemy base
			else {
				if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
					auto tp1                       = scoutingOptions.begin()->startToP1ToP2[1];
					auto p1                        = getBasePos(tp1);
					const bool firstOptionScouted  = scoutedPositions.find(p1) != scoutedPositions.end();
					auto tp2                       = scoutingOptions.begin()->startToP1ToP2[2];
					auto p2                        = getBasePos(tp2);
					const bool secondOptionScouted = scoutedPositions.find(p2) != scoutedPositions.end();
					if (!firstOptionScouted) {
						OrderManager::Instance().Move(u, p1);
						DebugMessenger::Instance() << " moving to tp1" << tp1.x << "," << tp1.y << std::endl;
					} else if (!secondOptionScouted) {
						OrderManager::Instance().Move(u, p2);
						DebugMessenger::Instance() << " moving to tp2" << tp2.x << "," << tp2.y << std::endl;
					} else {
						auto p = *unscoutedPositions.begin();
						OrderManager::Instance().Move(u, p);
						auto tp3 = (TilePosition)p;
						DebugMessenger::Instance() << " moving to else" << tp3.x << "," << tp3.y << std::endl;
					}
				} else { // map size isn't 4, so use old scouting
					auto p = *unscoutedPositions.begin();
					OrderManager::Instance().Move(u, p);
					//move(u, (*unscoutedOtherPositions.begin()));
					/*for (auto p : unscoutedOtherPositions) {
						u->move(p, true);
						}*/
				}
			}
		}
	} // end if idle
	else if (u->isMoving()) {
		UnitCommand lastCmd = u->getLastCommand();
		if (lastCmd.getType() == UnitCommandTypes::Move) {
			Position targetPos = lastCmd.getTargetPosition();
			//if (unscoutedOtherPositions.find(targetPos) == unscoutedOtherPositions.end()) {
			if ((unscoutedPositions.count(targetPos) == 0) && (!unscoutedPositions.empty())) {
				DebugMessenger::Instance() << "recalculate scouting" << std::endl;

				u->stop();
				auto p = *unscoutedPositions.begin();
				OrderManager::Instance().Move(u, p);
				/*for (auto p : unscoutedOtherPositions) {
					u->move(p, true);
					}*/
			}
		}
	} // end if moving
}
