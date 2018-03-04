#include "UnitInfo.h"

#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(BWAPI::Unit unitToWrap)
    : u(unitToWrap)
{
	UnitInfo::update();
}

void UnitInfo::update() const
{
	if (u) {
		if (exists()) {
			lastFrameSeen = Broodwar->getFrameCount();
			pos           = u->getPosition();
			type          = u->getType();
		}
	}
}

bool UnitInfo::exists() const
{
	if (!u) {
		errorMessage("unit null 3");
		return false;
	}

	return u->exists();
}

void EnemyUnitInfo::update() const
{
	UnitInfo::update();
	if (u) {
		if (exists()) {
			shields  = u->getShields();
			hp       = u->getHitPoints();
			energy   = u->getEnergy();
			velocity = std::make_pair(u->getVelocityX(), u->getVelocityY());
		}
	}
}

void ResourceUnitInfo::update() const
{
	UnitInfo::update();
	if (u) {
		if (exists()) {
			resources = u->getResources();
		}
	}
}

int ResourceUnitInfo::getResources() const
{
	return resources;
}

void FriendlyUnitInfo::update() const
{
	UnitInfo::update();
	if (u) {
		if (exists()) {
			if (u->isAttackFrame()) {
				lastFrameAttacking = Broodwar->getFrameCount();
			}
		}
	}
}
