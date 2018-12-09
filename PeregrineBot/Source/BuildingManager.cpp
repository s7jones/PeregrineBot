#include "BuildingManager.h"

#include "BuildOrderManager.h"
#include "OrderManager.h"

using namespace BWAPI;

bool BuildingManager::isAnythingToBuild(BWAPI::Unit builder)
{
	bool flag = false;
	auto ut   = BuildOrderManager::Instance().rebuildBuilding();
	if (ut != UnitTypes::Unknown)
	{
		return tryToBuild(builder, ut);
	}

	if (!BuildOrderManager::Instance().buildOrderComplete)
	{
		if (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Spawning_Pool)
		{
			return tryToBuild(builder, UnitTypes::Zerg_Spawning_Pool);
		}
	}
	else
	{
		return tryToBuild(builder, UnitTypes::Zerg_Hatchery);
	}

	return false;
}

bool BuildingManager::tryToBuild(BWAPI::Unit builder, BWAPI::UnitType ut)
{
	if (Broodwar->self()->minerals() >= ut.mineralPrice())
	{
		// this condition maintains previous behaviour
		auto delay = (ut == UnitTypes::Zerg_Hatchery) ? 400 : 115;
		if ((lastChecked + delay) < Broodwar->getFrameCount())
		{
			TilePosition buildPosition = Broodwar->getBuildLocation(ut, builder->getTilePosition());
			if (buildPosition != TilePositions::Unknown)
			{
				OrderManager::Instance().Build(builder, ut, buildPosition);
				lastChecked = Broodwar->getFrameCount();
				return true;
			}
		}
	}
	return false;
}
