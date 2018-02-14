#pragma once
#include "Common.h"

double DistanceAir(const BWAPI::Position p1, const BWAPI::Position p2);
double DistanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
BWAPI::Position GetPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut);
BWAPI::Position GetBasePos(const BWAPI::TilePosition tp);
double DistanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double DistanceGround(const BWAPI::Position start, const BWAPI::Position end);
double TimeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double TimeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
bool isReachable(BWTA::Region* region1, BWTA::Region* region2);

void errorMessage(std::string message);

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
