#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(Unit unitToWrap)
    : u(unitToWrap)
{
	update();
}

void UnitInfo::update()
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

bool UnitInfo::exists()
{
	return u->exists();
}

BWAPI::Position UnitInfo::getPosition()
{
	return pos;
}

bool UnitInfo::operator<(const UnitInfo& other) const
{
	return u < other.u;
}
