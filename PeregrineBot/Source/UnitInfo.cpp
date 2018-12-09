#include "UnitInfo.h"

#include "Utility.h"

using namespace BWAPI;
using namespace Filter;

void UnitInfo::update() const
{
	if (m_unit)
	{
		if (exists())
		{
			lastFrameSeen = Broodwar->getFrameCount();
			pos           = m_unit->getPosition();
			type          = m_unit->getType();
		}
	}
	else
	{
		errorMessage("unit null 2");
	}
}

bool UnitInfo::exists() const
{
	if (!m_unit)
	{
		errorMessage("unit null 3");
		return false;
	}

	return m_unit->exists();
}

void EnemyUnitInfo::update() const
{
	UnitInfo::update();
	if (m_unit)
	{
		if (exists())
		{
			shields  = m_unit->getShields();
			hp       = m_unit->getHitPoints();
			energy   = m_unit->getEnergy();
			velocity = std::make_pair(m_unit->getVelocityX(), m_unit->getVelocityY());
		}
	}
}

void ResourceUnitInfo::update() const
{
	UnitInfo::update();
	if (m_unit)
	{
		if (exists())
		{
			resources = m_unit->getResources();
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
	if (m_unit)
	{
		if (exists())
		{
			if (m_unit->isAttackFrame())
			{
				lastFrameAttacking = Broodwar->getFrameCount();
			}
		}
	}
}
