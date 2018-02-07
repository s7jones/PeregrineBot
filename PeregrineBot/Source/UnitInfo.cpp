#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(BWAPI::Unit unitToWrap)
    : u(unitToWrap)
{
	Update();
}

void UnitInfo::Update()
{
	if (u) {
		if (Exists()) {
			lastFrameSeen = Broodwar->getFrameCount();
			pos           = u->getPosition();
			type          = u->getType();
			shields       = u->getShields();
			hp            = u->getHitPoints();
			energy        = u->getEnergy();
			velocity      = std::make_pair(u->getVelocityX(), u->getVelocityY);
		}
	} else {
		Broodwar << "ERR: unit null" << std::endl;
	}
}

bool UnitInfo::Exists()
{
	return u->exists();
}
