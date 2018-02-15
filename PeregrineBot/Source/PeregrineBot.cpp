#include "PeregrineBot.h"

#include "ArmyManager.h"
#include "BWTAManager.h"
#include "BaseManager.h"
#include "BuildOrderManager.h"
#include "FileManager.h"
#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"

/**
Bot: PeregrineBot
Author: S Jones (s7jones)

A Zerg bot that is getting cleverer all the time.
Beats the vanilla AI consistently on the SSCAI maps.

With thanks to Chris Coxe's ZZZKbot
@ https://github.com/chriscoxe/ZZZKBot
for his getPos function and some useful UnitFilters.

With thanks to Dave Churchill's UAlbertaBot
@ https://github.com/davechurchill/ualbertabot
for drawExtendedInterface function and useful onUnitDestroy,etc functions for workers.

With thanks to Martin Rooijackers' LetaBot(CIG2016)
@ http://cilab.sejong.ac.kr/sc_competition2016/posting_data/source/LetaBot.rar
for his BaseManager.

Thanks to the following for help in iRC/Discord:
jaj22, krasi0, ++N00byEdge, PurpleWaveJadien.

Also thanks to BWAPI, BWTA2, and Teamliquid tutorials:
https://bwapi.github.io/
https://bitbucket.org/auriarte/bwta2
http://www.teamliquid.net/blogs/485544-intro-to-scbw-ai-development
*/

#ifdef _DEBUG
bool debug_flag = true;
#else
bool debug_flag = false;
#endif

using namespace BWAPI;
using namespace Filter;

void PeregrineBot::onStart()
{
	DebugMessenger::Instance().setup(debug_flag);
	DebugMessenger::Instance() << "TESTTESTTESTTEST" << std::endl;

	// Print the map name.
	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	DebugMessenger::Instance() << "The map is " << Broodwar->mapName() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	//Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(1);

	// Check if this is a replay
	if (Broodwar->isReplay()) {
		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;

		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for (auto p : players) {
			// Only print the player if they are not an observer
			if (!p->isObserver())
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}
	} else // if this is not a replay
	{
		// Make our bot run thousands of games as fast as possible!
		Broodwar->setLocalSpeed(0);
		//Broodwar->setGUI(false);

		BWTAManager::Instance().analyze();

		InformationManager::Instance().setup();
	}
}

void PeregrineBot::onEnd(bool isWinner)
{
	// Called when the game ends
	if (!Broodwar->isReplay()) {
		FileManager::Instance().writeStatisticsToFile(version, isWinner);

		BWTA::cleanMemory();
	}
}

void PeregrineBot::onFrame()
{
	GUIManager::Instance().draw();

	// Called once every game frame

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	// update waiting units
	OrderManager::Instance().update();
	InformationManager::Instance().update();

	// Iterate through all the units that we own
	for (auto& u : Broodwar->self()->getUnits()) {
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;

		// Ignore the unit if it has one of the following status ailments
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;

		// Ignore the unit if it is in one of the following states
		if (u->isLoaded() || !u->isPowered() || u->isStuck())
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		/* if ( !u->isCompleted() || u->isConstructing() )
		continue;*/

		if (u->getType() == UnitTypes::Zerg_Overlord
		    || u->getType() == UnitTypes::Zerg_Zergling
		    || u->getType().isBuilding()) {
			InformationManager::Instance().spotting(u);
		}

		bool unitNeedsToWait = OrderManager::Instance().DoesUnitHasOrder(u);
		if (unitNeedsToWait) {
			continue;
		}

		// Finally make the unit do some stuff!
		// If the unit is a worker unit
		if (u->getType().isWorker()) {
			BaseManager::Instance().DoAllWorkerTasks(u);
			continue;
		}

		if (!BuildOrderManager::Instance().buildOrderComplete) {
			if ((*BuildOrderManager::Instance().boIndex == UnitTypes::Zerg_Spawning_Pool)
			    && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isBeingConstructed())) {
				BuildOrderManager::Instance().incrementBuildOrder();
				BuildOrderManager::Instance().pool = true;
				DebugMessenger::Instance() << "pool isBeingConstructed: " << Broodwar->getFrameCount() << "F" << std::endl;
			}
		}

		if ((!BuildOrderManager::Instance().poolready)
		    && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isCompleted())) {
			BuildOrderManager::Instance().poolready = true;
			DebugMessenger::Instance() << "pool ready: " << Broodwar->getFrameCount() << "F" << std::endl;
		}

		if (u->getType().isResourceDepot()) {
			BaseManager::Instance().ManageBases(u);
			continue;
		}

		if (u->getType() == UnitTypes::Zerg_Overlord) {
			InformationManager::Instance().overlordScouting(u);
			continue;
		}

		if (u->getType() == UnitTypes::Zerg_Zergling) {
			ArmyManager::Instance().ZerglingAttack(u);
			continue;
		}

	} // closure: unit iterator

	if (Broodwar->getFrameCount() > 86400) Broodwar->leaveGame();
}

void PeregrineBot::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());

	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void PeregrineBot::onReceiveText(BWAPI::Player player, std::string text)
{
	// Parse the received text
	DebugMessenger::Instance() << player->getName() << " said \"" << text << "\"" << std::endl;
}

void PeregrineBot::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void PeregrineBot::onNukeDetect(BWAPI::Position target)
{

	// Check if the target is a valid position
	if (target) {
		// if so, print the location of the nuclear strike target
		DebugMessenger::Instance() << "Nuclear Launch Detected at " << target << std::endl;

	} else {
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void PeregrineBot::onUnitDiscover(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitEvade(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitShow(BWAPI::Unit unit)
{
	InformationManager::Instance().onUnitShow(unit);
	BaseManager::Instance().onUnitShow(unit);
}

void PeregrineBot::onUnitHide(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitCreate(BWAPI::Unit unit)
{
	BaseManager::Instance().onUnitCreate(unit);

	if (Broodwar->isReplay()) {
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral()) {
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void PeregrineBot::onUnitDestroy(BWAPI::Unit unit)
{
	InformationManager::Instance().onUnitDestroy(unit);
	BaseManager::Instance().onUnitDestroy(unit);
}

void PeregrineBot::onUnitMorph(BWAPI::Unit unit)
{
	/*
	Called when a unit changes its UnitType.

	For example, when a Drone transforms into a Hatchery, a Siege Tank uses Siege Mode, or a Vespene Geyser receives a Refinery.

	Parameters
	unit	Unit object representing the unit that had its UnitType change.
	Note
	This is NOT called if the unit type changes to or from UnitTypes::Unknown.
	*/
	InformationManager::Instance().onUnitMorph(unit);
	BaseManager::Instance().onUnitMorph(unit);

	if (Broodwar->isReplay()) {
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral()) {
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void PeregrineBot::onUnitRenegade(BWAPI::Unit unit)
{
	BaseManager::Instance().onUnitRenegade(unit);

	DebugMessenger::Instance() << unit->getType() << ", " << unit->getPlayer()->getName() << ": was renegaded!" << std::endl;
}

void PeregrineBot::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void PeregrineBot::onUnitComplete(BWAPI::Unit unit)
{
}
