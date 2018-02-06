#pragma once
#include "Common.h"
#include "OrderManager.h"
#include "Utility.h"

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
	bool isEnemyBaseDeduced   = false;
	bool isEnemyBaseFound     = false;
	bool isEnemyBaseReached   = false;
	bool isEnemyBaseDestroyed = false;
	BWAPI::Race enemyRace     = BWAPI::Races::Unknown;
	bool isEnemyRaceRandom    = false;
	bool isEnemyRaceUnknown   = false;
	bool isIslandsOnMap       = false;
	std::set<BWAPI::Unit> enemyArmy;
	std::map<BWAPI::TilePosition, std::array<double, 6>> scoutingInfo;
	std::set<BWAPI::TilePosition> allStarts;
	std::set<BWAPI::TilePosition> otherStarts; // would be a good idea to make this const
	std::set<BWAPI::Position> unscoutedPositions;
	std::set<BWAPI::Position> scoutedPositions;
	std::set<ScoutingOptionFor4, sortByMeanTime> scoutingOptions;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> zerglingNetwork;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> overlordNetwork;
	bool isEnemyBaseFromOverlordSpotting       = false;
	BWAPI::TilePosition enemyBaseSpottingGuess = { 0, 0 };

private:
	float maxBaseToBaseDistance;
	bool isPastSpottingTime = false;
};
