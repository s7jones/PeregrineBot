#pragma once
#include "Common.h"
#include "OrderManager.h"

struct ScoutingOptionFor4 {
	std::array<BWAPI::TilePosition, 3> startToP1ToP2;
	BWAPI::TilePosition POther;
	std::array<double, 2> groundTimeFromStartToP1ToP2;
	double airTimeFromStartToOther;
	double maxTime;
	double meanTime;
	double stdDev;
};

struct sortByMeanTime {
	bool operator()(const ScoutingOptionFor4& lhs, const ScoutingOptionFor4& rhs)
	{
		if (lhs.meanTime <= rhs.meanTime) {
			return true; // lhs meantTime is less
		} else {
			return false;
		}
	}
};

const auto GetPos =
    [](const BWAPI::TilePosition tp, const BWAPI::UnitType ut) {
	    return BWAPI::Position(BWAPI::Position(tp) + BWAPI::Position((ut.tileWidth() * BWAPI::TILEPOSITION_SCALE) / 2, (ut.tileHeight() * BWAPI::TILEPOSITION_SCALE) / 2));
    };

const auto GetBasePos =
    [](const BWAPI::TilePosition tp) {
	    return GetPos(tp, BWAPI::UnitTypes::Special_Start_Location);
    };

struct sortByMostTopThenLeft {
	bool operator()(const BWAPI::TilePosition& lhs_tp, const BWAPI::TilePosition& rhs_tp)
	{
		BWAPI::Position lhs = GetBasePos(lhs_tp);
		BWAPI::Position rhs = GetBasePos(rhs_tp);
		if (lhs.y <= rhs.y) {
			if (lhs.y == rhs.y) {
				if (lhs.x <= rhs.x) {
					return true; // lhs same y and less or equal x
				} else {
					return false; // lhs same y but greater x
				}
			} else {
				return true; // lhs less y
			}
		} else {
			return false; // lhs greater y
		}
	}
};

// for now these have to go under sortByMost functor until
// I can find out if I can put it in the header
struct distAndTime {
	double distance;
	double time;
};

const auto DistanceGround =
    [](const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	    auto dist = BWTA::getGroundDistance(start, end);
	    return dist;
    };

const auto DistanceAir =
    [](const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	    auto p1   = GetBasePos(start);
	    auto p2   = GetBasePos(end);
	    auto dx   = abs(p1.x - p2.x);
	    auto dy   = abs(p1.y - p2.y);
	    auto dist = sqrt(pow(dx, 2) + pow(dy, 2));
	    return dist;
    };

/*auto TimeGround =
[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const UnitType ut, const bool reach)
{
auto gdist = DistanceGround(start, end);

if (!reach) {
gdist -= ut.sightRange();
}

double time = gdist / ut.topSpeed();
return time;
};

auto TimeGround =
[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const bool reach)
{
auto time = TimeGround(start, end, UnitTypes::Zerg_Zergling, true);
return time;
}; */

const auto TimeGround =
    [](const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	    auto gdist = DistanceGround(start, end);

	    //if (!reach) {
	    //	gdist -= ut.sightRange();
	    //}

	    double time = gdist / BWAPI::UnitTypes::Zerg_Zergling.topSpeed();
	    return time;
    };

//auto TimeAir =
//[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const bool reach)
//{
//	auto time = TimeGround(start, end, UnitTypes::Zerg_Zergling, true);
//	return time;
//};

const auto TimeAir =
    [](const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	    auto adist = DistanceAir(start, end)
	        - BWAPI::UnitTypes::Zerg_Overlord.sightRange();

	    double travelTime = adist / BWAPI::UnitTypes::Zerg_Overlord.topSpeed();
	    double time       = travelTime;
	    return time;
    };

class InformationManager {
	InformationManager();

public:
	static InformationManager& Instance();
	void Setup();
	void SetupScouting();
	void Update();
	void UpdateScouting();
	void OverlordScouting(BWAPI::Unit overlord);
	void OverlordScoutingAtGameStart(BWAPI::Unit overlord);
	void OverlordScoutingAfterBaseFound(BWAPI::Unit overlord);
	void OverlordRetreatToHome(BWAPI::Unit overlord);

	std::set<BWAPI::Unit> enemyBuildings;
	BWAPI::Position enemyBase = { 0, 0 };
	bool enemyBaseDeduced     = false;
	bool enemyBaseFound       = false;
	bool enemyBaseReached     = false;
	bool enemyBaseDestroyed   = false;
	BWAPI::Race enemyRace     = BWAPI::Races::Unknown;
	bool enemyPickedRandom    = false;
	bool enemyRaceIsUnknown   = false;
	bool islandsOnMap         = false;
	std::set<BWAPI::Unit> enemyArmy;
	std::map<BWAPI::TilePosition, std::array<double, 6>> scoutingInfo;
	std::set<BWAPI::TilePosition> allStarts;
	std::set<BWAPI::TilePosition> otherStarts;
	std::set<BWAPI::Position> unscoutedPositions;
	std::set<BWAPI::Position> scoutedPositions;
	std::set<ScoutingOptionFor4, sortByMeanTime> scoutingOptions;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> zerglingNetwork;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> overlordNetwork;
};
