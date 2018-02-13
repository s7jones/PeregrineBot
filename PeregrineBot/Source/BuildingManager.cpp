#include "BuildingManager.h"

#include "BuildOrderManager.h"
#include "OrderManager.h"

using namespace BWAPI;

BuildingManager& BuildingManager::Instance()
{
	static BuildingManager instance;
	return instance;
}

bool BuildingManager::isAnythingToBuild(BWAPI::Unit builder)
{
	bool flag = false;

	if (!BuildOrderManager::Instance().buildOrderComplete) {
		if (*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Spawning_Pool) {
			//if ((!BuildOrderManager::Instance().pool) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Spawning_Pool.mineralPrice())) {
			//	if ((poolLastChecked + 115) < Broodwar->getFrameCount()) {
			//		TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Spawning_Pool, builder->getTilePosition());
			//		OrderManager::Instance().Build(builder, UnitTypes::Zerg_Spawning_Pool, buildPosition);
			//		poolLastChecked = Broodwar->getFrameCount();
			//		return true;
			//	}
			//}
			return tryToBuild(builder, UnitTypes::Zerg_Spawning_Pool);
		}
	} else {
		//if (Broodwar->self()->minerals() >= UnitTypes::Zerg_Hatchery.mineralPrice()) {
		//	if ((lastChecked + 400) < Broodwar->getFrameCount()) {
		//		TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Hatchery, builder->getTilePosition());
		//		OrderManager::Instance().Build(builder, UnitTypes::Zerg_Hatchery, buildPosition);
		//		lastChecked = Broodwar->getFrameCount();
		//		return true;
		//	}
		//}
		return tryToBuild(builder, UnitTypes::Zerg_Hatchery);
	}
	return false;
}

bool BuildingManager::tryToBuild(BWAPI::Unit builder, BWAPI::UnitType ut)
{
	if (Broodwar->self()->minerals() >= ut.mineralPrice()) {
		// this condition maintains previous behaviour
		auto delay = (ut == UnitTypes::Zerg_Hatchery) ? 400 : 115;
		if ((lastChecked + delay) < Broodwar->getFrameCount()) {
			TilePosition buildPosition = Broodwar->getBuildLocation(ut, builder->getTilePosition());
			if (buildPosition != TilePositions::Unknown) {
				OrderManager::Instance().Build(builder, ut, buildPosition);
				lastChecked = Broodwar->getFrameCount();
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}

BuildingManager::BuildingManager()
{
}
