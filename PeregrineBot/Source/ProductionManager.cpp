#include "ProductionManager.h"

using namespace BWAPI;

ProductionManager::ProductionManager()
{

	//useMultiThread = false;

	//addQueueMutex = CreateMutex(NULL, FALSE, NULL);

	reservedMinerals = 0;
	reservedGas      = 0;

	BuildingsQueueType.clear();

	BuildingsQueue.clear();
}

int ProductionManager::getAvailableMinerals()
{
	return Broodwar->self()->minerals() - reservedMinerals;
}

int ProductionManager::getAvailableGas()
{
	return Broodwar->self()->gas() - reservedGas;
}

void ProductionManager::addToBuildOrder(BWAPI::UnitType type, int supply)
{
	IndividualBuildOrder newBO;
	newBO.type   = type;
	newBO.supply = supply;
	BuildingAtSupplyOrder.push_back(newBO);
}

void ProductionManager::addToBuildOrderBegin(BWAPI::UnitType type, int supply)
{

	IndividualBuildOrder newBO;
	newBO.type   = type;
	newBO.supply = supply;
	this->BuildingAtSupplyOrder.insert(this->BuildingAtSupplyOrder.begin(), newBO);
}

void ProductionManager::removeFromQueue(BWAPI::UnitType type)
{
	for (int i = 0; i < BuildingsQueue.size(); i++) {
		if (BuildingsQueue[i].type == type) {
			if (BuildingsQueue[i].Started == false) {
				if (BuildingsQueue[i].Worker != NULL) {
					returnBuilderWorker(BuildingsQueue[i].Worker);
				}
				BuildingsQueue.erase(BuildingsQueue.begin() + i);
				break;
			}
		}
	}
}

void ProductionManager::addToQueueBegin(BWAPI::UnitType type, BWAPI::TilePosition tileClose)
{
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();
	BuildQueue addQueue;
	//if (tileClose == BWAPI::TilePositions::Unknown) {
	//	addQueue.buildLocation = bManager->getBuildLocationBase(CCmanager[0]->CommandCenter->getTilePosition(), type, 0);
	//} else {
	//	addQueue.buildLocation = bManager->getBuildLocationBase(tileClose, type, BWTA::getRegion(InfoMan->PosOurBase));
	//}
	addQueue.buildLocation = Broodwar->getBuildLocation(type, tileClose);

	//reserve space
	//bManager->mapArea(bManager->getBuildRectangle(addQueue.buildLocation, type), 0, 0);

	addQueue.type = type;
	//addQueue.Worker = CCmanager[0]->getBuilder();
	addQueue.Worker       = getWorkerBuilder(type, addQueue.buildLocation);
	addQueue.building     = NULL;
	addQueue.Started      = false;
	addQueue.FrameStarted = Broodwar->getFrameCount();
	BuildingsQueue.insert(BuildingsQueue.begin(), addQueue);
	Broodwar->printf("Next building added: %s", type.c_str());
	//BuildingOrder.erase( BuildingOrder.begin() );
}

void ProductionManager::addToQueue(BWAPI::UnitType type)
{

	//Broodwar->printf("Next building added: %s", type.c_str() );

	BWAPI::UnitType* Storetype = new BWAPI::UnitType(type);

	//this->reservedMinerals +=  type.mineralPrice();
	//this->reservedGas += type.gasPrice();

	BuildingsQueueType.push_back(type);

	//if (useMultiThread == true) {
	//	this->reservedMinerals += type.mineralPrice();
	//	this->reservedGas += type.gasPrice();
	//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addToQueueMultiThread, Storetype, 0, NULL);
	//} else {

	BuildQueue addQueue;
	//addQueue.buildLocation = bManager->getBuildLocationBase(CCmanager[0]->CommandCenter->getTilePosition(), type, 0);
	addQueue.buildLocation = Broodwar->getBuildLocation(type, baseManager->CommandCenter->getTilePosition());
	//reserve space
	if (addQueue.buildLocation == BWAPI::TilePositions::None) {
		return;
	}
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();
	//bManager->mapArea(bManager->getBuildRectangle(addQueue.buildLocation, type), 0, 0);

	addQueue.type = type;
	//addQueue.Worker = CCmanager[0]->getBuilder();
	addQueue.Worker       = getWorkerBuilder(type, addQueue.buildLocation);
	addQueue.building     = NULL;
	addQueue.Started      = false;
	addQueue.FrameStarted = Broodwar->getFrameCount();
	BuildingsQueue.push_back(addQueue);
	Broodwar->printf("Next building added: %s", type.c_str());
	//BuildingOrder.erase( BuildingOrder.begin() );
	//}
}

void ProductionManager::addToQueueTile(BWAPI::UnitType type, BWAPI::TilePosition tile)
{
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();
	BuildQueue addQueue;
	addQueue.buildLocation = tile;
	//reserve space
	//bManager->mapArea(bManager->getBuildRectangle(addQueue.buildLocation, type), 0, 0);

	addQueue.type = type;
	//addQueue.Worker = CCmanager[0]->getBuilder();
	addQueue.Worker       = getWorkerBuilder(type, addQueue.buildLocation);
	addQueue.building     = NULL;
	addQueue.Started      = false;
	addQueue.FrameStarted = Broodwar->getFrameCount();
	BuildingsQueue.push_back(addQueue);
	Broodwar->printf("Next building added: %s", type.c_str());
	//BuildingOrder.erase( BuildingOrder.begin() );
}

void ProductionManager::addToQueueClose(BWAPI::UnitType type, BWAPI::TilePosition tile, BWTA::Region* region)
{
	//yea I know  NULL == 0 , but just in case of some compiler setting or something like that
	if (region == NULL) {
		region = 0;
	}
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();
	BuildQueue addQueue;
	//addQueue.buildLocation = bManager->getBuildLocationBase(tile, type, region);
	addQueue.buildLocation = Broodwar->getBuildLocation(type, tile);
	//reserve space
	//bManager->mapArea(bManager->getBuildRectangle(addQueue.buildLocation, type), 0, 0);

	addQueue.type = type;
	//addQueue.Worker = CCmanager[0]->getBuilder();
	addQueue.Worker       = getWorkerBuilder(type, addQueue.buildLocation);
	addQueue.building     = NULL;
	addQueue.Started      = false;
	addQueue.FrameStarted = Broodwar->getFrameCount();
	BuildingsQueue.push_back(addQueue);
	Broodwar->printf("Next building added: %s", type.c_str());
	//BuildingOrder.erase( BuildingOrder.begin() );
}

void ProductionManager::addToQueueClose(BWAPI::UnitType type, BWAPI::TilePosition tile, bool inBase)
{
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();
	BuildQueue addQueue;
	//if (inBase) {
	//	addQueue.buildLocation = bManager->getBuildLocationBase(tile, type, BWTA::getRegion(InfoMan->PosOurBase));
	//} else {
	//	addQueue.buildLocation = bManager->getBuildLocationBase(tile, type, 0);
	//}
	addQueue.buildLocation = Broodwar->getBuildLocation(type, tile);
	//reserve space
	//bManager->mapArea(bManager->getBuildRectangle(addQueue.buildLocation, type), 0, 0);

	addQueue.type = type;
	//addQueue.Worker = CCmanager[0]->getBuilder();
	addQueue.Worker       = getWorkerBuilder(type, addQueue.buildLocation);
	addQueue.building     = NULL;
	addQueue.Started      = false;
	addQueue.FrameStarted = Broodwar->getFrameCount();
	BuildingsQueue.push_back(addQueue);
	Broodwar->printf("Next building added: %s", type.c_str());
	//BuildingOrder.erase( BuildingOrder.begin() );
}

int ProductionManager::TotalTypeInQueue(BWAPI::UnitType type)
{
	int total = 0;
	for (unsigned int i = 0; i < BuildingsQueue.size(); i++) {
		if (BuildingsQueue[i].type == type) {
			total++;
		}
	}

	//maybe there is one in BuildingsQueueType
	if (total == 0 && BuildingsQueueType.size() > 0) {
		for (unsigned int i = 0; i < BuildingsQueueType.size(); i++) {
			if (BuildingsQueueType[i] == type) {
				total++;
			}
		}
	}

	return total;
}

int ProductionManager::TotalUnitInProduction(BWAPI::UnitType type)
{
	//int total = 0;
	//if (type == BWAPI::UnitTypes::Terran_Science_Vessel) {
	//	for (int i = 0; i < StarportCtower.size(); i++) {
	//		if (StarportCtower[i]->getTrainingQueue().size() > 0) {
	//			if (StarportCtower[i]->getTrainingQueue().front() == BWAPI::UnitTypes::Terran_Science_Vessel) {
	//				total++;
	//			}
	//		}
	//	}
	//}
	//return total;
}

BWAPI::Unit ProductionManager::getWorkerBuilder(BWAPI::UnitType type, BWAPI::TilePosition position)
{

	//TODO: make a better selection for both the CC and other buildings
	//if (type == BWAPI::UnitTypes::Terran_Command_Center) {
	//	if (BWTA::getRegion(position) == InfoMan->OurMainregion
	//	    || BWTA::getRegion(position) == InfoMan->NatRegion) {
	//		return CCmanager[0]->getBuilder();
	//	} else {
	//		if (CCmanager.size() > 1) {
	//			return CCmanager[1]->getBuilder();
	//		} else {
	//			return CCmanager[0]->getBuilder();
	//		}
	//	}
	//} else {

	//	if (BWTA::getRegion(position) == InfoMan->OurMainregion) {
	//		return CCmanager[0]->getBuilder();
	//	} else {
	//		if (CCmanager.size() > 1) {
	//			return CCmanager[1]->getBuilder();
	//		} else {
	//			return CCmanager[0]->getBuilder();
	//		}
	//	}
	//}

	return baseManager->getBuilder();
}

void ProductionManager::returnBuilderWorker(BWAPI::Unit Worker)
{

	if (Worker == NULL) {
		return;
	}

	////get nearest base
	//int closesBaseDist = 9999;
	//int closestID      = -1;
	//for (int i = 0; i < CCmanager.size(); i++) {

	//	if (CCmanager[i]->CommandCenter->getPosition().getDistance(Worker->getPosition()) < closesBaseDist
	//	    && CCmanager[i]->BaseReady == true) {
	//		closesBaseDist = CCmanager[i]->CommandCenter->getPosition().getDistance(Worker->getPosition());
	//		closestID      = i;
	//	}
	//}

	//if (closestID != -1) {
	//	CCmanager[closestID]->addWorker(Worker);
	//}

	baseManager->addWorker(Worker);
}

void ProductionManager::checkWorkerBuild()
{

	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {
		return;
	}

	for (unsigned int i = 0; i < BuildingsQueue.size(); i++) {
		if (BuildingsQueue[i].Worker == NULL || !BuildingsQueue[i].Worker->exists()) {
			//Broodwar->printf("No Worker assigned. Reassigning");
			//BuildingsQueue[i].Worker = CCmanager[0]->getBuilder();
			BuildingsQueue[i].Worker = getWorkerBuilder(BuildingsQueue[i].type, BuildingsQueue[i].buildLocation);
			continue;
		}
		BWAPI::Position moveTo = BWAPI::Position(BuildingsQueue[i].buildLocation);

		// start frame not set
		if (BuildingsQueue[i].FrameStarted == 0) {
			BuildingsQueue[i].FrameStarted = Broodwar->getFrameCount();
		}

		//check if the Worker is stuck or otherwise unable to build
		if (Broodwar->getFrameCount() - BuildingsQueue[i].FrameStarted > 2500
		    || ((Broodwar->getFrameCount() - BuildingsQueue[i].FrameStarted > 1500)
		        && Broodwar->self()->minerals() >= BuildingsQueue[i].type.mineralPrice()
		        && Broodwar->self()->gas() >= BuildingsQueue[i].type.gasPrice())) {
			bool buildingExists = true;
			if (BuildingsQueue[i].building == NULL) {
				buildingExists = false;
			}
			if (buildingExists) {
				if (!BuildingsQueue[i].building->exists()) {
					buildingExists = false;
				}
			}
			if (!buildingExists) {
				Broodwar->printf("Worker stuck. Reassigning");
				Unit stuck = BuildingsQueue[i].Worker;
				//BuildingsQueue[i].Worker = CCmanager[0]->getBuilder();
				BuildingsQueue[i].Worker = getWorkerBuilder(BuildingsQueue[i].type, BuildingsQueue[i].buildLocation);
				//CCmanager[0]->addWorker( stuck );
				returnBuilderWorker(stuck);
				BuildingsQueue[i].FrameStarted = Broodwar->getFrameCount();
				continue;
			}
		}

		/*
		if(  !Broodwar->isVisible( BuildingsQueue[i].buildLocation ) ){
		BuildingsQueue[i].Worker->move(moveTo);
		} else {
		BuildingsQueue[i].Worker->build( BuildingsQueue[i].buildLocation , BuildingsQueue[i].type );
		}
		*/
		BWAPI::TilePosition BotRight(BuildingsQueue[i].buildLocation.x + BuildingsQueue[i].type.tileWidth(), BuildingsQueue[i].buildLocation.y + BuildingsQueue[i].type.tileHeight());
		if (!Broodwar->isVisible(BuildingsQueue[i].buildLocation)
		    || !Broodwar->isVisible(BotRight)) {
			BuildingsQueue[i].Worker->move(moveTo);
		} else if (Broodwar->isVisible(BuildingsQueue[i].buildLocation)
		           && BuildingsQueue[i].building == NULL) {
			BuildingsQueue[i].Worker->build(BuildingsQueue[i].type, BuildingsQueue[i].buildLocation);
		} else { //building already placed down
			//if it has been destroyed, rebuild it
			if (!BuildingsQueue[i].building->exists()) {
				BuildingsQueue[i].Worker->build(BuildingsQueue[i].type, BuildingsQueue[i].buildLocation);
			} else if (BuildingsQueue[i].Worker->getOrder() != BWAPI::Orders::ConstructingBuilding) {
				BuildingsQueue[i].Worker->rightClick(BuildingsQueue[i].building);
			}
		}

		//The Worker is currently building
		if (BuildingsQueue[i].building == NULL && BuildingsQueue[i].Worker->getBuildUnit() != NULL) {
			BuildingsQueue[i].building = BuildingsQueue[i].Worker->getBuildUnit();
		} // TODO handle zerg building
		if (BuildingsQueue[i].building != NULL) {
			if (BuildingsQueue[i].Started == false) {
				reservedMinerals -= BuildingsQueue[i].type.mineralPrice();
				reservedGas -= BuildingsQueue[i].type.gasPrice();
				BuildingsQueue[i].Started = true;
			}
			if (BuildingsQueue[i].building->isCompleted()) {
				//CCmanager[0]->addWorker(BuildingsQueue[i].Worker);
				if (Broodwar->self()->getRace() != Races::Zerg)
					returnBuilderWorker(BuildingsQueue[i].Worker);
				//reservedMinerals -= BuildingsQueue[i].type.mineralPrice();
				//reservedGas -= BuildingsQueue[i].type.gasPrice();

				/*
				if(BuildingsQueue[i].type == BWAPI::UnitTypes::Terran_Refinery){
				Broodwar->printf("Refinery done");
				CCmanager[0]->Refinerys.push_back(BuildingsQueue[i].building);
				CCmanager[0]->toGas();
				}
				*/
				//if (BuildingsQueue[i].type == BWAPI::UnitTypes::Terran_Refinery) {
				//	int closestCC = 9999;
				//	int closeID   = 0;
				//	for (unsigned int j = 0; j < CCmanager.size(); j++) {
				//		if (CCmanager[j]->CommandCenter->getPosition().getDistance(BuildingsQueue[i].building->getPosition()) < closestCC) {
				//			closestCC = CCmanager[j]->CommandCenter->getPosition().getDistance(BuildingsQueue[i].building->getPosition());
				//			closeID   = j;
				//		}
				//	}

				//	Broodwar->printf("Refinery done");
				//	CCmanager[closeID]->Refinerys.push_back(BuildingsQueue[i].building);
				//	CCmanager[closeID]->toGas();
				//}

				// remove from BuildingsQueueType and BuildingsQueue
				for (int j = 0; j < BuildingsQueueType.size(); j++) {
					if (BuildingsQueueType[j] == BuildingsQueue[i].type) {
						BuildingsQueueType.erase(BuildingsQueueType.begin() + j);
						break;
					}
				}

				BuildingsQueue.erase(BuildingsQueue.begin() + i);
				i--;
				Broodwar->printf("job done");
			}
		}
	}
}

//void ProductionManager::onFrameMain()
//{
//
//	checkWorkerBuild();
//
//	if (BuildingOrder.size() > 0 && BuildingOrder[0].mineralPrice() <= (Broodwar->self()->minerals() - reservedMinerals)) {
//		//Broodwar->printf("adding possible");
//	}
//	if (BuildingOrder.size() > 0 && BuildingOrder[0].mineralPrice() <= (Broodwar->self()->minerals() - reservedMinerals) && BuildingOrder[0].gasPrice() <= (Broodwar->self()->gas() - reservedGas)) { // && bManager->WallCalculated
//		addToQueue(BuildingOrder[0]);
//		BuildingOrder.erase(BuildingOrder.begin());
//	}
//
//	//build as many wraiths as possible
//	if (StarportNoAddon.size() > 0 || StarportCtower.size() > 0) {
//		bool buildingDepot = false;
//		for (unsigned int i = 0; i < BuildingsQueue.size(); i++) {
//			if (BuildingsQueue[i].type == BWAPI::UnitTypes::Terran_Supply_Depot) {
//				buildingDepot = true;
//			}
//		}
//		if ((Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed()) < 10 && !buildingDepot) {
//			//if( !buildingDepot ){
//			addToQueue(BWAPI::UnitTypes::Terran_Supply_Depot);
//		}
//
//		if (StarportNoAddon.size() > 0 && StarportCtower.size() == 0) {
//			if (StarportNoAddon[0]->getAddon() == NULL) {
//				StarportNoAddon[0]->buildAddon(BWAPI::UnitTypes::Terran_Control_Tower);
//			}
//			/*
//			if( StarportNoAddon[0]->getAddon() != NULL){
//			StarportNoAddon[0]->getAddon()->research( BWAPI::TechTypes::Cloaking_Field);
//			}
//			*/
//		}
//
//		//train wraith
//		for (unsigned int i = 0; i < StarportNoAddon.size(); i++) {
//			if (StarportNoAddon[i]->getTrainingQueue().size() == 0) {
//				StarportNoAddon[i]->train(BWAPI::UnitTypes::Terran_Wraith);
//			}
//		}
//		//train wraith
//		for (unsigned int i = 0; i < StarportCtower.size(); i++) {
//			if (StarportCtower[i]->getTrainingQueue().size() == 0) {
//				StarportCtower[i]->train(BWAPI::UnitTypes::Terran_Wraith);
//			}
//		}
//	}
//
//	// && Broodwar->self()->completedUnitCount( BWAPI::UnitTypes::Terran_Worker) < CCmanager[0]->WorkerSaturation
//	if ((Broodwar->self()->minerals() - reservedMinerals) >= 50) {
//		CCmanager[0]->buildWorker();
//	}
//
//	//time to start training marines
//	if (getAvailableMinerals() >= 200) {
//		bool trainMore = true;
//		if (Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Factory) == 0) {
//			trainMore = false;
//		}
//		for (unsigned int i = 0; i < Barracks.size(); i++) {
//			if (Barracks[i]->getTrainingQueue().size() == 0 && trainMore) {
//				Barracks[i]->train(BWAPI::UnitTypes::Terran_Marine);
//			}
//		}
//		//if we have to many minerals, build another barracks
//		if (StarportNoAddon.size() > 0 && (Broodwar->self()->minerals() - reservedMinerals) >= 250 && BuildingsQueue.size() == 0
//		    && BuildingOrder.size() == 0 && Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Barracks) < 3) {
//			BuildingOrder.push_back(BWAPI::UnitTypes::Terran_Barracks);
//		}
//	}
//}

void ProductionManager::onFrameMacro()
{

	checkWorkerBuild();

	//if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg
	//    && BuildingAtSupplyOrder.size() > 0) {
	//	if (BuildingAtSupplyOrder[0].type == BWAPI::UnitTypes::Terran_Barracks
	//	    && Broodwar->self()->supplyUsed() / 2 == 10) {
	//		addToQueue(BuildingAtSupplyOrder[0].type);
	//		BuildingAtSupplyOrder.erase(BuildingAtSupplyOrder.begin());
	//	}
	//}

	//if( BuildingAtSupplyOrder.size() > 0 && BuildingAtSupplyOrder[0].supply <=  Broodwar->self()->supplyUsed()/2 ){ // && bManager->WallCalculated
	if (BuildingAtSupplyOrder.size() > 0
		&& BuildingAtSupplyOrder[0].type.mineralPrice() <= getAvailableMinerals()
		&& BuildingAtSupplyOrder[0].type.gasPrice() <= getAvailableGas()) {

		addToQueue(BuildingAtSupplyOrder[0].type);
		BuildingAtSupplyOrder.erase(BuildingAtSupplyOrder.begin());
	}

	//future macro build
	if (BuildingAtSupplyOrder.size() == 0) {

		bool buildingDepot = false;
		//if (TotalTypeInQueue(BWAPI::UnitTypes::Terran_Supply_Depot) > 0) {
		//	buildingDepot = true;
		//}

		int DoubleSupplyCount = 140;

		//if ((Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed()) < 10 && !buildingDepot
		//	&& Broodwar->self()->supplyUsed() < DoubleSupplyCount) {
		//	//if( !buildingDepot ){
		//	addToQueue(BWAPI::UnitTypes::Terran_Supply_Depot);
		//}
		//if ((Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed()) < 20 && !buildingDepot
		//	&& Broodwar->self()->supplyUsed() >= DoubleSupplyCount
		//	&& Broodwar->self()->supplyTotal() != 400) {
		//	//if( !buildingDepot ){
		//	Broodwar->printf("Double supply");
		//	addToQueue(BWAPI::UnitTypes::Terran_Supply_Depot);
		//	addToQueue(BWAPI::UnitTypes::Terran_Supply_Depot);
		//}

		//extra buildings

		//expand
		if (getAvailableMinerals() > 300) {
			if (TotalTypeInQueue(BWAPI::UnitTypes::Zerg_Hatchery) == 0) {
				double ClosestToBase = 99999;
				TilePosition newBaseLoc = BWAPI::TilePositions::None;
				//BOOST_FOREACH (BWTA::BaseLocation* BaseLocation, BWTA::getBaseLocations()) {
				for (auto BaseLocation : BWTA::getBaseLocations()) {
					bool already = false;
					//for (int j = 0; j < CCmanager.size(); j++) {
					//	if (BWAPI::Position(CCmanager[j]->LandLocation).getDistance(BaseLocation->getPosition()) < 7 * 32) {
					//		already = true;
					//	}
					//}
					if (Position(baseManager->LandLocation).getDistance(BaseLocation->getPosition()) < 7 * 32)
						already = true;
					if (already == false
						&& ClosestToBase > BaseLocation->getPosition().getDistance(baseManager->CommandCenter->getPosition())) {
						ClosestToBase = BaseLocation->getPosition().getDistance(baseManager->CommandCenter->getPosition());
						newBaseLoc = BaseLocation->getTilePosition();
					}
				}
				if (newBaseLoc != TilePositions::None) {
					addToQueueTile(UnitTypes::Zerg_Hatchery, newBaseLoc);
				}
			}
		}
	}

	//&& Broodwar->self()->completedUnitCount( BWAPI::UnitTypes::Terran_Worker) < CCmanager[0]->WorkerSaturation
	//for (int i = 0; i < CCmanager.size(); i++) {

	//	bool earlyRaxNeeded = false;
	//	if (Broodwar->getFrameCount() < 3000 && TotalTypeInQueue(BWAPI::UnitTypes::Terran_Barracks) > 0) {
	//		earlyRaxNeeded = true;
	//	}
	//	//prioritise building a barracks
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg && earlyRaxNeeded && getAvailableMinerals() < 50) {
	//		continue;
	//	}

	//	//if( (CCmanager[i]->WorkerAndMineralList.size() < CCmanager[i]->WorkerSaturation )
	//	//	&& (getAvailableMinerals() >= 50 || MacroMan->ZState == Z_Pool_Defend )){
	//	if (CCmanager[i]->WorkerAndMineralList.size() < CCmanager[i]->WorkerSaturation) {
	//		CCmanager[i]->buildWorker();
	//	}
	//}

	if (baseManager->WorkerAndMineralList.size() < baseManager->WorkerSaturation) {
		baseManager->buildWorker();
	}

	//for (int i = 0; i < Barracks.size(); i++) {

	//	//&&  (GMan->stillLifting == true || GMan->BarracksLift == true )

	//	if (i == 0
	//	    && (GMan->stillLifting == true)
	//	    && bManager->WallSound) {
	//		if (Barracks[i]->getTrainingQueue().size() != 0) {
	//			Barracks[i]->cancelTrain();
	//		}
	//		continue;
	//	}

	//	BWAPI::UnitType toTrain = MacroMan->BarrackSuggest();
	//	if (toTrain != BWAPI::UnitTypes::None && Barracks[i]->getTrainingQueue().size() == 0) {
	//		Barracks[i]->train(toTrain);
	//	}
	//}

	//for (int i = 0; i < FactoryNoAddon.size(); i++) {
	//	BWAPI::UnitType getSug = MacroMan->FactorySuggest();
	//	if (getSug == BWAPI::UnitTypes::None) {
	//		continue;
	//	}
	//	if (getSug == BWAPI::UnitTypes::Terran_Machine_Shop) {
	//		FactoryNoAddon[i]->buildAddon(BWAPI::UnitTypes::Terran_Machine_Shop);
	//		continue;
	//	}
	//	if (FactoryNoAddon[i]->getTrainingQueue().size() == 0) {
	//		FactoryNoAddon[i]->train(getSug);
	//	}
	//}

	//for (int i = 0; i < FactoryShop.size(); i++) {
	//	BWAPI::UnitType getSug = MacroMan->FactorySuggestShop();
	//	if (FactoryShop[i]->getTrainingQueue().size() == 0) {
	//		FactoryShop[i]->train(getSug);
	//	}
	//	if (FactoryShop[i]->getAddon() != NULL) {
	//		if (Broodwar->enemy()->getRace() == BWAPI::Races::Protoss) {
	//			if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode)) {
	//				FactoryShop[i]->getAddon()->research(BWAPI::TechTypes::Tank_Siege_Mode);
	//			}
	//		}
	//		if (MacroMan->CurrentStrat == "2 rax FE") {
	//			if (MacroMan->Squads[0].Tanks.size() > 0 && !Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode)) {
	//				FactoryShop[i]->getAddon()->research(BWAPI::TechTypes::Tank_Siege_Mode);
	//			}
	//		} else if (MacroMan->CurrentStrat == "Goliath M&M") {
	//			if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Charon_Boosters) == 0) {
	//				FactoryShop[i]->getAddon()->upgrade(BWAPI::UpgradeTypes::Charon_Boosters);
	//			} else if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode)) {
	//				FactoryShop[i]->getAddon()->research(BWAPI::TechTypes::Tank_Siege_Mode);
	//			}
	//		}

	//		if (Broodwar->enemy()->getRace() == BWAPI::Races::Terran) {
	//			if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode)) {
	//				FactoryShop[i]->getAddon()->research(BWAPI::TechTypes::Tank_Siege_Mode);
	//			} else if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Spider_Mines)) {
	//				FactoryShop[i]->getAddon()->research(BWAPI::TechTypes::Spider_Mines);
	//			}
	//		}
	//	}
	//}

	//for (int i = 0; i < StarportNoAddon.size(); i++) {
	//	StarportNoAddon[i]->buildAddon(BWAPI::UnitTypes::Terran_Control_Tower);
	//	/*
	//	if( StarportNoAddon[i]->getTrainingQueue().size() == 0){
	//	BWAPI::UnitType getSug = MacroMan->StarPortNoTSuggest();
	//	StarportNoAddon[i]->train( getSug);
	//	}
	//	*/
	//}

	//for (int i = 0; i < StarportCtower.size(); i++) {
	//	if (StarportCtower[i]->getTrainingQueue().size() == 0) {
	//		BWAPI::UnitType getSug = MacroMan->StarPortCTSuggest();
	//		StarportCtower[i]->train(getSug);
	//	}
	//}

	//if (Academy != NULL) {
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg) {
	//		if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Stim_Packs)) {
	//			Academy->research(BWAPI::TechTypes::Stim_Packs);
	//		} else if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells) == 0) {
	//			Academy->upgrade(BWAPI::UpgradeTypes::U_238_Shells);
	//		}
	//	}

	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Protoss) {
	//		if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Stim_Packs)) {
	//			Academy->research(BWAPI::TechTypes::Stim_Packs);
	//		} else if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells) == 0) {
	//			Academy->upgrade(BWAPI::UpgradeTypes::U_238_Shells);
	//		}
	//	}
	//}

	//if (EngineeringBay != NULL) {
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Protoss) {
	//		if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Weapons) == 0) {
	//			EngineeringBay->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
	//		} else if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Armor) == 0) {
	//			EngineeringBay->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Armor);
	//		}
	//	}
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg) {
	//		if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Weapons) == 0) {
	//			EngineeringBay->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
	//		} else if (Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Armor) == 0) {
	//			EngineeringBay->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Armor);
	//		}
	//	}
	//}

	//if (ScienceFacility != NULL) {

	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg) {
	//		if (!Broodwar->self()->hasResearched(BWAPI::TechTypes::Irradiate)) {
	//			ScienceFacility->research(BWAPI::TechTypes::Irradiate);
	//		} else if (ScienceFacility->getAddon() == NULL) {
	//			ScienceFacility->buildAddon(BWAPI::UnitTypes::Terran_Physics_Lab);
	//		}
	//		if (ScienceFacility->getAddon() != NULL) {
	//			//ScienceFacility->getAddon()->research( BWAPI::TechTypes::Yamato_Gun);
	//		}
	//	}

	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Terran) {
	//		if (ScienceFacility->getAddon() == NULL) {
	//			ScienceFacility->buildAddon(BWAPI::UnitTypes::Terran_Physics_Lab);
	//		}
	//		if (ScienceFacility->getAddon() != NULL) {
	//			//ScienceFacility->getAddon()->research( BWAPI::TechTypes::Yamato_Gun);
	//		}
	//	}
	//}
}

void ProductionManager::onFrame()
{

	if (reservedMinerals < 0) {
		reservedMinerals = 0;
	}
	if (reservedGas < 0) {
		reservedGas = 0;
	}

	Broodwar->drawTextScreen(300, 60, "Reserved minerals:%d", reservedMinerals);
	Broodwar->drawTextScreen(300, 70, "Reserved gas:%d", reservedGas);

	Broodwar->drawTextScreen(300, 80, "Current Worker:%d", baseManager->WorkerAndMineralList.size());
	Broodwar->drawTextScreen(300, 90, "Saturation Worker:%d", baseManager->WorkerSaturation);

	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {
		return;
	}

	//if (CurrentStrategy == Macro_Strat) {
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg) {
	//		onFrameMacroZerg();
	//	} else if (Broodwar->enemy()->getRace() == BWAPI::Races::Protoss) {
	//		onFrameMacroProtoss();
	//	} else if (Broodwar->enemy()->getRace() == BWAPI::Races::Terran) {
	//		onFrameMacroTerran();
	//	} else if (Broodwar->enemy()->getRace() == BWAPI::Races::Random
	//	           || Broodwar->enemy()->getRace() == BWAPI::Races::Unknown) {
	//		onFrameMacroRandom();
	//	} else {
	//		onFrameMacro();
	//	}
	//}
	//if (CurrentStrategy == Worker_Rush) {
	//	onFrameWorkerRush();
	//}

	onFrameMacro();
}

void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{

	if (unit->getPlayer() != Broodwar->self() || !unit->getType().isBuilding()) {
		return;
	}

	if (unit->getType().isAddon()) {
		return;
	}

	//rebuild the building
	//addToQueue( unit->getType() );

	if (unit->getType() == BWAPI::UnitTypes::Terran_Barracks) {
		addToQueue(unit->getType());
	}
}

void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{

	if (unit->getPlayer() == Broodwar->enemy()) {
		return;
	}

	// && Broodwar->enemy()->getRace() == BWAPI::Races::Protoss
	//if ((CurrentStrategy == Macro_Strat && Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	//    || (CurrentStrategy == Macro_Strat && Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	//        && Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Supply_Depot) == 2) {
	//	if (GMan->alreadySupply != true) {
	//		GMan->BarracksLift  = true;
	//		GMan->alreadySupply = true;
	//	}
	//}

	/*
	if( unit->getType().isBuilding() ){
	reservedMinerals -= unit->getType().mineralPrice();
	reservedGas -= unit->getType().gasPrice();
	}
	*/

	/*
	if( unit->getType() == BWAPI::UnitTypes::Terran_Refinery ){
	Broodwar->printf("Refinery done");
	CCmanager[0]->Refinerys.push_back(unit);
	CCmanager[0]->toGas();
	}
	*/

	/*
	if( unit->getType() == BWAPI::UnitTypes::Terran_Refinery ){

	int closestCC = 999;
	int closeID = 0;
	for( unsigned int j=0; j<CCmanager.size(); j++ ){
	if(  CCmanager[j]->CommandCenter->getPosition().getDistance(unit->getPosition()) < closestCC ){
	closestCC = CCmanager[j]->CommandCenter->getPosition().getDistance(unit->getPosition());
	closeID = j;
	}
	}

	Broodwar->printf("Refinery done");
	CCmanager[closeID]->Refinerys.push_back( unit );
	CCmanager[closeID]->toGas();

	}
	*/

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Armory) {
	//	Armory.push_back(unit);
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Science_Facility) {
	//	ScienceFacility = unit;
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Command_Center) {
	//	bool alreadyExists = false;
	//	for (unsigned int i = 0; i < CCmanager.size(); i++) {
	//		//no double entry
	//		if (CCmanager[i]->CommandCenter->getID() == unit->getID()) {
	//			alreadyExists = true;
	//		}
	//	}
	//	if (!alreadyExists) {
	//		//float if needed
	//		bool alreadBaseloc = false;
	//		BOOST_FOREACH (BWTA::BaseLocation* baseLoc, BWTA::getBaseLocations()) {
	//			if (unit->getTilePosition() == baseLoc->getTilePosition()) {
	//				alreadBaseloc = true;
	//				break;
	//			}
	//		}
	//		if (alreadBaseloc == false) {
	//			BaseManager* newBase = new BaseManager(unit, InfoMan->OurNat);
	//			CCmanager.push_back(newBase);
	//		} else {
	//			BaseManager* newBase = new BaseManager(unit, unit->getTilePosition());
	//			CCmanager.push_back(newBase);
	//		}
	//		/*
	//		if( Broodwar->self()->completedUnitCount(  BWAPI::UnitTypes::Terran_Command_Center ) == 2 ){
	//		BaseManager* newBase = new BaseManager( unit, InfoMan->OurNat );
	//		CCmanager.push_back( newBase );
	//		} else {
	//		BaseManager* newBase = new BaseManager( unit, unit->getTilePosition() );
	//		CCmanager.push_back( newBase );
	//		}
	//		*/
	//	}
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Engineering_Bay) {
	//	Broodwar->printf("Added engineering bay");
	//	EngineeringBay = unit;
	//	if (Broodwar->enemy()->getRace() == BWAPI::Races::Protoss) {
	//		addToQueueClose(BWAPI::UnitTypes::Terran_Missile_Turret, BWAPI::TilePosition(InfoMan->PosMainChoke));
	//		addToQueue(BWAPI::UnitTypes::Terran_Missile_Turret);
	//	}
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Starport) {
	//	Broodwar->printf("Added starport");
	//	StarportNoAddon.push_back(unit);
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Control_Tower) {
	//	Broodwar->printf("Control tower done");
	//	for (unsigned int i = 0; i < StarportNoAddon.size(); i++) {
	//		if (StarportNoAddon[i]->getAddon() != NULL) {
	//			StarportCtower.push_back(StarportNoAddon[i]);
	//			StarportNoAddon.erase(StarportNoAddon.begin() + i);
	//			break;
	//		}
	//	}
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Factory) {
	//	FactoryNoAddon.push_back(unit);
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Machine_Shop) {
	//	for (unsigned int i = 0; i < FactoryNoAddon.size(); i++) {
	//		if (FactoryNoAddon[i]->getAddon() != NULL) {
	//			FactoryShop.push_back(FactoryNoAddon[i]);
	//			FactoryNoAddon.erase(FactoryNoAddon.begin() + i);
	//			break;
	//		}
	//	}
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Barracks) {
	//	Barracks.push_back(unit);
	//}

	//if (unit->getType() == BWAPI::UnitTypes::Terran_Academy) {
	//	Academy = unit;
	//	/*
	//	if( CurrentStrategy == Bio_8_Rax ){
	//	CCmanager[0]->toGas(2);//put two more on gas
	//	}
	//	*/

	//	if (MacroMan->CurrentStrat == "2 rax FE") {
	//		//addToQueue( BWAPI::UnitTypes::Terran_Missile_Turret);
	//		//addToQueue( BWAPI::UnitTypes::Terran_Missile_Turret);
	//		//get tile position between CC and mineral field
	//		//BWAPI::TilePosition Tur1( InfoMan->OurNat.x + 1, InfoMan->OurNat.y - 2);
	//		//BWAPI::TilePosition Tur2( InfoMan->OurNat.x + 1, InfoMan->OurNat.y + 4);
	//		//addToQueueTile( BWAPI::UnitTypes::Terran_Missile_Turret,Tur1);
	//		//addToQueueTile( BWAPI::UnitTypes::Terran_Missile_Turret,Tur2);
	//	}
	//}

	if (unit->getType().isWorker()) {
		bool alreadyAssigned = false;
		//if (ScoutMan != NULL) {
		//	for (unsigned int i = 0; i < ScoutMan->workerScouts.size(); i++) {
		//		if (ScoutMan->workerScouts[i].Worker->getID() == unit->getID()) {
		//			alreadyAssigned = true;
		//		}
		//	}
		//}
		for (unsigned int j = 0; j < this->BuildingsQueue.size(); j++) {
			if (this->BuildingsQueue[j].Worker != NULL) {
				if (this->BuildingsQueue[j].Worker->getID() == unit->getID()) {
					alreadyAssigned = true;
				}
			}
		}
		if (!alreadyAssigned) {
			//find the command center that this unit belongs to
			//int closestCC = 999;
			//int closeID   = 0;
			//for (unsigned int j = 0; j < CCmanager.size(); j++) {
			//	if (CCmanager[j]->CommandCenter->getPosition().getDistance(unit->getPosition()) < closestCC) {
			//		closestCC = CCmanager[j]->CommandCenter->getPosition().getDistance(unit->getPosition());
			//		closeID   = j;
			//	}
			//}
			//CCmanager[closeID]->addWorker(unit);
			baseManager->addWorker(unit);
		}
	}
}