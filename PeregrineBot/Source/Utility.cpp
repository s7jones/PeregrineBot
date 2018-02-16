#include "Utility.h"

using namespace BWAPI;

double distanceAir(const BWAPI::Position p1, const BWAPI::Position p2)
{
	auto dx   = abs(p1.x - p2.x);
	auto dy   = abs(p1.y - p2.y);
	auto dist = sqrt(pow(dx, 2) + pow(dy, 2));
	return dist;
}

double distanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end)
{
	auto p1   = getBasePos(start);
	auto p2   = getBasePos(end);
	auto dist = distanceAir(p1, p2);
	return dist;
}

BWAPI::Position getPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut)
{
	return BWAPI::Position(BWAPI::Position(tp) + BWAPI::Position((ut.tileWidth() * BWAPI::TILEPOSITION_SCALE) / 2, (ut.tileHeight() * BWAPI::TILEPOSITION_SCALE) / 2));
}

BWAPI::Position getBasePos(const BWAPI::TilePosition tp)
{
	return getPos(tp, BWAPI::UnitTypes::Special_Start_Location);
}

double distanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end)
{
	auto dist = BWTA::getGroundDistance(start, end);
	return dist;
}

double distanceGround(const BWAPI::Position start, const BWAPI::Position end)
{
	return distanceGround(TilePosition(start), TilePosition(end));
}

/*auto timeGround =
[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const UnitType ut, const bool reach)
{
auto gdist = distanceGround(start, end);

if (!reach) {
gdist -= ut.sightRange();
}

double time = gdist / ut.topSpeed();
return time;
};

auto timeGround =
[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const bool reach)
{
auto time = timeGround(start, end, UnitTypes::Zerg_Zergling, true);
return time;
}; */

double timeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end)
{
	auto gdist = distanceGround(start, end);

	//if (!reach) {
	//	gdist -= ut.sightRange();
	//}

	double time = gdist / BWAPI::UnitTypes::Zerg_Zergling.topSpeed();
	return time;
}

//auto timeAir =
//[](const BWAPI::TilePosition start, const BWAPI::TilePosition end, const bool reach)
//{
//	auto time = timeGround(start, end, UnitTypes::Zerg_Zergling, true);
//	return time;
//};

double timeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end)
{
	auto adist = distanceAir(start, end)
	    - BWAPI::UnitTypes::Zerg_Overlord.sightRange();

	double travelTime = adist / BWAPI::UnitTypes::Zerg_Overlord.topSpeed();
	double time       = travelTime;
	return time;
}

bool isReachable(BWTA::Region* region1, BWTA::Region* region2)
{
	if (region1 == nullptr || region2 == nullptr) {
		return false;
	} else {
		return region1->isReachable(region2);
	}
}

// jaj22 linked this on discord: https://pastebin.com/k6esbYUj
// tscmoo said he used the below code in OpenBW
static const bool psi_field_mask[5][8] = {
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 0, 0 },
	{ 1, 1, 1, 0, 0, 0, 0, 0 }
};

bool isInPylonRange(int relx, int rely)
{
	unsigned x = std::abs(relx);
	unsigned y = std::abs(rely);
	if (x >= 256) return false;
	if (y >= 160) return false;
	if (relx < 0) --x;
	if (rely < 0) --y;
	return psi_field_mask[y / 32u][x / 32u];
}

void errorMessage(std::string message)
{
	Broodwar << "err: " << message << std::endl;
}
