#include "UnitInfo.h"

#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(const Unit unitToWrap)
{
	if (unitToWrap) {
		u = unitToWrap;
		update();
	} else {
		errorMessage("unit null 1");
	}
}

void UnitInfo::update() const
{
	if (u) {
		if (exists()) {
			lastFrameSeen = Broodwar->getFrameCount();
			pos           = u->getPosition();
			type          = u->getType();
			shields       = u->getShields();
			hp            = u->getHitPoints();
			energy        = u->getEnergy();
			velocity      = std::make_pair(u->getVelocityX(), u->getVelocityY());
		}
	} else {
		errorMessage("unit null 2");
	}
}

bool UnitInfo::exists() const
{
	if (u) {
		return u->exists();
	} else {
		errorMessage("unit null 3");
		return false;
	}
}

BWAPI::Position UnitInfo::getPosition() const
{
	return pos;
}

bool UnitInfo::operator<(const UnitInfo& other) const
{
	return u < other.u;
}

bool operator<(const UnitInfo& lhs, const BWAPI::Unit rhs)
{
	return lhs.u < rhs;
}
bool operator<(const BWAPI::Unit lhs, const UnitInfo& rhs)
{
	return lhs < rhs.u;
}
