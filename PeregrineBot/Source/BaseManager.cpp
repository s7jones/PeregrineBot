#include "BaseManager.h"

using namespace BWAPI;

bool LanLatency = true;
std::vector<BaseManager*> CCmanager;

bool isLeft(Position a, Position b, Position c)
{
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) >= 0;
}

//void BaseManager::CalculateTrickChoice()
//{
//	if (LanLatency == true) {
//		CalculateTrick();
//	}
//}
// COMMENTED BY SAM

BaseManager::BaseManager(Unit CC, TilePosition land)
{

	//Broodwar << "added command center" << std::endl; // COMMENTED BY SAM

	InitialMinerals = 0; // will be added on addMineralField()
	WorkersonGas    = 3;
	cycleMineral    = 0;
	WorkerAndMineralList.clear();
	CommandCenter = CC;
	LandLocation  = land;
	buildComsat   = false;

	if (LandLocation == CC->getTilePosition()) {
		BaseReady = true;
	} else {
		BaseReady = false;
	}

	Position landPos = Position(land);

	if (BaseReady == true) {

		/*
		for(std::set<Unit>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
		{
		if ( CommandCenter->getDistance(*m)<  11*32 ){
		// Minerals.push_back( *m);
		addMineralField(*m);
		}
		}
		*/

		Unitset Minerals = CommandCenter->getUnitsInRadius(15 * 32, Filter::IsMineralField);

		// BOOST_FOREACH (Unit mineral, GetMinerals) {
		for (auto mineral : Minerals) {
			addMineralField(mineral);
		}

		//CalculateTrickChoice(); // COMMENTED BY SAM
		// for (std::set<Unit>::iterator g = Broodwar->getGeysers().begin(); g != Broodwar->getGeysers().end(); g++) {
		for (auto geyser : Broodwar->getGeysers()) {
			if (CommandCenter->getDistance(geyser) < 11 * 32) {
				Geysers.insert(geyser);
				// Broodwar->printf("added geyser");
			}
		}
		// WorkerSaturation = Minerals.size() * 2 + 2; // force gather trick is almost saturated at 2 times minerals
	}

	/*
	for(unsigned int i=0; i<InfoMan->Minerals.size(); i++){
	if ( landPos.getDistance( InfoMan->Minerals[i]->getPosition() )<  300 ){
	Minerals.push_back( InfoMan->Minerals[i] );
	}
	}
	for(unsigned int i=0; i<InfoMan->Geysers.size(); i++){
	if ( landPos.getDistance( InfoMan->Geysers[i]->getPosition() )<  300 ){
	Geysers.push_back( InfoMan->Geysers[i] );
	Broodwar->printf("added geyser");
	}
	}

	*/
	/*
	for(std::set<Unit>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
	{
	if ( CommandCenter->getDistance(*m)<  300 ){
	Minerals.push_back( *m);
	}
	}
	for(std::set<Unit>::iterator g=Broodwar->getGeysers().begin();g!=Broodwar->getGeysers().end();g++)
	{
	if ( CommandCenter->getDistance(*g)<  300 ){
	Geysers.push_back(*g);
	Broodwar->printf("added geyser");
	}
	}
	*/

	// WorkerSaturation = Minerals.size() * 2.5; // build in is (almost) saturated at 2.5 times minerals
	// WorkerSaturation = Minerals.size() * 2 + 2; // force gather trick is almost saturated at 2 times minerals

	/*
	// maynarding
	if (BaseReady == true && Broodwar->self()->completedUnitCount(UnitTypes::Terran_Command_Center) == 2) {
		for (int i = 0; i < 6; i++) {
			addWorker(CCmanager[0]->getBuilder());
		}
		// add a refinery
		if (Geysers.size() > 0) {
			// ||  (Broodwar->enemy()->getRace() == Races::Zerg && CCmanager.size() >= 2 )
			if (Broodwar->enemy()->getRace() != Races::Zerg) {
				//ProdMan->addToQueueTile(UnitTypes::Terran_Refinery, Geysers[0]->getTilePosition()); // COMMENTED BY SAM
			}
			// ProdMan->addToQueueTile( UnitTypes::Terran_Refinery, Geysers[0]->getTilePosition() );
		}
	}

	if (BaseReady == true && Broodwar->self()->completedUnitCount(UnitTypes::Terran_Command_Center) > 2) {
		if (Geysers.size() > 0) {
			// if( Broodwar->enemy()->getRace() != Races::Zerg ){
			//ProdMan->addToQueueTile(UnitTypes::Terran_Refinery, Geysers[0]->getTilePosition()); // COMMENTED BY SAM
			//}
			// ProdMan->addToQueueTile( UnitTypes::Terran_Refinery, Geysers[0]->getTilePosition() );
		}
	}
	*/ // COMMENTED BY SAM
}

void BaseManager::addWorker(Unit worker)
{

	//if (worker == NULL) {
	//	return;
	//}
	// COMMENTED BY SAM

	/*
	worker->rightClick( Minerals[cycleMineral] );
	cycleMineral++;
	cycleMineral = cycleMineral % Minerals.size();
	WorkerAndMineralList.push_back( worker );
	*/

	// check if the worker is already assigned
	for (auto currentWorkerAndMineral : WorkerAndMineralList) {
		if (currentWorkerAndMineral.worker->getID() == worker->getID()) {
			// Broodwar->printf("worker already stored");
			return;
		}
	}

	WorkerAndMineral newWorker;
	newWorker.worker             = worker;
	newWorker.mineralPatch       = NULL;
	newWorker.useTrick           = false;
	int leastWorkersOnMineral    = 999;
	MineralPatch* closestMineral = NULL;
	for (auto currentMineral : Minerals) {
		int workersOnMineral = 0;
		for (auto currentWorkerAndMineral : WorkerAndMineralList) {
			if (currentWorkerAndMineral.mineralPatch == NULL) {
				//Broodwar->printf("assignment problems mineral worker"); // COMMENTED BY SAM
				continue;
			}
			if (currentWorkerAndMineral.mineralPatch->getID() == currentMineral.mineralPatch->getID()) {
				workersOnMineral++;
			}
		}
		if (workersOnMineral < leastWorkersOnMineral) {
			leastWorkersOnMineral = workersOnMineral;
			closestMineral        = &currentMineral;
		} // TODO: make closestMineral not least but least and closest!!
	}

	if (closestMineral != NULL) {
		newWorker.mineralPatch = closestMineral->mineralPatch;
		if (closestMineral->mineralTrick != NULL) {
			newWorker.useTrick     = true;
			newWorker.mineralTrick = closestMineral->mineralTrick;
		}
	}
	if (newWorker.mineralPatch != NULL) {
		newWorker.worker->gather(newWorker.mineralPatch);
	} else {
		Broodwar << "new worker doesn't have a mineral patch" << std::endl;
	}
	WorkerAndMineralList.insert(newWorker);
}

void BaseManager::buildWorker()
{

	if (BaseReady == false) {
		return;
	}

	if (WorkerAndMineralList.size() >= WorkerSaturation) {
		return;
	}

	/*
	if (ProdMan->Academy != NULL && CommandCenter->getAddon() == NULL
	    && buildComsat == true) {
		return; // build comsat station first
	}
	*/


	CommandCenter->train(UnitTypes::Zerg_Drone);
}

void BaseManager::addMineralField(Unit mineral)
{
	InitialMinerals++;

	// Minerals.push_back(mineral);
	bool isPathTrick = false;

	/*
	for(int i=0; i<InfoMan->FSmineral.MineralID.size(); i++){
	if( InfoMan->FSmineral.MineralID[i] == mineral->getID() ){
	isPathTrick = true;
	}
	}
	*/
	isPathTrick = false;

	MineralPatch newpatch;
	newpatch.trickPossible = isPathTrick;
	newpatch.mineralPatch  = mineral;
	newpatch.mineralTrick  = NULL; // still has to be determined at calculate trick
	Minerals.insert(newpatch);
}

//void BaseManager::CalculateTrick()
//{
//	/*
//	for(int i=0; i<Minerals.size(); i++){
//	for(int j=0; j<Minerals.size(); j++){
//	if( i != j && Minerals[j].trickPossible && !Minerals[i].trickPossible
//	&& Minerals[i].mineralPatch->getDistance( Minerals[j].mineralPatch ) < 1*32 ){
//	Minerals[i].mineralTrick = Minerals[j].mineralPatch;
//	}
//	}
//	}
//	*/
//
//	int GetMineralPos[5] = { 0, 0, 0, 0, 0 };
//
//	for (int i = 0; i < Minerals.size(); i++) {
//
//		// determine orientation for mineral field, 1=up,2=bottom,3=left,4=right
//		int orientation = 0;
//		Position TopLeft(CommandCenter->getPosition().x - 32 * 10, CommandCenter->getPosition().y - 32 * 10);
//		Position BottomRight(CommandCenter->getPosition().x + 32 * 10, CommandCenter->getPosition().y + 32 * 10);
//		Position BottomLeft(CommandCenter->getPosition().x - 32 * 10, CommandCenter->getPosition().y + 32 * 10);
//		Position TopRight(CommandCenter->getPosition().x + 32 * 10, CommandCenter->getPosition().y - 32 * 10);
//		bool isUpRight    = false;
//		bool isBottomLeft = false;
//		if (isLeft(TopLeft, BottomRight, Minerals[i].mineralPatch->getPosition())) {
//			isBottomLeft = true;
//		}
//		if (isLeft(BottomLeft, TopRight, Minerals[i].mineralPatch->getPosition())) {
//			isUpRight = true;
//		}
//		if (isBottomLeft == true && isUpRight == true) {
//			orientation = 2;
//		}
//		if (isBottomLeft == true && isUpRight == false) {
//			orientation = 3;
//		}
//		if (isBottomLeft == false && isUpRight == true) {
//			orientation = 4;
//		}
//		if (isBottomLeft == false && isUpRight == false) {
//			orientation = 1;
//		}
//		GetMineralPos[orientation]++;
//	}
//
//	int MineralOrient = 0;
//	int bestCount     = 0;
//	for (int i = 1; i <= 4; i++) {
//		if (GetMineralPos[i] > bestCount) {
//			bestCount     = GetMineralPos[i];
//			MineralOrient = i;
//		}
//	}
//
//	for (int i = 0; i < Minerals.size(); i++) {
//
//		// determine orientation for mineral field, 1=up,2=bottom,3=left,4=right
//		int orientation = 0;
//		Position TopLeft(CommandCenter->getPosition().x - 32 * 10, CommandCenter->getPosition().y - 32 * 10);
//		Position BottomRight(CommandCenter->getPosition().x + 32 * 10, CommandCenter->getPosition().y + 32 * 10);
//		Position BottomLeft(CommandCenter->getPosition().x - 32 * 10, CommandCenter->getPosition().y + 32 * 10);
//		Position TopRight(CommandCenter->getPosition().x + 32 * 10, CommandCenter->getPosition().y - 32 * 10);
//		bool isUpRight    = false;
//		bool isBottomLeft = false;
//		if (isLeft(TopLeft, BottomRight, Minerals[i].mineralPatch->getPosition())) {
//			isBottomLeft = true;
//		}
//		if (isLeft(BottomLeft, TopRight, Minerals[i].mineralPatch->getPosition())) {
//			isUpRight = true;
//		}
//		if (isBottomLeft == true && isUpRight == true) {
//			orientation = 2;
//		}
//		if (isBottomLeft == true && isUpRight == false) {
//			orientation = 3;
//		}
//		if (isBottomLeft == false && isUpRight == true) {
//			orientation = 4;
//		}
//		if (isBottomLeft == false && isUpRight == false) {
//			orientation = 1;
//		}
//
//		int TotalTrick = 0;
//		for (int j = 0; j < Minerals.size(); j++) {
//			if (i != j && Minerals[i].mineralPatch->getDistance(Minerals[j].mineralPatch) <= 1 * 16) {
//				if (orientation == 1 && MineralOrient == 1) {
//					TotalTrick++;
//				}
//				if (orientation == 2 && MineralOrient == 2) {
//					// Broodwar->printf("Added trick bottom");
//					TotalTrick++;
//				}
//				if (orientation == 3 && MineralOrient == 3) {
//					// Broodwar->printf("Added trick left");
//					TotalTrick++;
//				}
//				if (orientation == 4 && MineralOrient == 4) {
//					// Broodwar->printf("Added trick right");
//					TotalTrick++;
//				}
//				// Minerals[i].mineralTrick = Minerals[j].mineralPatch;
//			}
//		}
//		/*
//		if( TotalTrick != 2){
//		continue;
//		}
//		*/
//		for (int j = 0; j < Minerals.size(); j++) {
//			if (i != j && Minerals[i].mineralPatch->getDistance(Minerals[j].mineralPatch) <= 1 * 16) {
//				if (orientation == 1 && MineralOrient == 1) {
//					// Broodwar->printf("Added trick up");
//					if (Minerals[i].mineralPatch->getPosition().y < Minerals[j].mineralPatch->getPosition().y) {
//						Minerals[j].trickPossible = true;
//						Minerals[j].mineralTrick  = Minerals[i].mineralPatch;
//					}
//				}
//				if (orientation == 2 && MineralOrient == 2) {
//					// Broodwar->printf("Added trick bottom");
//					if (Minerals[i].mineralPatch->getPosition().y > Minerals[j].mineralPatch->getPosition().y) {
//						Minerals[j].trickPossible = true;
//						Minerals[j].mineralTrick  = Minerals[i].mineralPatch;
//					}
//				}
//				if (orientation == 3 && MineralOrient == 3) {
//					// Broodwar->printf("Added trick left");
//					if (Minerals[i].mineralPatch->getPosition().x < Minerals[j].mineralPatch->getPosition().x) {
//						Minerals[j].trickPossible = true;
//						Minerals[j].mineralTrick  = Minerals[i].mineralPatch;
//					}
//				}
//				if (orientation == 4 && MineralOrient == 4) {
//					// Broodwar->printf("Added trick right");
//					if (Minerals[i].mineralPatch->getPosition().x > Minerals[j].mineralPatch->getPosition().x) {
//						Minerals[j].trickPossible = true;
//						Minerals[j].mineralTrick  = Minerals[i].mineralPatch;
//					}
//				}
//				// Minerals[i].mineralTrick = Minerals[j].mineralPatch;
//			}
//		}
//	}
//}
// COMMENTED BY SAM

//void BaseManager::MineralGather()
//{
//
//	for (unsigned int i = 0; i < WorkerAndMineralList.size(); i++) {
//		if (!WorkerAndMineralList[i].worker->exists()) { // remove destroyed workers
//			WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//			i--;
//			continue;
//		}
//		// check if worker is not used in constructing buildings
//		bool alreadyBuilding = false;
//		/*
//		for (unsigned int j = 0; j < ProdMan->BuildingsQueue.size(); j++) {
//			if (ProdMan->BuildingsQueue[j].worker != NULL) {
//				if (ProdMan->BuildingsQueue[j].worker->getID() == WorkerAndMineralList[i].worker->getID()) {
//					alreadyBuilding = true;
//				}
//			}
//		}
//		*/ // COMMENTED BY SAM
//		if (alreadyBuilding) {
//			Broodwar->printf("worker already assigned to building");
//			WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//			i--;
//			continue;
//		}
//
//		// no mineral patches availabe anymore
//		if (WorkerAndMineralList[i].mineralPatch == NULL) {
//			// Broodwar->printf("No mineral patch assigned to worker");
//			// TODO: give this worker another task
//			continue;
//		}
//
//		// reassign Worker when current mineral patch has run out
//		if (WorkerAndMineralList[i].mineralPatch != NULL) {
//			if (!WorkerAndMineralList[i].mineralPatch->exists()) {
//				Broodwar->printf("Mineral patch used up. Reassigning Worker");
//				Unit thisWorker = WorkerAndMineralList[i].worker;
//				WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//				addWorker(thisWorker);
//				break; // TODO: make this a continue that doesn't mess up the WorkerAndMineralList loop
//			}
//		}
//
//		if (WorkerAndMineralList[i].useTrick == false) {
//			if (!WorkerAndMineralList[i].worker->isGatheringMinerals()) {
//				WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralPatch);
//			}
//
//			// worker currently working at the correct mineral patch
//			if (WorkerAndMineralList[i].worker->isCarryingMinerals()) {
//				if (WorkerAndMineralList[i].worker->getOrder() != Orders::ReturnMinerals) {
//					WorkerAndMineralList[i].worker->returnCargo();
//					continue;
//				}
//				continue;
//			}
//			if (WorkerAndMineralList[i].worker->getOrderTarget() == NULL) {
//				WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralPatch);
//				continue;
//			}
//
//			// worker going to the wrong mineral patch
//			if (WorkerAndMineralList[i].worker->getOrderTarget()->getID() != WorkerAndMineralList[i].mineralPatch->getID()) {
//				// Broodwar->printf("reassinging worker");
//				WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralPatch);
//			}
//		} else if (WorkerAndMineralList[i].mineralTrick != NULL) { // always gotta check for null pointers
//
//			// worker currently working at the correct mineral patch
//			if (WorkerAndMineralList[i].worker->isCarryingMinerals()) {
//				if (WorkerAndMineralList[i].worker->getOrder() != Orders::ReturnMinerals) {
//					WorkerAndMineralList[i].worker->returnCargo();
//					continue;
//				}
//				continue;
//			}
//
//			// use path trick as long as the Worker isn't close to the actual mineral field
//			if (WorkerAndMineralList[i].mineralPatch->getDistance(WorkerAndMineralList[i].worker->getPosition()) > 40) {
//				// don't spam
//				if (WorkerAndMineralList[i].worker->getTarget() != NULL) {
//					if (WorkerAndMineralList[i].worker->getTarget()->getID() == WorkerAndMineralList[i].mineralTrick->getID()) {
//						continue;
//					}
//				}
//				WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralTrick);
//				continue;
//			}
//
//			// use path trick as long as the Worker isn't close to the actual mineral field
//			if (WorkerAndMineralList[i].mineralPatch->getDistance(WorkerAndMineralList[i].worker->getPosition()) <= 40) {
//				if (WorkerAndMineralList[i].worker->getTarget() != NULL) {
//					if (WorkerAndMineralList[i].worker->getTarget()->getID() == WorkerAndMineralList[i].mineralPatch->getID()) {
//						continue;
//					}
//				}
//				WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralPatch);
//				continue;
//			}
//
//			/*
//			if( !WorkerAndMineralList[i].worker->isGatheringMinerals() ){
//			WorkerAndMineralList[i].worker->gather( WorkerAndMineralList[i].mineralPatch );
//			}
//
//			if(  WorkerAndMineralList[i].worker->getOrderTarget() == NULL  ){
//			WorkerAndMineralList[i].worker->gather( WorkerAndMineralList[i].mineralPatch );
//			continue;
//			}
//
//			// worker going to the wrong mineral patch
//			if( WorkerAndMineralList[i].worker->getOrderTarget()->getID()  !=  WorkerAndMineralList[i].mineralPatch->getID()){
//			// Broodwar->printf("reassinging worker");
//			WorkerAndMineralList[i].worker->gather( WorkerAndMineralList[i].mineralPatch );
//			}
//			*/
//		}
//		/*
//		// set idle workers to mine
//		if(  !WorkerAndMineralList[i].worker->isGatheringMinerals() ){
//
//		WorkerAndMineralList[i]->rightClick( Minerals[cycleMineral] );
//		cycleMineral++;
//		cycleMineral = cycleMineral % Minerals.size();
//
//		}
//		*/
//	}
//} // COMMENTED BY SAM

void BaseManager::MineralGatherLock()
{
	for (auto currentWorkerAndMineral : WorkerAndMineralList) {
		if (!currentWorkerAndMineral.worker->exists()) { // remove destroyed workers
			WorkerAndMineralList.erase(currentWorkerAndMineral);
			continue;
		}
		// check if worker is not used in constructing buildings
		bool alreadyBuilding = false;
		/*
		for (unsigned int j = 0; j < ProdMan->BuildingsQueue.size(); j++) {
			if (ProdMan->BuildingsQueue[j].worker != NULL) {
				if (ProdMan->BuildingsQueue[j].worker->getID() == WorkerAndMineralList[i].worker->getID()) {
					alreadyBuilding = true;
				}
			}
		}
		*/ // COMMENTED BY SAM
		if (alreadyBuilding) {
			//Broodwar->printf("worker already assigned to building"); // COMMENTED BY SAM
			WorkerAndMineralList.erase(currentWorkerAndMineral);
			continue;
		}

		// no mineral patches available anymore
		if (currentWorkerAndMineral.mineralPatch == NULL) {
			// Broodwar->printf("No mineral patch assigned to worker");
			// TODO: give this worker another task
			continue;
		}

		// reassign Worker when current mineral patch has run out
		if (currentWorkerAndMineral.mineralPatch != NULL) {
			if (!currentWorkerAndMineral.mineralPatch->exists()) {
				//Broodwar->printf("Mineral patch used up. Reassigning Worker"); // COMMENTED BY SAM
				Unit thisWorker = currentWorkerAndMineral.worker;
				WorkerAndMineralList.erase(currentWorkerAndMineral);
				addWorker(thisWorker);
				break; // TODO: make this a continue that doesn't mess up the WorkerAndMineralList loop
				       // TODO: Sam's edit: Can I change to a continue now that I've changed to a set?
			}
		}

		if (!currentWorkerAndMineral.worker->isGatheringMinerals()) {
			currentWorkerAndMineral.worker->gather(currentWorkerAndMineral.mineralPatch);
			// continue;
		}

		// worker currently working at the correct mineral patch
		if (currentWorkerAndMineral.worker->isCarryingMinerals()) {
			if (currentWorkerAndMineral.worker->getOrder() != Orders::ReturnMinerals) {
				currentWorkerAndMineral.worker->returnCargo();
				continue;
			}
			continue;
		}
		if (currentWorkerAndMineral.worker->getOrderTarget() == NULL) {
			currentWorkerAndMineral.worker->gather(currentWorkerAndMineral.mineralPatch);
			continue;
		}

		// worker going to the wrong mineral patch
		if (currentWorkerAndMineral.worker->getOrderTarget()->getID() != currentWorkerAndMineral.mineralPatch->getID()) {
			// Broodwar->printf("reassinging worker");
			currentWorkerAndMineral.worker->gather(currentWorkerAndMineral.mineralPatch);
		}

		/*
		// set idle workers to mine
		if(  !currentWorkerAndMineral.worker->isGatheringMinerals() ){

		currentWorkerAndMineral->rightClick( Minerals[cycleMineral] );
		cycleMineral++;
		cycleMineral = cycleMineral % Minerals.size();

		}
		*/
	}
}

//void BaseManager::MineralGatherNoTrick()
//{
//
//	for (unsigned int i = 0; i < WorkerAndMineralList.size(); i++) {
//		if (!WorkerAndMineralList[i].worker->exists()) { // remove destroyed workers
//			WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//			i--;
//			continue;
//		}
//		// check if worker is not used in constructing buildings
//		bool alreadyBuilding = false;
//		/*
//		for (unsigned int j = 0; j < ProdMan->BuildingsQueue.size(); j++) {
//			if (ProdMan->BuildingsQueue[j].worker != NULL) {
//				if (ProdMan->BuildingsQueue[j].worker->getID() == WorkerAndMineralList[i].worker->getID()) {
//					alreadyBuilding = true;
//				}
//			}
//		}
//		*/ // COMMENTED BY SAM
//		if (alreadyBuilding) {
//			Broodwar->printf("worker already assigned to building");
//			WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//			i--;
//			continue;
//		}
//
//		// no mineral patches availabe anymore
//		if (WorkerAndMineralList[i].mineralPatch == NULL) {
//			// Broodwar->printf("No mineral patch assigned to worker");
//			// TODO: give this worker another task
//			continue;
//		}
//
//		// reassign Worker when current mineral patch has run out
//		if (WorkerAndMineralList[i].mineralPatch != NULL) {
//			if (!WorkerAndMineralList[i].mineralPatch->exists()) {
//				Broodwar->printf("Mineral patch used up. Reassigning Worker");
//				Unit thisWorker = WorkerAndMineralList[i].worker;
//				WorkerAndMineralList.erase(WorkerAndMineralList.begin() + i);
//				addWorker(thisWorker);
//				break; // TODO: make this a continue that doesn't mess up the WorkerAndMineralList loop
//			}
//		}
//
//		if (!WorkerAndMineralList[i].worker->isGatheringMinerals()) {
//			WorkerAndMineralList[i].worker->gather(WorkerAndMineralList[i].mineralPatch);
//		}
//
//		if (WorkerAndMineralList[i].worker->isCarryingMinerals()) {
//			if (WorkerAndMineralList[i].worker->getOrder() != Orders::ReturnMinerals) {
//				WorkerAndMineralList[i].worker->returnCargo();
//				continue;
//			}
//			continue;
//		}
//	}
//}
// COMMENTED BY SAM

void BaseManager::MineralGatherChoice()
{

	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {
		return;
	}
	/*
	if (LanLatency == true) {
		// MineralGather(); // TODO: compensate for latency
		MineralGatherLock();
	} else {
		MineralGatherNoTrick();
	}
	*/ // COMMENTED BY SAM
}

//void BaseManager::LiftToNat()
//{
//
//	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {
//		return;
//	}
//
//	if (!CommandCenter->isLifted()) {
//		CommandCenter->lift();
//	}
//
//	if (CommandCenter->isLifted() && !Broodwar->isVisible(LandLocation)) {
//		CommandCenter->move(Position(LandLocation));
//	} else if (CommandCenter->isLifted() && CommandCenter->getOrder() != Orders::BuildingLand) {
//		CommandCenter->land(LandLocation);
//	}
//}

void BaseManager::onFrame()
{
	for (auto currentMineral : Minerals) {
		if (currentMineral.mineralTrick != NULL) {
			Broodwar->drawCircleMap(currentMineral.mineralPatch->getPosition().x, currentMineral.mineralPatch->getPosition().y, 4, Colors::Purple, true);
		}
	}

	for (auto currentWorkerAndMineral : WorkerAndMineralList) {
		if (currentWorkerAndMineral.useTrick == true) {
			// Broodwar->drawCircleMap( Minerals[i].mineralPatch->getPosition().x,Minerals[i].mineralPatch->getPosition().y,2,Colors::Purple,true);
			Position workerpos = currentWorkerAndMineral.worker->getPosition();
			Position minpos    = currentWorkerAndMineral.mineralPatch->getPosition();
			Broodwar->drawLineMap(workerpos.x, workerpos.y, minpos.x, minpos.y, Colors::Red);
		}
	}

	// Broodwar->drawTextMap( CommandCenter->getPosition().x , CommandCenter->getPosition().y, "%d" , BaseReady );
	// Position LanLocPos = Position( LandLocation );
	// Broodwar->drawCircleMap( LanLocPos.x,  LanLocPos.y, 5 , Colors::Orange , true );

	//if (BaseReady == false) {
	//	if (LandLocation == CommandCenter->getTilePosition()
	//	    && !CommandCenter->isLifted()
	//	    && CommandCenter->getOrder() != Orders::BuildingLand) {
	//		BaseReady = true;
	//		//Broodwar->printf("CC has landed"); // COMMENTED BY SAM

	//		if (Broodwar->enemy()->getRace() == Races::Protoss) {

	//			//TilePosition TurretLoc  = bManager->getBuildLocationNear(LandLocation, UnitTypes::Terran_Missile_Turret, 0); // COMMENTED BY SAM
	//			//TilePosition TurretLoc2 = TilePosition(LandLocation.x + 4, LandLocation.y - 2); // COMMENTED BY SAM

	//			int ClosestChokeDist = 99999;
	//			Position BunkerPos   = Positions::None;
	//			//Position natPos    = Position(InfoMan->OurNat); // COMMENTED BY SAM
	//			//Position mainPos   = InfoMan->PosOurBase; // Position(InfoMan->PosOurBase); // COMMENTED BY SAM
	//			Position natPos          = { 0, 0 };
	//			Position mainPos         = (Position)Broodwar->self()->getStartLocation();
	//			BWTA::Region* natRegion  = BWTA::getRegion(natPos);
	//			BWTA::Region* mainRegion = BWTA::getRegion(mainPos);

	//			// BOOST_FOREACH (BWTA::Chokepoint* choke, BWTA::getChokepoints()) {
	//			for (auto choke : BWTA::getChokepoints()) {
	//				if ((choke->getRegions().first == natRegion && choke->getRegions().second != mainRegion) || (choke->getRegions().second == natRegion && choke->getRegions().first != mainRegion)) {
	//					/*
	//					if (choke->getCenter().getDistance(InfoMan->PosEnemyBase) < ClosestChokeDist) {
	//						ClosestChokeDist = choke->getCenter().getDistance(InfoMan->PosEnemyBase);
	//						BunkerPos        = choke->getCenter();
	//					}
	//					*/ // COMMENTED BY SAM
	//				}
	//			}

	//			/*
	//			if (TurretLoc.getDistance(TilePosition(BunkerPos)) < TurretLoc2.getDistance(TilePosition(BunkerPos))) {
	//				ProdMan->addToQueueTile(UnitTypes::Terran_Missile_Turret, TurretLoc);
	//			} else {
	//				ProdMan->addToQueueTile(UnitTypes::Terran_Missile_Turret, TurretLoc2);
	//			}
	//			*/ // COMMENTED BY SAM
	//		}

	//		/*
	//		if (InfoMan->PosOurNat.getDistance(Position(LandLocation)) < 5 * 32) {
	//			// MacroMan->PlaceNatBunker();
	//		}
	//		*/ // COMMENTED BY SAM

	//		/*
	//		for(std::set<Unit>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
	//		{
	//		if ( CommandCenter->getDistance(*m) <  11*32 ){
	//		// Minerals.push_back( *m);
	//		addMineralField(*m);
	//		}
	//		}
	//		*/
	//		Unitset GetMinerals = CommandCenter->getUnitsInRadius(15 * 32);
	//		// BOOST_FOREACH (Unit mineral, GetMinerals) {
	//		for (auto mineral : GetMinerals) {
	//			{
	//				if (mineral->getType() == UnitTypes::Resource_Mineral_Field) {
	//					addMineralField(mineral);
	//				}
	//			}

	//			//CalculateTrickChoice(); // COMMENTED BY SAM
	//			for (auto geyser : Broodwar->getGeysers()) {
	//				if (CommandCenter->getDistance(geyser) < 11 * 32) {
	//					Geysers.insert(geyser);
	//					// Broodwar->printf("added geyser");
	//				}
	//			}

	//			// Maynarding
	//			// CCmanager[0]->WorkerSaturation -= 18;
	//			for (int i = 0; i < 8; i++) {
	//				addWorker(CCmanager[0]->getBuilder());
	//			}

	//			// add a refinery
	//			if (Geysers.size() > 0) {
	//				if (Broodwar->enemy()->getRace() != Races::Zerg) {
	//					//ProdMan->addToQueueTile(UnitTypes::Terran_Refinery, Geysers[0]->getTilePosition()); // COMMENTED BY SAM
	//				}
	//			}

	//			/*
	//		// remove initial defence
	//		IDMan->stopDefence = true;
	//		BOOST_FOREACH( Unit unit, IDMan->DFunits ){
	//		AMan->AddUnit(unit);
	//		}
	//		BOOST_FOREACH( Unit unit, IDMan->Vultures ){
	//		AMan->AddUnit(unit);
	//		}
	//		IDMan->DFunits.clear();
	//		IDMan->Vultures.clear();
	//		*/

	//			return;
	//		} // if ( lanlocation == CC.landposition

	//		/*
	//		if (MacroMan->PState == P_Push || MacroMan->PState == P_Defend_Nat) {
	//			LiftToNat();
	//		*/ // COMMENTED BY SAM

	//		/*
	//		if( !CommandCenter->isLifted() ){
	//		CommandCenter->lift();
	//		}

	//		if( CommandCenter->isLifted() && !Broodwar->isVisible( LandLocation ) ){
	//		CommandCenter->move( Position( LandLocation ) );
	//		}
	//		else if( CommandCenter->isLifted() && CommandCenter->getOrder() != Orders::BuildingLand ){
	//		CommandCenter->land( LandLocation );
	//		}
	//		*/
	//	}
	//	if (Broodwar->enemy()->getRace() == Races::Zerg) { // MacroMan->ZState == Z_Expand_Defend
	//		LiftToNat();
	//		/*
	//		if( !CommandCenter->isLifted() ){
	//		CommandCenter->lift();
	//		}

	//		if( CommandCenter->isLifted() && !Broodwar->isVisible( LandLocation ) ){
	//		CommandCenter->move( Position( LandLocation ) );
	//		}
	//		else if( CommandCenter->isLifted() && CommandCenter->getOrder() != Orders::BuildingLand ){
	//		CommandCenter->land( LandLocation );
	//		}
	//		*/
	//	}
	//}

	if (LanLatency == true) {
		WorkerSaturation = Minerals.size() * 2 + 1; // Update saturation
	} else {
		WorkerSaturation = (int) ceil(Minerals.size() * 2.5 + 1); // Update saturation
	}
	// build a comsat station after saturation is complete
	//if (WorkerAndMineralList.size() >= (WorkerSaturation - 2)
	//    && buildComsat == false) {
	//	buildComsat = true;
	//	//Broodwar->printf("Enough Worker, now building comsat");
	//}
	// COMMENT BY SAM

	// don't build a comsat station when saturation isn't complete
	//if (WorkerAndMineralList.size() < (WorkerSaturation - 2)
	//    && buildComsat == true) {
	//	buildComsat = false;
	//	// Broodwar->printf("Enough Worker, now building comsat");
	//}
	// COMMENT BY SAM

	//// check if there are to many Workers here, if so let them join the fight
	//if (WorkerAndMineralList.size() > WorkerSaturation + 3) {
	//	// in case of terran, just add Workers to other CC
	//	if (Broodwar->enemy()->getRace() == Races::Terran) {
	//		bool addedToCC = false;
	//		for (int i = 0; i < CCmanager.size(); i++) {
	//			if (CCmanager[i]->WorkerSaturation > CCmanager[0]->WorkerAndMineralList.size()) {
	//				if (CCmanager[i]->CommandCenter != NULL) {
	//					if (CCmanager[i]->CommandCenter->exists()) {
	//						addedToCC = true;
	//						CCmanager[i]->addWorker(getFullHPWorker());
	//						break;
	//					}
	//				}
	//			}
	//		}
	//		// if not possible just add it to the fight
	//		if (addedToCC == false) {
	//			//MacroMan->AddWorker(getFullHPWorker()); // COMMENTED BY SAM
	//		}
	//	} else {
	//		//MacroMan->AddWorker(getFullHPWorker()); // COMMENTED BY SAM
	//	}
	//}

	//if (BaseReady == true && buildComsat) {
	//	// build a comsat station when it is complete
	//	/*
	//		if (ProdMan->Academy != NULL && CommandCenter->getAddon() == NULL) {
	//			CommandCenter->buildAddon(UnitTypes::Terran_Comsat_Station);
	//		}
	//		*/ // COMMENTED BY SAM
	//}

	for (auto currentMineral : Minerals) {
		if (!currentMineral.mineralPatch->exists()) { // remove empty mineral field
			Minerals.erase(currentMineral);
			continue;
		}
		// draw ID data
		// Broodwar->drawTextMap( Minerals[i].mineralPatch->getPosition().x,Minerals[i].mineralPatch->getPosition().y,"%d",Minerals[i].mineralPatch->getID() );
	}

	// MineralGather();
	MineralGatherChoice(); // functon used to get Workers to gather minerals

	//// remove dead Workers on gas
	//for (unsigned int i = 0; i < GasWorker.size(); i++) {
	//	if (!GasWorker[i]->exists()) { // remove dead worker
	//		GasWorker.erase(GasWorker.begin() + i);
	//		i--;
	//		continue;
	//	}
	//}

	//if (Refinerys.size() > 0 && GasWorker.size() < WorkersonGas) {
	//	toGas(3 - GasWorker.size());
	//}

	//if (GasWorker.size() > WorkersonGas && GasWorker.size() > 0) {
	//	addWorker(GasWorker.back());
	//	GasWorker.pop_back();
	//}

	//for (unsigned int i = 0; i < GasWorker.size(); i++) {
	//	if (!GasWorker[i]->isGatheringGas() && Refinerys.size() > 0) {
	//		GasWorker[i]->gather(Refinerys[0]);
	//	}
	//}
}

Unit BaseManager::getBuilder()
{

	Unit builder = NULL;
	for (auto currentWorkerAndMineral : WorkerAndMineralList) {
		// check if the Worker is not to far away
		if (currentWorkerAndMineral.worker->getDistance(CommandCenter) < 4 * 32) {
			builder = currentWorkerAndMineral.worker;
			WorkerAndMineralList.erase(currentWorkerAndMineral);
			break;
		}
	}
	/*
	if(  WorkerAndMineralList.size() > 0 ){
	WorkerAndMineral getBuilder = WorkerAndMineralList.back();
	WorkerAndMineralList.pop_back();
	builder = getBuilder.worker;
	// builder = WorkerAndMineralList[0];
	// WorkerAndMineralList.erase( WorkerAndMineralList.begin() );
	}
	*/
	return builder;
}

Unit BaseManager::getFullHPWorker()
{
	Unit worker = NULL;
	for (auto currentWorkerAndMineral : WorkerAndMineralList) {
		// check if the Worker is not to far away and full hp
		if (currentWorkerAndMineral.worker->getDistance(CommandCenter) < 4 * 32
		    && currentWorkerAndMineral.worker->getHitPoints() == UnitTypes::Zerg_Drone.maxHitPoints()) {
			worker = currentWorkerAndMineral.worker;
			WorkerAndMineralList.erase(currentWorkerAndMineral);
			break;
		}
	}
	// grab the highest hp possible
	if (worker == NULL) {
		int highestHP            = 0;
		WorkerAndMineral* choice = NULL;
		for (auto currentWorkerAndMineral : WorkerAndMineralList) {
			// check if the Worker is not to far away and full hp
			if (currentWorkerAndMineral.worker->getDistance(CommandCenter) < 4 * 32
			    && currentWorkerAndMineral.worker->getHitPoints() > highestHP) {
				choice    = &currentWorkerAndMineral;
				highestHP = currentWorkerAndMineral.worker->getHitPoints();
			}
		}
		if (choice != NULL) {
			worker = choice->worker;
			WorkerAndMineralList.erase(*choice);
		}
	}
	/*
	if(  WorkerAndMineralList.size() > 0 ){
	WorkerAndMineral getBuilder = WorkerAndMineralList.back();
	WorkerAndMineralList.pop_back();
	builder = getBuilder.worker;
	// builder = WorkerAndMineralList[0];
	// WorkerAndMineralList.erase( WorkerAndMineralList.begin() );
	}
	*/
	return worker;
}

//// transfer 3 workers to gas
//void BaseManager::toGas(int ToSend)
//{
//	// int ToSend = 3;
//	// Broodwar->printf("to gas");
//	while (WorkerAndMineralList.size() > 0) {
//		if (Refinerys.size() == 1 && GasWorker.size() < 3) {
//			ToSend--;
//			Unit gasWorker = getBuilder();
//			if (gasWorker == NULL) {
//				// return;
//				// continue;
//				break;
//			}
//			gasWorker->gather(Refinerys[0]);
//			GasWorker.push_back(gasWorker);
//			//Broodwar->printf("added 1 to gas"); // COMMENTED BY SAM
//			if (ToSend == 0) {
//				break;
//			}
//		} else {
//			break;
//		}
//	}
//}