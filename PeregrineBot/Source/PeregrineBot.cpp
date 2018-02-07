#include "PeregrineBot.h"

#include "ArmyManager.h"
#include "BaseManager.h"
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

bool analyzed               = false;
bool analysis_just_finished = false;
bool analyzing              = false;
const bool analysis         = true;
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
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if (Broodwar->enemy()) // First make sure there is an enemy
		{
			//Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
		}
		// Make our bot run thousands of games as fast as possible!
		Broodwar->setLocalSpeed(0);
		//Broodwar->setGUI(false);

		if (analysis) {
			DebugMessenger::Instance() << "Begin analyzing map." << std::endl;

			BWTA::readMap();
			BWTA::analyze();
			analyzed               = true;
			analysis_just_finished = true;
		}

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

	drawAdditionalInformation();

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

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		BaseManager::Instance().workers.insert(unit);
	}
}

void PeregrineBot::onUnitHide(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitCreate(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		BaseManager::Instance().workers.insert(unit);
	}

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

	if (unit->getType().isResourceDepot() && unit->getPlayer() == Broodwar->self()) {
		BaseManager::Instance().hatcheries.erase(unit);
	}

	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		BaseManager::Instance().workers.erase(unit);
	}
}

void PeregrineBot::onUnitMorph(BWAPI::Unit unit)
{
	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self() && unit->getHitPoints() >= 0) {
		BaseManager::Instance().workers.insert(unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == Broodwar->self() && unit->getPlayer()->getRace() == Races::Zerg) {
		BaseManager::Instance().workers.erase(unit);
	}

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
	DebugMessenger::Instance() << unit->getType() << ", " << unit->getPlayer()->getName() << ": was renegaded!" << std::endl;
	if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
		BaseManager::Instance().workers.erase(unit);
	}
}

void PeregrineBot::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void PeregrineBot::onUnitComplete(BWAPI::Unit unit)
{
}

void PeregrineBot::drawAdditionalInformation()
{
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(1, 0, "Supply: %i/%i", Broodwar->self()->supplyUsed(), Broodwar->self()->supplyTotal());
	Broodwar->drawTextScreen(1, 10, "Frame Count: %iF", Broodwar->getFrameCount());
	Broodwar->drawTextScreen(1, 20, "Last Error: %i", lastError);
	Broodwar->drawTextScreen(1, 30, "Enemy Buildings: %i", InformationManager::Instance().enemyBuildings.size());
	Broodwar->drawTextScreen(1, 40, "Enemy Army: %i", InformationManager::Instance().enemyArmy.size());
	Broodwar->drawTextScreen(1, 50, "Htchrs/Wrkrs: %i/%i", BaseManager::Instance().hatcheries.size(), BaseManager::Instance().workers.size());

	Broodwar->drawTextScreen(100, 0, "BO index: %i", WorkerManager::Instance().indx);
	Broodwar->drawTextScreen(100, 10, "Pool: %i", WorkerManager::Instance().pool);

	int screenVPos = 20;
	int count      = 1;
	//for (auto scoutData : scoutingInfo) {
	//	Broodwar->drawTextScreen(100, screenVPos, "%i: gd%.1f ad%.1f md%.1f gt%.1f at%.1f mt%.1f", count,
	//	                         scoutData.second[0], scoutData.second[1], scoutData.second[2],
	//	                         scoutData.second[3], scoutData.second[4], scoutData.second[5]);
	//	count++;
	//	screenVPos += 10;
	//}

	for (auto scoutingOption : InformationManager::Instance().scoutingOptions) {
		Broodwar->drawTextScreen(100, screenVPos, "%i: %i,%i; %i,%iTP : %.1f +- %.1fF", count,
		                         scoutingOption.startToP1ToP2[1].x, scoutingOption.startToP1ToP2[1].y,
		                         scoutingOption.startToP1ToP2[2].x, scoutingOption.startToP1ToP2[2].y,
		                         scoutingOption.meanTime, scoutingOption.stdDev);

		count++;
		screenVPos += 10;
	}

	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 10, "Average FPS: %.1f", Broodwar->getAverageFPS());

	//BWTA draw
	//if (analyzed)	drawTerrainData();
	//else if (!analyzing) {
	//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
	//	analyzing = true;
	//}

	if (analyzed) {
		drawTerrainData();
	}

	drawExtendedInterface();

	if (analysis_just_finished) {
		DebugMessenger::Instance() << "Finished analyzing map." << std::endl;
		analysis_just_finished = false;
	}
}

void PeregrineBot::drawTerrainData()
{
	//we will iterate through all the base locations, and draw their outlines.
	for (const auto& baseLocation : BWTA::getBaseLocations()) {
		TilePosition p = baseLocation->getTilePosition();

		//draw outline of center location
		Position leftTop(p.x * TILE_SIZE, p.y * TILE_SIZE);
		Position rightBottom(leftTop.x + 4 * TILE_SIZE, leftTop.y + 3 * TILE_SIZE);
		Broodwar->drawBoxMap(leftTop, rightBottom, Colors::Blue);

		//draw a circle at each mineral patch
		for (const auto& mineral : baseLocation->getStaticMinerals()) {
			Broodwar->drawCircleMap(mineral->getInitialPosition(), 30, Colors::Cyan);
		}

		//draw the outlines of Vespene geysers
		for (const auto& geyser : baseLocation->getGeysers()) {
			TilePosition p1 = geyser->getInitialTilePosition();
			Position leftTop1(p1.x * TILE_SIZE, p1.y * TILE_SIZE);
			Position rightBottom1(leftTop1.x + 4 * TILE_SIZE, leftTop1.y + 2 * TILE_SIZE);
			Broodwar->drawBoxMap(leftTop1, rightBottom1, Colors::Orange);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if (baseLocation->isIsland()) {
			Broodwar->drawCircleMap(baseLocation->getPosition(), 80, Colors::Yellow);
		}
	}

	//we will iterate through all the regions and ...
	for (const auto& region : BWTA::getRegions()) {
		// draw the polygon outline of it in green
		BWTA::Polygon p = region->getPolygon();
		for (size_t j = 0; j < p.size(); ++j) {
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLineMap(point1, point2, Colors::Green);
		}
		// visualize the chokepoints with red lines
		for (auto const& chokepoint : region->getChokepoints()) {
			Position point1 = chokepoint->getSides().first;
			Position point2 = chokepoint->getSides().second;
			Broodwar->drawLineMap(point1, point2, Colors::Red);
		}
	}
}

void PeregrineBot::drawExtendedInterface()
{
	int verticalOffset = -10;

	// draw enemy units
	for (auto& unit : Broodwar->enemy()->getUnits()) {
		UnitType type = unit->getType();
		if (type == UnitTypes::Unknown)
			continue;

		//int hitPoints = ui.lastHealth;
		//int shields = ui.lastShields;

		const Position pos = unit->getPosition();

		if (pos == Positions::Unknown)
			continue;

		int left   = pos.x - type.dimensionLeft();
		int right  = pos.x + type.dimensionRight();
		int top    = pos.y - type.dimensionUp();
		int bottom = pos.y + type.dimensionDown();

		int hitPoints = unit->getHitPoints();
		int shields   = unit->getShields();

		if (!Broodwar->isVisible(TilePosition(pos))) {
			Broodwar->drawBoxMap(Position(left, top), Position(right, bottom), Colors::Grey, false);
			Broodwar->drawTextMap(Position(left + 3, top + 4), "%s", type.getName().c_str());
		}

		if (!type.isResourceContainer() && type.maxHitPoints() > 0) {
			double hpRatio = (double)hitPoints / (double)type.maxHitPoints();

			Color hpColor = Colors::Green;
			if (hpRatio <= 0.67) hpColor = Colors::Orange;
			if (hpRatio <= 0.33) hpColor = Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop      = top + verticalOffset;
			int hpBottom   = top + 4 + verticalOffset;

			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Grey, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(ratioRight, hpBottom), hpColor, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth) {
				Broodwar->drawLineMap(Position(i, hpTop), Position(i, hpBottom), Colors::Black);
			}
		}

		if (!type.isResourceContainer() && type.maxShields() > 0) {
			double shieldRatio = (double)shields / (double)type.maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop      = top - 3 + verticalOffset;
			int hpBottom   = top + 1 + verticalOffset;

			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Grey, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(ratioRight, hpBottom), Colors::Blue, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth) {
				Broodwar->drawLineMap(Position(i, hpTop), Position(i, hpBottom), Colors::Black);
			}
		}
	}

	// draw neutral units and our units
	for (auto& unit : Broodwar->getAllUnits()) {
		if (unit->getPlayer() == Broodwar->enemy()) {
			continue;
		}

		const Position& pos = unit->getPosition();

		int left   = pos.x - unit->getType().dimensionLeft();
		int right  = pos.x + unit->getType().dimensionRight();
		int top    = pos.y - unit->getType().dimensionUp();
		int bottom = pos.y + unit->getType().dimensionDown();

		//Broodwar->drawBoxMap(Position(left, top), Position(right, bottom), Colors::Grey, false);

		if (!unit->getType().isResourceContainer() && unit->getType().maxHitPoints() > 0) {
			double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();

			Color hpColor = Colors::Green;
			if (hpRatio < 0.66) hpColor = Colors::Orange;
			if (hpRatio < 0.33) hpColor = Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop      = top + verticalOffset;
			int hpBottom   = top + 4 + verticalOffset;

			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Grey, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(ratioRight, hpBottom), hpColor, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth) {
				Broodwar->drawLineMap(Position(i, hpTop), Position(i, hpBottom), Colors::Black);
			}
		}

		if (!unit->getType().isResourceContainer() && unit->getType().maxShields() > 0) {
			double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop      = top - 3 + verticalOffset;
			int hpBottom   = top + 1 + verticalOffset;

			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Grey, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(ratioRight, hpBottom), Colors::Blue, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth) {
				Broodwar->drawLineMap(Position(i, hpTop), Position(i, hpBottom), Colors::Black);
			}
		}

		if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0) {

			double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();

			int ratioRight = left + (int)((right - left) * mineralRatio);
			int hpTop      = top + verticalOffset;
			int hpBottom   = top + 4 + verticalOffset;

			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Grey, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(ratioRight, hpBottom), Colors::Cyan, true);
			Broodwar->drawBoxMap(Position(left, hpTop), Position(right, hpBottom), Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth) {
				Broodwar->drawLineMap(Position(i, hpTop), Position(i, hpBottom), Colors::Black);
			}
		}
	}
}
