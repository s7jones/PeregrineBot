#include "Utility.h"
#include <BWTA.h>

float DistanceAir(const BWAPI::Position p1, const BWAPI::Position p2)
{
	auto dx   = abs(p1.x - p2.x);
	auto dy   = abs(p1.y - p2.y);
	auto dist = sqrt(pow(dx, 2) + pow(dy, 2));
	return dist;
}

float DistanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end)
{
	auto p1   = GetBasePos(start);
	auto p2   = GetBasePos(end);
	auto dist = DistanceAir(p1, p2);
	return dist;
}

BWAPI::Position GetPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut)
{
	return BWAPI::Position(BWAPI::Position(tp) + BWAPI::Position((ut.tileWidth() * BWAPI::TILEPOSITION_SCALE) / 2, (ut.tileHeight() * BWAPI::TILEPOSITION_SCALE) / 2));
}

BWAPI::Position GetBasePos(const BWAPI::TilePosition tp)
{
	return GetPos(tp, BWAPI::UnitTypes::Special_Start_Location);
}

double DistanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	auto dist = BWTA::getGroundDistance(start, end);
	return dist;
}


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

double TimeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	auto gdist = DistanceGround(start, end);

	//if (!reach) {
	//	gdist -= ut.sightRange();
	//}

	double time = gdist / BWAPI::UnitTypes::Zerg_Zergling.topSpeed();
	return time;
}

//auto TimeAir =
//[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const bool reach)
//{
//	auto time = TimeGround(start, end, UnitTypes::Zerg_Zergling, true);
//	return time;
//};

double TimeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end) {
	auto adist = DistanceAir(start, end)
		- BWAPI::UnitTypes::Zerg_Overlord.sightRange();

	double travelTime = adist / BWAPI::UnitTypes::Zerg_Overlord.topSpeed();
	double time = travelTime;
	return time;
}
