#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(Unit unitToWrap)
    : u(unitToWrap)
{
	update();
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
		Broodwar << "ERR: unit null" << std::endl;
	}
}

bool UnitInfo::exists() const
{
	return u->exists();
}

BWAPI::Position UnitInfo::getPosition() const
{
	return pos;
}

bool UnitInfo::operator<(const UnitInfo& other) const
{
	return u < other.u;
}

bool operator<(const UnitInfo& lhs, const BWAPI::Unit& rhs)
{
	return lhs.u < rhs;
}
bool operator<(const BWAPI::Unit& lhs, const UnitInfo& rhs)
{
	return lhs < rhs.u;
}
