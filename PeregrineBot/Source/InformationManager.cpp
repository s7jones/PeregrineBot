#include "InformationManager.h"

#include "DebugMessenger.h"
#include "OrderManager.h"
#include <boost/range/adaptor/reversed.hpp>

using namespace BWAPI;
using namespace Filter;

void InformationManager::setup()
{
	auto enemy = Broodwar->enemy();
	if (enemy)
	{
		auto race = enemy->getRace();
		if (race == Races::Unknown)
		{
			isEnemyRaceRandom = true;
			DebugMessenger::Instance() << "Enemy is Random" << std::endl;
		}
		else
		{
			m_enemyRace = race;
		}
	}
	else
	{
		// this is required as there was a crash when one bot leaves
		// during the game lobby countdown.
		errorMessage("no enemy");
	}

	for (auto base : BWTA::getBaseLocations())
	{
		if (base == nullptr)
		{
			continue;
		}

		if (base->isIsland())
		{
			isIslandsOnMap = true;
			DebugMessenger::Instance() << "Islands on map!" << std::endl;
			break;
		}
	}

	setupScouting();

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1)
	{
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2)
		{
			auto base1tp    = *iter1;
			auto base2tp    = *iter2;
			auto base1      = getBasePos(base1tp);
			auto base2      = getBasePos(base2tp);
			auto distGround = distanceGround(base1, base2);
			if (distGround > maxBaseToBaseDistance.ground)
			{
				maxBaseToBaseDistance.ground = distGround;
			}
			auto distAir = distanceAir(base1, base2);
			if (distAir > maxBaseToBaseDistance.air)
			{
				maxBaseToBaseDistance.air = distAir;
			}
		}
	}
	DebugMessenger::Instance() << "max base to base ground is: " << maxBaseToBaseDistance.ground << "P" << std::endl;
	DebugMessenger::Instance() << "max base to base air is: " << maxBaseToBaseDistance.air << "P" << std::endl;

	// maybe make 128 * 1.5 a const "smudge factor" variable
	auto addToSpottingMap = [this](UnitType ut, double smudgeFactor) {
		auto maxDist = ut.isFlyer() ? maxBaseToBaseDistance.air : maxBaseToBaseDistance.ground;
		m_spottingTimes.insert({ ut,
		                         (maxDist + smudgeFactor) / ut.topSpeed() });
	};

	addToSpottingMap(UnitTypes::Zerg_Overlord, 128 * 1.5);
	for (auto ut : { UnitTypes::Terran_SCV, UnitTypes::Zerg_Drone, UnitTypes::Protoss_Probe })
	{
		addToSpottingMap(ut, 128 * 0.5);
	}

	using pair_type = decltype(m_spottingTimes)::value_type;
	auto ptr        = std::max_element(
        std::begin(m_spottingTimes), std::end(m_spottingTimes),
        [](const pair_type& p1, const pair_type& p2) {
            return p1.second < p2.second;
        });

	m_spottingTime = ptr->second;
}

void InformationManager::setupScouting()
{
	std::set<Unit> overlords;
	for (Unit u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Zerg_Overlord)
			overlords.insert(u);
	}

	TilePosition airOrigin;
	if (overlords.size() == 1)
	{
		airOrigin = (TilePosition)(*overlords.begin())->getPosition();
	}
	else
	{
		airOrigin = Broodwar->self()->getStartLocation();
		DebugMessenger::Instance() << "Not exactly 1 Overlord at start?!" << std::endl;
	}

	for (auto iter1 = Broodwar->getStartLocations().begin(); iter1 != (Broodwar->getStartLocations().end() - 1); ++iter1)
	{
		for (auto iter2 = iter1 + 1; iter2 != Broodwar->getStartLocations().end(); ++iter2)
		{
			std::set<TilePosition> zerglingLink = { *iter1, *iter2 };
			double zerglingDist                 = distanceGround(*iter1, *iter2);
			double zerglingTime                 = timeGround(*iter1, *iter2);

			// calculate distanceAir from firstOverlordPosition
			TilePosition tp1, tp2;
			if (*iter1 == Broodwar->self()->getStartLocation())
			{
				tp1 = airOrigin;
			}
			else
			{
				tp1 = *iter1;
			}

			if (*iter2 == Broodwar->self()->getStartLocation())
			{
				tp2 = airOrigin;
			}
			else
			{
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

	if (zerglingNetwork.size() != networkSize || overlordNetwork.size() != networkSize)
	{
		DebugMessenger::Instance() << "Network size does not match maths." << std::endl;
	}
	std::map<std::array<TilePosition, 3>, std::array<double, 3>> scoutingNetwork;

	auto allStartsList = Broodwar->getStartLocations();
	//allStarts(allStartsList.begin(), allStartsList.end());
	std::copy(allStartsList.begin(), allStartsList.end(), std::inserter(allStarts, allStarts.end()));
	//m_otherStarts(allStarts);
	std::copy(allStarts.begin(), allStarts.end(), std::inserter(m_otherStarts, m_otherStarts.end()));
	m_otherStarts.erase(Broodwar->self()->getStartLocation());
	//unscoutedPositions(m_otherStarts);
	//std::copy(m_otherStarts.begin(), m_otherStarts.end(), std::inserter(unscoutedPositions, unscoutedPositions.end()));
	for (auto otherStart : m_otherStarts)
	{
		unscoutedPositions.insert(getBasePos(otherStart));
	}

	DebugMessenger::Instance() << allStarts.size() << " starts / " << m_otherStarts.size() << " otherstarts" << std::endl;
	if (Broodwar->getStartLocations().size() < 4)
		DebugMessenger::Instance() << "less than 4 start positions" << std::endl;

	for (TilePosition tp1 : m_otherStarts)
	{
		std::set<TilePosition> startToP1 = { Broodwar->self()->getStartLocation(), tp1 };
		DebugMessenger::Instance() << "ad" << overlordNetwork.find(startToP1)->second.distance << "P,   at" << overlordNetwork.find(startToP1)->second.time << "F" << std::endl;

		if (Broodwar->getStartLocations().size() == 4)
		{
			for (TilePosition tp2 : m_otherStarts)
			{
				if (tp2 == tp1)
					continue;
				// tp1 is any position that is not my start position,
				// tp2 is any position that is not start position or tp1

				std::set<TilePosition> remainingPlaces(m_otherStarts);
				remainingPlaces.erase(tp1);
				remainingPlaces.erase(tp2);
				if (remainingPlaces.size() != 1)
				{
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
	if (m_enemyRace == Races::Unknown)
	{
		auto enemy = Broodwar->enemy();
		if (enemy)
		{
			auto race = enemy->getRace();
			if ((race == Races::Terran) || (race == Races::Zerg) || (race == Races::Protoss))
			{
				m_enemyRace = race;
				DebugMessenger::Instance() << "Enemy is " << m_enemyRace.c_str() << std::endl;
			}
		}
	}

	for (auto friendly : Broodwar->self()->getUnits())
	{
		auto iterAndBool = friendlyUnits.emplace(friendly);

		// if unit already exists in friendlies
		if (!iterAndBool.second)
		{
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
	while (it != unscoutedPositions.end())
	{
		auto p = *it;
		if (Broodwar->isVisible(TilePosition(p)))
		{
			scoutedPositions.insert(p);
			it = unscoutedPositions.erase(it);
			if (!m_enemyMain.m_unit)
			{
				// replace IsBuilding by IsResourceDepot?
				auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(p),
				                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
				if (unitsOnBaseTile.size() > 0)
				{
					m_enemyMain          = { *unitsOnBaseTile.begin() };
					m_isEnemyBaseDeduced = true;
					DebugMessenger::Instance() << "Found enemy base at: " << Broodwar->getFrameCount() << "F" << std::endl;
					if ((m_enemyMain.x() == 0) && (m_enemyMain.y() == 0))
					{
						errorMessage("Found enemy base at 0,0P");
					}
				}
			}
		}
		else
		{
			it++;
		}
	}

	// add logic here for "not" finding base even after scouting everything
	// probably only applicable to Terran weird lifting stuff

	if (!(m_isEnemyBaseDeduced || m_enemyMain.m_unit) && unscoutedPositions.size() == 1)
	{
		m_isEnemyBaseDeduced = true;
		BWAPI::Position base = (*unscoutedPositions.begin());
		DebugMessenger::Instance() << "Enemy base deduced to be at: " << base.x << ", " << base.y << "P" << std::endl;
	}

	for (auto singleUnitWithPotentialBases : spottedUnitsAndPotentialBases)
	{
		for (auto scoutedPosition : scoutedPositions)
		{
			singleUnitWithPotentialBases.second.erase(scoutedPosition);
		}

		if (singleUnitWithPotentialBases.second.size() == 1)
		{
			m_isEnemyBaseFromSpotting = true;
			enemyBaseSpottingGuess    = *singleUnitWithPotentialBases.second.begin();
			Broodwar << "Spotted guess by removal and determined base at: " << enemyBaseSpottingGuess << "P" << std::endl;
			return;
		}
	}

	if (m_enemyMain.m_unit || m_isEnemyBaseFromSpotting)
	{
		if (m_isSpottingUnitsTime)
		{
			m_isSpottingUnitsTime = false;
		}
		if (m_isSpottingCreepTime)
		{
			m_isSpottingCreepTime = false;
		}
	}
}

void InformationManager::overlordScouting(BWAPI::Unit overlord)
{
	if (overlord->isUnderAttack())
	{ // if overlord is under attack run back to own base
		overlordRetreatToHome(overlord);
		return;
	}

	if (!m_enemyMain.m_unit)
	{
		overlordScoutingAtGameStart(overlord);
	}
	else
	{
		overlordScoutingAfterBaseFound(overlord);
	}
}

void InformationManager::overlordScoutingAtGameStart(BWAPI::Unit overlord)
{
	if (overlord->isIdle())
	{
		if (Broodwar->getStartLocations().size() == 4)
		{ // map size is 4, use new scouting
			auto tp                       = scoutingOptions.begin()->POther;
			auto p                        = getBasePos(tp);
			const bool firstOptionScouted = scoutedPositions.find(p) != scoutedPositions.end();
			if (!firstOptionScouted)
			{
				OrderManager::Instance().Move(overlord, p, true);
			}
			else
			{
				for (auto p2 : boost::adaptors::reverse(unscoutedPositions))
				{ //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
					if (p2 == p)
						continue;
					OrderManager::Instance().Move(overlord, p2, true);
				}
			}
		}
		else
		{ // map size isn't 4, so use old scouting
			for (auto p : boost::adaptors::reverse(unscoutedPositions))
			{ //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
				OrderManager::Instance().Move(overlord, p, true);
			}
		}
	}
}

void InformationManager::spotting(BWAPI::Unit spotter)
{

	// creep spotting
	if ((m_enemyRace == Races::Zerg || m_enemyRace == Races::Unknown)
	    && m_isSpottingCreepTime && (!IsBuilding)(spotter))
	{
		spotCreep(spotter);
	}

	if (m_isSpottingUnitsTime)
	{
		if (Broodwar->getFrameCount() > m_spottingTime)
		{
			m_isSpottingUnitsTime = false;
			DebugMessenger::Instance() << "Past spotting time" << std::endl;
		}
		else
		{
			spotUnits(spotter);
		}
	}
}

void InformationManager::spotUnits(BWAPI::Unit spotter)
{
	if (m_isEnemyBaseFromSpotting)
	{
		return;
	}
	const auto largestZergSightRange = UnitTypes::Zerg_Hive.sightRange();
	// ADDING 32 incase there is funkiness with getUnitsInRange
	const auto range  = largestZergSightRange + 32;
	auto unitsSpotted = spotter->getUnitsInRadius(range, IsEnemy && IsVisible);
	for (auto target : unitsSpotted)
	{
		auto itTarget = find_if(spottedUnitsAndPotentialBases.begin(), spottedUnitsAndPotentialBases.end(),
		                        [target](const unitAndPotentialBases& val) -> bool {
			                        return val.first == target;
		                        });
		if (itTarget == spottedUnitsAndPotentialBases.end())
		{
			continue;
		}

		auto targetType = target->getType();
		auto itType     = m_spottingTimes.find(targetType);
		if (itType == m_spottingTimes.end())
		{
			continue;
		}

		auto pO               = target->getPosition();
		double smudgeFactor   = 128 * 1.5; // add a bit to account for overlord/workers spawning in a different place from base
		double searchDistance = itType->first.topSpeed() * Broodwar->getFrameCount() + smudgeFactor;

		std::set<Position> potentialStartsFromSpotting;
		for (auto tp : m_otherStarts)
		{
			auto pB          = getBasePos(tp);
			auto distToStart = targetType.isFlyer() ? distanceAir(pB, pO) : distanceGround(pB, pO);
			if (distToStart < searchDistance)
			{
				potentialStartsFromSpotting.insert(pB);
			}
		}

		spottedUnitsAndPotentialBases.insert({ target, potentialStartsFromSpotting });

		if (potentialStartsFromSpotting.size() == 1)
		{
			m_isEnemyBaseFromSpotting = true;
			enemyBaseSpottingGuess    = *potentialStartsFromSpotting.begin();
			Broodwar << spotter->getType() << " spotted " << targetType << " and determined base at: " << enemyBaseSpottingGuess << "P" << std::endl;
			return;
		}
	}
}

void InformationManager::spotCreep(BWAPI::Unit spotter)
{
	auto regionUnit = BWTA::getRegion(spotter->getPosition());
	if (regionUnit == nullptr)
	{
		return;
	}

	auto regionStart = BWTA::getRegion(Broodwar->self()->getStartLocation());
	if (regionStart == nullptr)
	{
		return;
	}

	// don't spot for creep inside my own base.
	// not future proof.
	if (regionUnit == regionStart)
	{
		return;
	}

	const int radiusTPSpotter = (spotter->getType().sightRange() + 32) / BWAPI::TILEPOSITION_SCALE;
	auto tp                   = spotter->getTilePosition();
	for (auto i = -radiusTPSpotter; i < radiusTPSpotter + 1; i++)
	{
		auto x = tp.x + i;
		if (x < 0 || x >= Broodwar->mapWidth())
		{
			continue;
		}

		for (auto j = -radiusTPSpotter; j < radiusTPSpotter + 1; j++)
		{
			auto y = tp.y + j;
			if (y < 0 || y >= Broodwar->mapHeight())
			{
				continue;
			}

			TilePosition tpRelative = { x, y };

			bool hasCreep = Broodwar->hasCreep(tpRelative);
			if (!hasCreep)
			{
				continue;
			}

			auto regionRelative = BWTA::getRegion(tpRelative);
			if (regionRelative == nullptr)
			{
				continue;
			}

			for (auto otherStart : m_otherStarts)
			{
				auto regionOtherStart = BWTA::getRegion(otherStart);
				if (regionOtherStart == nullptr)
				{
					continue;
				}

				if (regionOtherStart == regionRelative)
				{
					m_isEnemyBaseFromSpotting = true;
					m_isSpottingCreepTime     = false;
					enemyBaseSpottingGuess    = getBasePos(otherStart);
					Broodwar << "Spotted creep and determined base at: " << enemyBaseSpottingGuess << "P" << std::endl;
					return;
				}
			}
		}
	}
}

void InformationManager::overlordScoutingAfterBaseFound(BWAPI::Unit overlord)
{
	if (!overlord->isIdle())
	{
		return;
	}

	if (m_enemyRace != Races::Terran)
	{
		// Overlord scouting perimeter of all regions
		// Might be more useful to have this as a text hovering over overlord.
		//DebugMessenger::Instance() << "Overlord Scouting!" << std::endl;
		static std::deque<Position> scoutLocations;
		if (scoutLocations.empty())
		{
			auto enemyRegion = BWTA::getRegion(m_enemyMain.getPosition());
			if (enemyRegion == nullptr)
			{
				return;
			}

			auto& poly = enemyRegion->getPolygon();
			for (size_t j = 0; j < poly.size(); ++j)
			{
				// The points in Polygon appear to be all along the perimeter.
				Position point1 = poly[j];
				scoutLocations.push_back(point1);
			}

			for (auto base : BWTA::getBaseLocations())
			{
				if (base == nullptr)
				{
					continue;
				}

				Position point1 = base->getPosition();
				scoutLocations.push_back(point1);
			}
		}
		else
		{
			auto it              = scoutLocations.begin();
			Position baseToScout = (*it);
			OrderManager::Instance().Move(overlord, baseToScout);
			scoutLocations.erase(it);
		}
	}
	else
	{ // enemy race is terran, move back to our own base
		overlordRetreatToHome(overlord);
	}
}

void InformationManager::overlordRetreatToHome(BWAPI::Unit overlord)
{
	auto ownBasePos          = getBasePos(Broodwar->self()->getStartLocation());
	auto distanceFromOwnBase = overlord->getDistance(ownBasePos);
	if (distanceFromOwnBase > 128)
	{
		DebugMessenger::Instance() << "Retreat overlord." << std::endl;
		OrderManager::Instance().Move(overlord, ownBasePos);
	}
	else
	{
		//DebugMessenger::Instance() << "Overlord being attacked in base." << std::endl;
		// spamming when enemy race is terran.
	}
}

std::unique_ptr<ResourceUnitInfo> InformationManager::getClosestMineral(BWAPI::Unit u)
{
	double closestDist                              = std::numeric_limits<double>::infinity();
	std::unique_ptr<ResourceUnitInfo> chosenMineral = nullptr;
	for (auto mineral : minerals)
	{
		auto dist = distanceGround(u->getPosition(), mineral.getPosition());
		if (closestDist > dist)
		{
			closestDist   = dist;
			chosenMineral = std::make_unique<ResourceUnitInfo>(mineral);
		}
	}
	return chosenMineral;
}

void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit))
	{
		if ((IsBuilding)(unit))
		{
			addToEnemyBuildings(unit);
		}
		else
		{
			addToEnemyArmy(unit);
		}
	}

	if ((IsResourceContainer)(unit))
	{
		if ((IsMineralField)(unit))
		{
			addToMinerals(unit);
		}
		else
		{
			addToGeysers(unit);
		}
	}
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit))
	{
		if ((IsBuilding)(unit))
		{
			auto it = m_enemyBuildings.find(unit);
			if (it != m_enemyBuildings.end())
			{
				m_enemyBuildings.erase(it);
			}
			if (m_enemyMain.m_unit)
			{
				if (((IsResourceDepot)(unit) == true) && (unit->getPosition() == m_enemyMain.getPosition()))
				{
					isEnemyBaseDestroyed = true;
					DebugMessenger::Instance() << "destroyed enemy base: " << Broodwar->getFrameCount() << std::endl;
				}
			}
		}
		else
		{
			auto it = m_enemyArmy.find(unit);
			if (it != m_enemyArmy.end())
			{
				m_enemyArmy.erase(it);
			}
		}
	}
}

void InformationManager::onUnitMorph(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit))
	{
		if ((IsBuilding)(unit))
		{
			addToEnemyBuildings(unit);
			auto it = m_enemyArmy.find(unit);
			if (it != m_enemyArmy.end())
			{
				m_enemyArmy.erase(it);
			}
		}
		else
		{
			addToEnemyArmy(unit);
			auto it = m_enemyBuildings.find(unit);
			if (it != m_enemyBuildings.end())
			{
				m_enemyBuildings.erase(it);
			}
		}
	}
}

void InformationManager::addToEnemyBuildings(BWAPI::Unit unit)
{
	auto iterAndBool = m_enemyBuildings.emplace(unit);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second)
	{
		iterAndBool.first->update();
	}
}

void InformationManager::addToEnemyArmy(BWAPI::Unit unit)
{
	auto iterAndBool = m_enemyArmy.emplace(unit);

	// if unit already exists in enemyArmy
	if (!iterAndBool.second)
	{
		iterAndBool.first->update();
	}
}

void InformationManager::validateEnemyUnits()
{
	// be careful about removing while iterating sets
	// https://stackoverflow.com/a/2874533/5791272
	{
		auto it = m_enemyBuildings.begin();
		while (it != m_enemyBuildings.end())
		{
			bool erase = false;
			if (it->exists())
			{
				if ((!IsBuilding || !IsEnemy)(it->m_unit))
				{
					erase = true;
					DebugMessenger::Instance() << "remove enemy building on validation" << std::endl;
				}
			}

			if (erase)
			{
				it = m_enemyBuildings.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	{
		auto it = m_enemyArmy.begin();
		while (it != m_enemyArmy.end())
		{
			bool erase = false;
			if (it->exists())
			{
				if ((IsBuilding || !IsEnemy)(it->m_unit))
				{
					erase = true;
					DebugMessenger::Instance() << "remove enemy army on validation" << std::endl;
				}
			}

			if (erase)
			{
				it = m_enemyArmy.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

void InformationManager::addToMinerals(BWAPI::Unit mineral)
{
	auto iterAndBool = minerals.emplace(mineral);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second)
	{
		iterAndBool.first->update();
	}
}

void InformationManager::addToGeysers(BWAPI::Unit geyser)
{
	auto iterAndBool = geysers.emplace(geyser);

	// if unit already exists in enemyBuildings
	if (!iterAndBool.second)
	{
		iterAndBool.first->update();
	}
}

void InformationManager::validateResources()
{
	auto it = minerals.begin();
	while (it != minerals.end())
	{
		bool erase = false;
		auto tp    = TilePosition(it->getPosition());
		if (Broodwar->isVisible(tp))
		{
			auto visibleMinerals = Broodwar->getUnitsOnTile(tp, IsMineralField);
			if (visibleMinerals.empty())
			{
				erase = true;
				DebugMessenger::Instance() << "remove mineral" << std::endl;
			}
		}

		if (erase)
		{
			it = minerals.erase(it);
		}
		else
		{
			it++;
		}
	}
}
