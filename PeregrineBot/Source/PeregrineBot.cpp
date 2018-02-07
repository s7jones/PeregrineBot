#include "PeregrineBot.h"

#include "ArmyManager.h"
#include "BWTAManager.h"
#include "BaseManager.h"
#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "WorkerManager.h"

/**
Bot: PeregrineBot
Author: S Jones (s7jones)

A Zerg bot that is getting cleverer all the time.
Beats the vanilla AI consistently on the SSCAI maps.

With thanks to Chris Coxe's ZZZKbot
@ https://github.com/chriscoxe/ZZZKBot
for his GetPos function and some useful UnitFilters.

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

int frameCount              = 1;
int i;
std::string Version = "v4";
Error lastError     = Errors::None;
double duration     = 0;
std::chrono::steady_clock::time_point start;

void PeregrineBot::onStart()
{
	DebugMessenger::Instance().Setup(debug_flag);
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

		InformationManager::Instance().Setup();
	}
}

void PeregrineBot::onEnd(bool isWinner)
{
	// Called when the game ends
	if (!Broodwar->isReplay()) {
		struct Scores {
			std::string name;
			int matches;
			int score;
			int percent;
		};

		Scores score[3];

		boost::filesystem::path readDir("./bwapi-data/read");
		boost::filesystem::path writeDir("./bwapi-data/write");
		boost::filesystem::path pathIn, pathOut;

		std::stringstream ss;
		ss << "data_" << Version << ".txt";
		std::string filename = ss.str();

		pathIn /= readDir /= filename;
		pathOut /= writeDir /= filename;

		boost::filesystem::ifstream input(pathIn, std::ios::in);

		if (input.fail()) {
			score[0].name = "z";
			score[1].name = "t";
			score[2].name = "p";
			for (i = 0; i < 3; i++) {
				score[i].matches = 0;
				score[i].score   = 0;
				score[i].percent = 0;
			}
		} else if (input.is_open()) {
			for (i = 0; i < 3; i++) {
				input >> score[i].name >> score[i].matches >> score[i].score >> score[i].percent;
			}
		}

		input.close();

		if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].matches++;
		if (Broodwar->enemy()->getRace() == Races::Terran) score[1].matches++;
		if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].matches++;

		if (isWinner) {
			if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].score++;
			if (Broodwar->enemy()->getRace() == Races::Terran) score[1].score++;
			if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].score++;
		}

		if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].percent = (int)(100 * score[0].score / score[0].matches);
		if (Broodwar->enemy()->getRace() == Races::Terran) score[1].percent = (int)(100 * score[1].score / score[1].matches);
		if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].percent = (int)(100 * score[2].score / score[2].matches);

		boost::filesystem::ofstream output(pathOut, std::ios::trunc);

		for (i = 0; i < 3; i++) {
			output << score[i].name << "\t" << score[i].matches << "\t" << score[i].score << "\t" << score[i].percent << "\n";
		}

		output.close();

		BWTA::cleanMemory();
	}
}

void PeregrineBot::onFrame()
{
	static int choice = -1;

	if (Broodwar->getFrameCount() == 2300) {
		choice = rand() % 100;
		switch (choice) {
		case 17:
			Broodwar->sendText("Status: 0x00000011");
			Broodwar->sendText("Info: Software faillllure. An errdsaor oc.....'''#cured transferring exec/1234/");
		default:
			Broodwar->sendText("Hi Twitch!");
			break;
		}
	}

	if (Broodwar->getFrameCount() == 2300 + 480) {
		if (choice == 17) {
			Broodwar->sendText("Help! Is anyone there? Help! Help! Please! Help!");
		}
	}

	if (frameCount > 23) {
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		//double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 24;
		std::chrono::duration<double, std::milli> fp_ms = end - start;
		duration                                        = fp_ms.count() / 24;
		frameCount                                      = 1;
	} else {
		if (frameCount == 1) {
			start = std::chrono::steady_clock::now();
		}
		++frameCount;
	}
	Broodwar->drawTextScreen(1, 60, "Frame Time: %.1fms", duration);

	// Called once every game frame

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	GUIManager::Instance().draw();

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	// Update waiting units
	OrderManager::Instance().Update();
	InformationManager::Instance().Update();

	if (WorkerManager::Instance().indx > (WorkerManager::Instance().bo.size() * 2))
		WorkerManager::Instance().indx = WorkerManager::Instance().bo.size() * 2;

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

		bool unitNeedsToWait = OrderManager::Instance().DoesUnitHasOrder(u);
		if (unitNeedsToWait) {
			continue;
		}

		// Finally make the unit do some stuff!
		// If the unit is a worker unit
		if (u->getType().isWorker()) {
			WorkerManager::Instance().DoAllWorkerTasks(u);
			continue;
		}

		if ((WorkerManager::Instance().bo[WorkerManager::Instance().indx] == UnitTypes::Zerg_Spawning_Pool) && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isBeingConstructed())) {
			WorkerManager::Instance().indx++;
			WorkerManager::Instance().pool = true;
			DebugMessenger::Instance() << "pool isBeingConstructed: " << Broodwar->getFrameCount() << "F" << std::endl;
		}

		if ((!WorkerManager::Instance().poolready) && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isCompleted())) {
			WorkerManager::Instance().poolready = true;
			DebugMessenger::Instance() << "pool ready: " << Broodwar->getFrameCount() << "F" << std::endl;
		}

		if (u->getType().isResourceDepot()) {
			BaseManager::Instance().ManageBases(u);
			continue;
		}

		if (u->getType() == UnitTypes::Zerg_Overlord) {
			InformationManager::Instance().OverlordScouting(u);
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
