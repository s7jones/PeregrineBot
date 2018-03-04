#include "UnitInfo.h"

#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(BWAPI::Unit unitToWrap)
    : unit(unitToWrap)
{
	UnitInfo::update();
}

void UnitInfo::update() const
{
	if (unit) {
		if (exists()) {
			lastFrameSeen = Broodwar->getFrameCount();
			pos           = unit->getPosition();
			type          = unit->getType();
		}
	}
}

bool UnitInfo::exists() const
{
	if (!unit) {
		errorMessage("unit null 3");
		return false;
	}

	return unit->exists();
}

void EnemyUnitInfo::update() const
{
	UnitInfo::update();
	if (unit) {
		if (exists()) {
			shields  = unit->getShields();
			hp       = unit->getHitPoints();
			energy   = unit->getEnergy();
			velocity = std::make_pair(unit->getVelocityX(), unit->getVelocityY());
		}
	}
}

void ResourceUnitInfo::update() const
{
	UnitInfo::update();
	if (unit) {
		if (exists()) {
			resources = unit->getResources();
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
	if (unit) {
		if (exists()) {
			if (unit->isAttackFrame()) {
				lastFrameAttacking = Broodwar->getFrameCount();
			}
		}
	}
}
