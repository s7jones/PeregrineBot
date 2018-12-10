#include "UnitInfo.h"

#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

UnitInfo::UnitInfo(BWAPI::Unit unitToWrap)
    : m_unit(unitToWrap)
{
	update();
};

void UnitInfo::update() const
{
	if (exists())
	{
		lastFrameSeen = Broodwar->getFrameCount();
		pos           = m_unit->getPosition();
		type          = m_unit->getType();
	}
}

bool UnitInfo::exists() const
{
	if (m_unit == nullptr)
	{
		return false;
	}

	return m_unit->exists();
}

EnemyUnitInfo::EnemyUnitInfo(BWAPI::Unit unitToWrap)
    : UnitInfo(unitToWrap)
{
	update();
}

void EnemyUnitInfo::update() const
{
	UnitInfo::update();
	if (exists())
	{
		shields  = m_unit->getShields();
		hp       = m_unit->getHitPoints();
		energy   = m_unit->getEnergy();
		velocity = std::make_pair(m_unit->getVelocityX(), m_unit->getVelocityY());
	}
}

ResourceUnitInfo::ResourceUnitInfo(BWAPI::Unit unitToWrap)
    : UnitInfo(unitToWrap)
{
	update();
}

void ResourceUnitInfo::update() const
{
	UnitInfo::update();
	if (exists())
	{
		resources = m_unit->getResources();
	}
}

int ResourceUnitInfo::getResources() const
{
	return resources;
}

FriendlyUnitInfo::FriendlyUnitInfo(BWAPI::Unit unitToWrap)
    : UnitInfo(unitToWrap)
{
	update();
}

void FriendlyUnitInfo::update() const
{
	UnitInfo::update();
	if (exists())
	{
		if (m_unit->isAttackFrame())
		{
			lastFrameAttacking = Broodwar->getFrameCount();
		}
	}
}
