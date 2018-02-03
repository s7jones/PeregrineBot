#include "InformationManager.h"

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
	auto race = Broodwar->enemy()->getRace();
	if (race == Races::Unknown) {
		enemyPickedRandom  = true;
		enemyRaceIsUnknown = true;
		DebugMessenger::Instance() << "Enemy is Random" << std::endl;
	} else {
		enemyRace = race;
	}

	bool islandsOnMap = false;
	for (auto bl : BWTA::getBaseLocations()) {
		if (bl->isIsland()) {
			islandsOnMap = true;
			DebugMessenger::Instance() << "Islands on map!" << std::endl;
			break;
		}
	}

	SetupScouting();
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
			TilePosition p1, p2;
			if (*iter1 == Broodwar->self()->getStartLocation()) {
				p1 = airOrigin;
			} else {
				p1 = *iter1;
			}

			if (*iter2 == Broodwar->self()->getStartLocation()) {
				p2 = airOrigin;
			} else {
				p2 = *iter2;
			}

			std::set<TilePosition, sortByMostTopThenLeft> overlordLink = { *iter1, *iter2 };
			double overlordDist                                        = DistanceAir(p1, p2);
			double overlordTime                                        = TimeAir(p1, p2);

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

	for (TilePosition p1 : otherStarts) {
		std::set<TilePosition, sortByMostTopThenLeft> startToP1 = { Broodwar->self()->getStartLocation(), p1 };
		DebugMessenger::Instance() << "ad" << overlordNetwork.find(startToP1)->second.distance << "   at" << overlordNetwork.find(startToP1)->second.time << std::endl;

		if (Broodwar->getStartLocations().size() != 4) {
			DebugMessenger::Instance() << "less than 4 start positions" << std::endl;

		} else {
			for (TilePosition p2 : otherStarts) {
				if (p2 == p1)
					continue;
				// p1 is any position that is not my start position,
				// p2 is any position that is not start position or p1

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
	if (enemyRaceIsUnknown) {
		auto race = Broodwar->enemy()->getRace();
		if ((race == Races::Terran) || (race == Races::Zerg) || (race == Races::Protoss)) {
			enemyRace          = race;
			enemyRaceIsUnknown = false;
			DebugMessenger::Instance() << "Enemy is " << enemyRace.c_str() << std::endl;
		}
	}

	UpdateScouting();
}

void InformationManager::UpdateScouting()
{
	for (auto p : unscoutedPositions) {
		if (Broodwar->isVisible(TilePosition(p))) {
			scoutedPositions.insert(p);
			unscoutedPositions.erase(p);
			// replace IsBuilding by IsResourceDepot?
			if (Broodwar->getUnitsOnTile(TilePosition(p),
			                             IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted)
			        .size()
			    > 0) {
				enemyBase        = p;
				enemyBaseDeduced = true;
				enemyBaseFound   = true;
			}
		}
	}

	if (!(enemyBaseDeduced || enemyBaseFound) && unscoutedPositions.size() == 1) {
		enemyBaseDeduced     = true;
		BWAPI::Position base = (*unscoutedPositions.begin());
		DebugMessenger::Instance() << "Enemy base deduced to be at (" << base.x << ", " << base.y << ")" << std::endl;
	}
}

void InformationManager::OverlordScouting(BWAPI::Unit overlord)
{
	auto u = overlord;
	if (u->isIdle()) {
		if (!enemyBaseFound) {
			if (Broodwar->getStartLocations().size() == 4) { // map size is 4, use new scouting
				auto tp                       = scoutingOptions.begin()->POther;
				auto p                        = GetBasePos(tp);
				const bool firstOptionScouted = scoutedPositions.find(p) != scoutedPositions.end();
				if (!firstOptionScouted) {
					OrderManager::Instance().Move(u, p, true);
				} else {
					for (auto p2 : boost::adaptors::reverse(unscoutedPositions)) { //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
						if (p2 == p)
							continue;
						OrderManager::Instance().Move(u, p2, true);
					}
				}
			} else {                                                          // map size isn't 4, so use old scouting
				for (auto p : boost::adaptors::reverse(unscoutedPositions)) { //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
					OrderManager::Instance().Move(u, p, true);
				}
			}
		} else if (enemyRace != Races::Terran) { // enemy race isn't terran
			// Overlord scouting perimeter of all regions
			// Might be more useful to have this as a text hovering over overlord.
			//DebugMessenger::Instance() << "Overlord Scouting!" << std::endl;

			static std::deque<Position> scoutLocations;
			if (scoutLocations.empty()) {
				BWTA::Region* enemyRegion = BWTA::getRegion(enemyBase);
				BWTA::Polygon poly        = enemyRegion->getPolygon();
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
				OrderManager::Instance().Move(u, baseToScout);
				scoutLocations.erase(it);
			}
		} else { // enemy race is terran, move back to our own base
			auto ownBasePos = GetBasePos(Broodwar->self()->getStartLocation());
			OrderManager::Instance().Move(u, ownBasePos);
		}
	} else if (u->isUnderAttack()) { // if overlord is under attack run back to own base
		auto ownBasePos = GetBasePos(Broodwar->self()->getStartLocation());
		OrderManager::Instance().Move(u, ownBasePos);
	}
}
