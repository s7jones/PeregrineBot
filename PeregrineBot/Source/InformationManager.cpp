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

void InformationManager::setup()
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
		errorMessage("no enemy");
	}

	for (auto bl : BWTA::getBaseLocations()) {
		if (bl->isIsland()) {
			isIslandsOnMap = true;
			DebugMessenger::Instance() << "Islands on map!" << std::endl;
			break;
		}
	}

	setupScouting();

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1) {
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2) {
			auto base1tp = *iter1;
			auto base2tp = *iter2;
			auto base1   = getBasePos(base1tp);
			auto base2   = getBasePos(base2tp);
			auto dist    = distanceAir(base1, base2);
			if (dist > maxBaseToBaseDistance) {
				maxBaseToBaseDistance = dist;
			}
		}
	}
	DebugMessenger::Instance() << "max base to base is: " << maxBaseToBaseDistance << "P" << std::endl;
}

void InformationManager::setupScouting()
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

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1) {
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2) {
			std::set<TilePosition> zerglingLink = { *iter1, *iter2 };
			double zerglingDist                 = distanceGround(*iter1, *iter2);
			double zerglingTime                 = timeGround(*iter1, *iter2);

			// calculate distanceAir from firstOverlordPosition
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

			std::set<TilePosition> overlordLink = { *iter1, *iter2 };
			double overlordDist                 = distanceAir(tp1, tp2);
			double overlordTime                 = timeAir(tp1, tp2);

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
		unscoutedPositions.insert(getBasePos(otherStart));
	}

	DebugMessenger::Instance() << allStarts.size() << " starts / " << otherStarts.size() << " otherstarts" << std::endl;
	if (Broodwar->getStartLocations().size() < 4)
		DebugMessenger::Instance() << "less than 4 start positions" << std::endl;

	for (TilePosition tp1 : otherStarts) {
		std::set<TilePosition> startToP1 = { Broodwar->self()->getStartLocation(), tp1 };
		DebugMessenger::Instance() << "ad" << overlordNetwork.find(startToP1)->second.distance << "P,   at" << overlordNetwork.find(startToP1)->second.time << "F" << std::endl;

		if (Broodwar->getStartLocations().size() == 4) {
			for (TilePosition tp2 : otherStarts) {
				if (tp2 == tp1)
					continue;
				// tp1 is any position that is not my start position,
				// tp2 is any position that is not start position or tp1

				std::set<TilePosition> remainingPlaces(otherStarts);
				remainingPlaces.erase(tp1);
				remainingPlaces.erase(tp2);
				if (remainingPlaces.size() != 1) {
					DebugMessenger::Instance() << "remaining places not equal to 1" << std::endl;

					continue;
				}

				std::set<TilePosition> startToOther       = { Broodwar->self()->getStartLocation(), *remainingPlaces.begin() };
				std::set<TilePosition> p1ToP2             = { tp1, tp2 };
				std::array<TilePosition, 3> startToP1ToP2 = { Broodwar->self()->getStartLocation(), tp1, tp2 };

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

void InformationManager::update()
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

	for (auto friendly : Broodwar->self()->getUnits()) {
		auto iterAndBool = friendlyUnits.emplace(friendly);

		// if unit already exists in friendlies
		if (!iterAndBool.second) {
			iterAndBool.first->update();
		}
	}

	validateResources();

	validateEnemyUnits();

	updateScouting();
}

void InformationManager::updateScouting()
{
	auto it = unscoutedPositions.begin();
	while (it != unscoutedPositions.end()) {
		auto p = *it;
		if (Broodwar->isVisible(TilePosition(p))) {
			scoutedPositions.insert(p);
			it = unscoutedPositions.erase(it);
			if (!enemyMain) {
				// replace IsBuilding by IsResourceDepot?
				auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(p),
				                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
				if (unitsOnBaseTile.size() > 0) {
					enemyMain          = std::make_shared<EnemyUnitInfo>(*unitsOnBaseTile.begin());
					isEnemyBaseDeduced = true;
					DebugMessenger::Instance() << "Found enemy base at: " << Broodwar->getFrameCount() << "F" << std::endl;
					if ((enemyMain->x() == 0) && (enemyMain->y() == 0)) {
						errorMessage("Found enemy base at 0,0P");
					}
				}
			}
		} else {
			it++;
		}
	}

	// add logic here for "not" finding base even after scouting everything
	// probably only applicable to Terran weird lifting stuff

	if (!(isEnemyBaseDeduced || enemyMain) && unscoutedPositions.size() == 1) {
		isEnemyBaseDeduced   = true;
		BWAPI::Position base = (*unscoutedPositions.begin());
		DebugMessenger::Instance() << "Enemy base deduced to be at: " << base.x << ", " << base.y << "P" << std::endl;
	}
}

void InformationManager::overlordScouting(BWAPI::Unit overlord)
{
	if (overlord->isUnderAttack()) { // if overlord is under attack run back to own base
		overlordRetreatToHome(overlord);
		return;
	}

	if (!enemyMain) {
		overlordScoutingAtGameStart(overlord);
	} else {
		overlordScoutingAfterBaseFound(overlord);
	}
}

void InformationManager::overlordScoutingAtGameStart(BWAPI::Unit overlord)
{
	if (overlord->isIdle()) {
		if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
			auto tp                       = scoutingOptions.begin()->POther;
			auto p                        = getBasePos(tp);
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
							auto pB          = getBasePos(tp);
							auto distToStart = distanceAir(pB, pO);
							if (distToStart < searchRadius) {
								potentialStartsFromSpotting.insert(tp);
							}
						}
					}
					if (potentialStartsFromSpotting.size() == 1) {
						isEnemyBaseFromOverlordSpotting = true;
						auto base                       = *potentialStartsFromSpotting.begin();
						enemyBaseSpottingGuess          = getBasePos(base);
						Broodwar << "Overlord spotted overlord and determined base at: " << base.x << "," << base.y << "P" << std::endl;
					}
				}
			}
		}
	}
}

void InformationManager::overlordScoutingAfterBaseFound(BWAPI::Unit overlord)
{
	if (overlord->isIdle()) {
		if (enemyRace != Races::Terran) {
			// Overlord scouting perimeter of all regions
			// Might be more useful to have this as a text hovering over overlord.
			//DebugMessenger::Instance() << "Overlord Scouting!" << std::endl;
			static std::deque<Position> scoutLocations;
			if (scoutLocations.empty()) {
				auto enemyRegion = BWTA::getRegion(enemyMain->getPosition());
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
			overlordRetreatToHome(overlord);
		}
	}
}

void InformationManager::overlordRetreatToHome(BWAPI::Unit overlord)
{
	auto ownBasePos          = getBasePos(Broodwar->self()->getStartLocation());
	auto distanceFromOwnBase = overlord->getDistance(ownBasePos);
	if (distanceFromOwnBase > 128) {
		DebugMessenger::Instance() << "Retreat overlord." << std::endl;
		OrderManager::Instance().Move(overlord, ownBasePos);
	} else {
		//DebugMessenger::Instance() << "Overlord being attacked in base." << std::endl;
		// spamming when enemy race is terran.
	}
}

ResourceUnitInfo* InformationManager::getClosestMineral(BWAPI::Unit u)
{
	double closestDist              = std::numeric_limits<double>::infinity();
	ResourceUnitInfo* chosenMineral = nullptr;
	for (auto mineral : minerals) {
		auto dist = distanceGround(u->getPosition(), mineral.getPosition());
		if (closestDist > dist) {
			closestDist   = dist;
			chosenMineral = &mineral;
		}
	}
	return chosenMineral;
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

	if ((IsResourceContainer)(unit)) {
		if ((IsMineralField)(unit)) {
			addToMinerals(unit);
		} else {
			addToGeysers(unit);
		}
	}
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)) {
			enemyBuildings.erase(unit);
			if (enemyMain) {
				if (((IsResourceDepot)(unit) == true) && (unit->getPosition() == enemyMain->getPosition())) {
					isEnemyBaseDestroyed = true;
					DebugMessenger::Instance() << "destroyed enemy base: " << Broodwar->getFrameCount() << std::endl;
				}
			}
		} else {
			enemyArmy.erase(unit);
		}
	}
}

void InformationManager::onUnitMorph(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)) {
			addToEnemyBuildings(unit);
			enemyArmy.erase(unit);
		} else {
			addToEnemyArmy(unit);
			enemyBuildings.erase(unit);
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

void InformationManager::validateEnemyUnits()
{
	// be careful about removing while iterating sets
	// https://stackoverflow.com/a/2874533/5791272
	{
		auto it = enemyBuildings.begin();
		while (it != enemyBuildings.end()) {
			bool erase = false;
			if (it->exists()) {
				if ((!IsBuilding || !IsEnemy)(it->u)) {
					erase = true;
					DebugMessenger::Instance() << "remove enemy building on validation" << std::endl;
				}
			}

			if (erase) {
				it = enemyBuildings.erase(it);
			} else {
				it++;
			}
		}
	}

	{
		auto it = enemyArmy.begin();
		while (it != enemyArmy.end()) {
			bool erase = false;
			if (it->exists()) {
				if ((IsBuilding || !IsEnemy)(it->u)) {
					erase = true;
					DebugMessenger::Instance() << "remove enemy army on validation" << std::endl;
				}
			}

			if (erase) {
				it = enemyArmy.erase(it);
			} else {
				it++;
			}
		}
	}
}

void InformationManager::addToMinerals(BWAPI::Unit mineral)
{
	auto iterAndBool = minerals.emplace(mineral);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second) {
		iterAndBool.first->update();
	}
}

void InformationManager::addToGeysers(BWAPI::Unit geyser)
{
	auto iterAndBool = geysers.emplace(geyser);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second) {
		iterAndBool.first->update();
	}
}

void InformationManager::validateResources()
{
	auto it = minerals.begin();
	while (it != minerals.end()) {
		bool erase = false;
		auto tp    = TilePosition(it->getPosition());
		if (Broodwar->isVisible(tp)) {
			auto visibleMinerals = Broodwar->getUnitsOnTile(tp, IsMineralField);
			if (visibleMinerals.size() == 0) {
				erase = true;
				DebugMessenger::Instance() << "remove mineral" << std::endl;
			}
		}

		if (erase) {
			it = minerals.erase(it);
		} else {
			it++;
		}
	}
}
