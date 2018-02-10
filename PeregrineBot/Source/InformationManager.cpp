#include "InformationManager.h"

#include "OrderManager.h"

using namespace BWAPI;
using namespace Filter;

InformationManager::InformationManager()
{
}

InformationManager& InformationManager::Instance()
{
	static InformationManager instance;
	return instance;
}

void InformationManager::Setup()
{
	auto enemy = Broodwar->enemy();
	if (enemy) {
		auto race = enemy->getRace();
		if (race == Races::Unknown) {
			isEnemyRaceRandom = true;
			DebugMessenger::Instance() << "Enemy is Random" << std::endl;
		} else {
			enemyRace = race;
		}
	} else {
		// this is required as there was a crash when one bot leaves
		// during the game lobby countdown.
		DebugMessenger::Instance() << "Err: no enemy" << std::endl;
	}

	bool isIslandsOnMap = false;
	for (auto bl : BWTA::getBaseLocations()) {
		if (bl->isIsland()) {
			isIslandsOnMap = true;
			DebugMessenger::Instance() << "Islands on map!" << std::endl;
			break;
		}
	}

	SetupScouting();

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1) {
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2) {
			auto base1tp = *iter1;
			auto base2tp = *iter2;
			auto base1   = GetBasePos(base1tp);
			auto base2   = GetBasePos(base2tp);
			auto dist    = DistanceAir(base1, base2);
			if (dist > maxBaseToBaseDistance) {
				maxBaseToBaseDistance = dist;
			}
		}
	}
	DebugMessenger::Instance() << "max base to base is: " << maxBaseToBaseDistance << "P" << std::endl;
}

void InformationManager::SetupScouting()
{
	std::set<Unit> overlords;
	for (Unit u : Broodwar->self()->getUnits()) {
		if (u->getType() == UnitTypes::Zerg_Overlord)
			overlords.insert(u);
	}

	TilePosition airOrigin;
	if (overlords.size() == 1) {
		airOrigin = (TilePosition)(*overlords.begin())->getPosition();
	} else {
		airOrigin = Broodwar->self()->getStartLocation();
		DebugMessenger::Instance() << "Not exactly 1 Overlord at start?!" << std::endl;
	}

	for (TilePosition p : Broodwar->getStartLocations()) {
		if (p == Broodwar->self()->getStartLocation())
			continue;
		auto dist_ground = DistanceGround(Broodwar->self()->getStartLocation(), p);
		auto dist_air    = DistanceAir(airOrigin, p);
		auto time_ground = TimeGround(Broodwar->self()->getStartLocation(), p);
		auto time_air    = TimeAir(airOrigin, p);
		auto metric_dist = dist_ground - dist_air;
		auto metric_time = time_ground - time_air;

		std::array<double, 6> arr = { dist_ground, dist_air, metric_dist, time_ground, time_air, metric_time };

		scoutingInfo.insert(std::pair<TilePosition, std::array<double, 6>>(p, arr));
	}

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1) {
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2) {
			std::set<TilePosition, sortByMostTopThenLeft> zerglingLink = { *iter1, *iter2 };
			double zerglingDist                                        = DistanceGround(*iter1, *iter2);
			double zerglingTime                                        = TimeGround(*iter1, *iter2);

			// calculate DistanceAir from firstOverlordPosition
			TilePosition tp1, tp2;
			if (*iter1 == Broodwar->self()->getStartLocation()) {
				tp1 = airOrigin;
			} else {
				tp1 = *iter1;
			}

			if (*iter2 == Broodwar->self()->getStartLocation()) {
				tp2 = airOrigin;
			} else {
				tp2 = *iter2;
			}

			std::set<TilePosition, sortByMostTopThenLeft> overlordLink = { *iter1, *iter2 };
			double overlordDist                                        = DistanceAir(tp1, tp2);
			double overlordTime                                        = TimeAir(tp1, tp2);

			distAndTime zerglingDnT = { zerglingDist, zerglingTime };
			distAndTime overlordDnT = { overlordDist, overlordTime };

			zerglingNetwork.insert(std::make_pair(zerglingLink, zerglingDnT));
			overlordNetwork.insert(std::make_pair(overlordLink, overlordDnT));
		}
	}

	int nodes       = Broodwar->getStartLocations().size();
	int networkSize = nodes * (nodes - 1) / 2;
	DebugMessenger::Instance() << "Network size from maths = " << networkSize << std::endl;

	if (zerglingNetwork.size() != networkSize || overlordNetwork.size() != networkSize) {
		DebugMessenger::Instance() << "Network size does not match maths." << std::endl;
	}
	std::map<std::array<TilePosition, 3>, std::array<double, 3>> scoutingNetwork;

	auto allStartsList = Broodwar->getStartLocations();
	//allStarts(allStartsList.begin(), allStartsList.end());
	std::copy(allStartsList.begin(), allStartsList.end(), std::inserter(allStarts, allStarts.end()));
	//otherStarts(allStarts);
	std::copy(allStarts.begin(), allStarts.end(), std::inserter(otherStarts, otherStarts.end()));
	otherStarts.erase(Broodwar->self()->getStartLocation());
	//unscoutedPositions(otherStarts);
	//std::copy(otherStarts.begin(), otherStarts.end(), std::inserter(unscoutedPositions, unscoutedPositions.end()));
	for (auto otherStart : otherStarts) {
		unscoutedPositions.insert(GetBasePos(otherStart));
	}

	DebugMessenger::Instance() << allStarts.size() << " starts / " << otherStarts.size() << " otherstarts" << std::endl;
	if (Broodwar->getStartLocations().size() < 4)
		DebugMessenger::Instance() << "less than 4 start positions" << std::endl;

	for (TilePosition p1 : otherStarts) {
		std::set<TilePosition, sortByMostTopThenLeft> startToP1 = { Broodwar->self()->getStartLocation(), p1 };
		DebugMessenger::Instance() << "ad" << overlordNetwork.find(startToP1)->second.distance << "P,   at" << overlordNetwork.find(startToP1)->second.time << "F" << std::endl;

		if (Broodwar->getStartLocations().size() == 4) {
			for (TilePosition p2 : otherStarts) {
				if (p2 == p1)
					continue;
				// tp1 is any position that is not my start position,
				// tp2 is any position that is not start position or tp1

				std::set<TilePosition> remainingPlaces(otherStarts);
				remainingPlaces.erase(p1);
				remainingPlaces.erase(p2);
				if (remainingPlaces.size() != 1) {
					DebugMessenger::Instance() << "remaining places not equal to 1" << std::endl;

					continue;
				}

				std::set<TilePosition, sortByMostTopThenLeft> startToOther = { Broodwar->self()->getStartLocation(), *remainingPlaces.begin() };
				std::set<TilePosition, sortByMostTopThenLeft> p1ToP2       = { p1, p2 };
				std::array<TilePosition, 3> startToP1ToP2                  = { Broodwar->self()->getStartLocation(), p1, p2 };

				double poolDone            = 2437;
				double zerglingTimeStartP1 = zerglingNetwork.find(startToP1)->second.time
				    + poolDone + (double)UnitTypes::Zerg_Zergling.buildTime();
				double zerglingTimeP1P2    = zerglingTimeStartP1 + zerglingNetwork.find(p1ToP2)->second.time;
				double overLordTimeToOther = overlordNetwork.find(startToOther)->second.time;
				double meanTime            = (overLordTimeToOther + zerglingTimeStartP1 + zerglingTimeP1P2) / 3;
				double stdDev              = sqrt((pow(overLordTimeToOther - meanTime, 2) + pow(zerglingTimeStartP1 - meanTime, 2) + pow(zerglingTimeP1P2 - meanTime, 2)) / 3);

				ScoutingOptionFor4 scoutingOption;
				scoutingOption.airTimeFromStartToOther     = overLordTimeToOther;
				scoutingOption.startToP1ToP2               = startToP1ToP2;
				scoutingOption.POther                      = *remainingPlaces.begin();
				scoutingOption.groundTimeFromStartToP1ToP2 = { { zerglingTimeStartP1, zerglingTimeP1P2 } };
				scoutingOption.maxTime                     = std::max(overLordTimeToOther, zerglingTimeP1P2);
				scoutingOption.meanTime                    = meanTime;
				scoutingOption.stdDev                      = stdDev;

				scoutingOptions.insert(scoutingOption);
			}
		}
	}
}

void InformationManager::Update()
{
	if (enemyRace == Races::Unknown) {
		auto enemy = Broodwar->enemy();
		if (enemy) {
			auto race = enemy->getRace();
			if ((race == Races::Terran) || (race == Races::Zerg) || (race == Races::Protoss)) {
				enemyRace = race;
				DebugMessenger::Instance() << "Enemy is " << enemyRace.c_str() << std::endl;
			}
		}
	}

	validateEnemyUnits();

	UpdateScouting();
}

void InformationManager::UpdateScouting()
{
	for (auto p : unscoutedPositions) {
		if (Broodwar->isVisible(TilePosition(p))) {
			scoutedPositions.insert(p);
			unscoutedPositions.erase(p);
			if (!isEnemyBaseFound) {
				// replace IsBuilding by IsResourceDepot?
				if (Broodwar->getUnitsOnTile(TilePosition(p),
				                             IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
				        .size()
				    > 0) {
					enemyBase          = p;
					isEnemyBaseDeduced = true;
					isEnemyBaseFound   = true;
					DebugMessenger::Instance() << "Found enemy base at: " << Broodwar->getFrameCount() << "F" << std::endl;
					if ((enemyBase.x == 0) && (enemyBase.y == 0)) {
						Broodwar << "ERR: Found enemy base at 0,0P" << std::endl;
					}
				}
			}
		}
	}

	if (!(isEnemyBaseDeduced || isEnemyBaseFound) && unscoutedPositions.size() == 1) {
		isEnemyBaseDeduced   = true;
		BWAPI::Position base = (*unscoutedPositions.begin());
		DebugMessenger::Instance() << "Enemy base deduced to be at: " << base.x << ", " << base.y << "P" << std::endl;
	}
}

void InformationManager::OverlordScouting(BWAPI::Unit overlord)
{
	if (overlord->isUnderAttack()) { // if overlord is under attack run back to own base
		OverlordRetreatToHome(overlord);
		return;
	}

	if (!isEnemyBaseFound) {
		OverlordScoutingAtGameStart(overlord);
	} else {
		OverlordScoutingAfterBaseFound(overlord);
	}
}

void InformationManager::OverlordScoutingAtGameStart(BWAPI::Unit overlord)
{
	if (overlord->isIdle()) {
		if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
			auto tp                       = scoutingOptions.begin()->POther;
			auto p                        = GetBasePos(tp);
			const bool firstOptionScouted = scoutedPositions.find(p) != scoutedPositions.end();
			if (!firstOptionScouted) {
				OrderManager::Instance().Move(overlord, p, true);
			} else {
				for (auto p2 : boost::adaptors::reverse(unscoutedPositions)) { //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
					if (p2 == p)
						continue;
					OrderManager::Instance().Move(overlord, p2, true);
				}
			}
		} else {                                                          // map size isn't 4, so use old scouting
			for (auto p : boost::adaptors::reverse(unscoutedPositions)) { //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
				OrderManager::Instance().Move(overlord, p, true);
			}
		}
	} else {
		if (!isPastSpottingTime) {
			// maybe make 128 * 1.5 a const "smudge factor" variable
			auto spottingTime = (maxBaseToBaseDistance + 128 * 1.5) / UnitTypes::Zerg_Overlord.topSpeed();
			if (Broodwar->getFrameCount() > spottingTime) {
				isPastSpottingTime = true;
				DebugMessenger::Instance() << "Past Overlord spotting time" << std::endl;
			} else {
				if (!isEnemyBaseFromOverlordSpotting) {
					// overlord spotting of overlords, very naive.
					// only allow "certain" spotting, therefore based on half max base to base distance.
					auto range        = overlord->getType().sightRange() + 32; // ADDING 32 incase the overlord needs more range
					auto unitsSpotted = overlord->getUnitsInRadius(range, IsEnemy && IsVisible && GetType == UnitTypes::Zerg_Overlord);
					std::set<TilePosition> potentialStartsFromSpotting;
					for (auto u : unitsSpotted) {
						auto pO           = u->getPosition();
						auto searchRadius = UnitTypes::Zerg_Overlord.topSpeed() * Broodwar->getFrameCount();
						searchRadius += 128 * 1.5; // add a bit to account for overlord spawning in a different place

						for (auto tp : otherStarts) {
							auto pB          = GetBasePos(tp);
							auto distToStart = DistanceAir(pB, pO);
							if (distToStart < searchRadius) {
								potentialStartsFromSpotting.insert(tp);
							}
						}
					}
					if (potentialStartsFromSpotting.size() == 1) {
						isEnemyBaseFromOverlordSpotting = true;
						auto base                       = *potentialStartsFromSpotting.begin();
						enemyBaseSpottingGuess          = base;
						Broodwar << "Overlord spotted overlord and determined base at: " << base.x << "," << base.y << "TP" << std::endl;
					}
				}
			}
		}
	}
}

void InformationManager::OverlordScoutingAfterBaseFound(BWAPI::Unit overlord)
{
	if (overlord->isIdle()) {
		if (enemyRace != Races::Terran) {
			// Overlord scouting perimeter of all regions
			// Might be more useful to have this as a text hovering over overlord.
			//DebugMessenger::Instance() << "Overlord Scouting!" << std::endl;
			static std::deque<Position> scoutLocations;
			if (scoutLocations.empty()) {
				auto enemyRegion = BWTA::getRegion(enemyBase);
				auto& poly       = enemyRegion->getPolygon();
				for (size_t j = 0; j < poly.size(); ++j) {
					// The points in Polygon appear to be all along the perimeter.
					Position point1 = poly[j];
					scoutLocations.push_back(point1);
				}
				for (const auto& region : BWTA::getRegions()) {
					for (const auto& base : region->getBaseLocations()) {
						Position point1 = base->getPosition();
						scoutLocations.push_back(point1);
					}
				}
			} else {
				auto it              = scoutLocations.begin();
				Position baseToScout = (*it);
				OrderManager::Instance().Move(overlord, baseToScout);
				scoutLocations.erase(it);
			}
		} else { // enemy race is terran, move back to our own base
			OverlordRetreatToHome(overlord);
		}
	}
}

void InformationManager::OverlordRetreatToHome(BWAPI::Unit overlord)
{
	auto ownBasePos          = GetBasePos(Broodwar->self()->getStartLocation());
	auto distanceFromOwnBase = overlord->getDistance(ownBasePos);
	if (distanceFromOwnBase > 128) {
		DebugMessenger::Instance() << "Retreat overlord." << std::endl;
		OrderManager::Instance().Move(overlord, ownBasePos);
	} else {
		//DebugMessenger::Instance() << "Overlord being attacked in base." << std::endl;
		// spamming when enemy race is terran.
	}
}

void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)) {
			addToEnemyBuildings(unit);
		} else {
			addToEnemyArmy(unit);
		}
	}
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)) {
			removeFromEnemyBuildings(unit);
			if (((IsResourceDepot)(unit) == true) && (unit->getPosition() == enemyBase)) {
				isEnemyBaseDestroyed = true;
				DebugMessenger::Instance() << "destroyed enemy base: " << Broodwar->getFrameCount() << std::endl;
			}
		} else {
			removeFromEnemyArmy(unit);
		}
	}
}

void InformationManager::onUnitMorph(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)) {
			addToEnemyBuildings(unit);
			removeFromEnemyArmy(unit);
		} else {
			addToEnemyArmy(unit);
			removeFromEnemyBuildings(unit);
		}
	}
}

void InformationManager::addToEnemyBuildings(BWAPI::Unit unit)
{
	auto iterAndBool = enemyBuildings.emplace(unit);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second) {
		iterAndBool.first->update();
	}
}

void InformationManager::addToEnemyArmy(BWAPI::Unit unit)
{
	auto iterAndBool = enemyArmy.emplace(unit);

	// if unit already exists in enemyArmy
	if (!iterAndBool.second) {
		iterAndBool.first->update();
	}
}

void InformationManager::removeFromEnemyBuildings(UnitInfo unit)
{
	enemyBuildings.erase(unit);
}

void InformationManager::removeFromEnemyArmy(UnitInfo unit)
{
	enemyArmy.erase(unit);
}

void InformationManager::validateEnemyUnits()
{
	for (auto iter = enemyBuildings.begin(); iter != enemyBuildings.end(); iter++) {
		if (iter->exists()) {
			if ((!IsBuilding || !IsEnemy)(iter->u)) {
				removeFromEnemyBuildings(*iter);
				DebugMessenger::Instance() << "remove enemy building on validation" << std::endl;
			}
		}
	}

	for (auto iter = enemyArmy.begin(); iter != enemyArmy.end(); iter++) {
		auto unit = *iter;
		if (iter->exists()) {
			if ((IsBuilding || !IsEnemy)(iter->u)) {
				removeFromEnemyArmy(*iter);
				DebugMessenger::Instance() << "remove enemy army on validation" << std::endl;
			}
		}
	}
}
