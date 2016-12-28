#include "PeregrineBot.h"

/**
Bot: PeregrineBot
Auther: S Jones (s7jones)

A simple, not very clever, 5 pooling zerg.
Beats the vanilla AI consistently on the SSCAI maps.

With thanks to Chris Coxe's ZZZKbot @ https://github.com/chriscoxe/ZZZKBot
for his getPos function and some useful UnitFilters.

With thanks to Dave Churchill's UAlbertaBot @ https://github.com/davechurchill/ualbertabot
for drawExtendedInterface function.

Also thanks to BWAPI, BWTA2, and Teamliquid tutorials:
https://bwapi.github.io/
https://bitbucket.org/auriarte/bwta2
http://www.teamliquid.net/blogs/485544-intro-to-scbw-ai-development
*/


using namespace BWAPI;
using namespace Filter;

bool analyzed;
bool analysis_just_finished;
bool analyzing;
const bool analysis = true;

bool pool = false;
int workercount = 4;

const std::vector<UnitType> bo = { UnitType(UnitTypes::Zerg_Drone),
UnitType(UnitTypes::Zerg_Spawning_Pool),
UnitType(UnitTypes::Zerg_Drone),
UnitType(UnitTypes::Zerg_Drone),
UnitType(UnitTypes::Zerg_Zergling),
UnitType(UnitTypes::Zerg_Zergling),
UnitType(UnitTypes::Zerg_Zergling),
UnitType(UnitTypes::Zerg_Overlord),
UnitType(UnitTypes::Zerg_Zergling),
UnitType(UnitTypes::Zerg_Zergling),
UnitType(UnitTypes::Zerg_Zergling) };

int indx = 0;

bool poolready = false;

int lastChecked = 0;
int poolLastChecked = 0;

bool reachEnemyBase = false;
bool destroyEnemyBase = false;

int i;

Position enemyBase(0, 0);

Race enemyRace;

std::string Version = "v4";

Error lastError = Errors::None;

std::set<Unit> enemyBuildings;

bool DrawUnitHealthBars = true;

void PeregrineBot::drawAdditionalInformation(){
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

	Broodwar->drawTextScreen(1, 0, "Supply: %i / %i", Broodwar->self()->supplyUsed(), Broodwar->self()->supplyTotal());
	Broodwar->drawTextScreen(1, 20, "Framecount: %i", Broodwar->getFrameCount());
	Broodwar->drawTextScreen(1, 30, "lasterror: %i", lastError);

	Broodwar->drawTextScreen(100, 0, "bo indx: %i", indx);
	Broodwar->drawTextScreen(100, 20, "pool: %i", pool);

	Broodwar->drawTextScreen(1, 40, "enemy buildings: %i", enemyBuildings.size());

	//BWTA draw
	//if (analyzed)	drawTerrainData();
	//else if (!analyzing) {
	//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
	//	analyzing = true;
	//}

	if (analyzed)
		drawTerrainData();

	drawExtendedInterface();

	if (analysis_just_finished) {
		Broodwar << "Finished analyzing map." << std::endl;
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
	if (DrawUnitHealthBars)
	{
		return;
	}

	int verticalOffset = -10;

	// draw enemy units
	//for (const auto & kv : getUnitData(BWAPI::Broodwar->enemy()).getUnits())
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		//const UnitInfo & ui(kv.second);
		//BWAPI::UnitType type(ui.type);
		BWAPI::UnitType type = unit->getType();
		if (type == UnitTypes::Unknown)
			continue;

		//int hitPoints = ui.lastHealth;
		//int shields = ui.lastShields;

		//const BWAPI::Position & pos = ui.lastPosition;
		const BWAPI::Position pos = unit->getPosition();

		if (pos == Positions::Unknown)
			continue;



		int left = pos.x - type.dimensionLeft();
		int right = pos.x + type.dimensionRight();
		int top = pos.y - type.dimensionUp();
		int bottom = pos.y + type.dimensionDown();

		int hitPoints = unit->getHitPoints();
		int shields = unit->getShields();

		if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(pos)))
		{
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);
			BWAPI::Broodwar->drawTextMap(BWAPI::Position(left + 3, top + 4), "%s", type.getName().c_str());
		}

		if (!type.isResourceContainer() && type.maxHitPoints() > 0)
		{
			double hpRatio = (double)hitPoints / (double)type.maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		if (!type.isResourceContainer() && type.maxShields() > 0)
		{
			double shieldRatio = (double)shields / (double)type.maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

	}

	// draw neutral units and our units
	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			continue;
		}

		const BWAPI::Position & pos = unit->getPosition();

		int left = pos.x - unit->getType().dimensionLeft();
		int right = pos.x + unit->getType().dimensionRight();
		int top = pos.y - unit->getType().dimensionUp();
		int bottom = pos.y + unit->getType().dimensionDown();

		//BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);

		if (!unit->getType().isResourceContainer() && unit->getType().maxHitPoints() > 0)
		{
			double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		if (!unit->getType().isResourceContainer() && unit->getType().maxShields() > 0)
		{
			double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0)
		{

			double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();

			int ratioRight = left + (int)((right - left) * mineralRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Cyan, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}
	}
}

auto getPos =
[](const BWAPI::TilePosition tp, const BWAPI::UnitType ut)
{
	return Position(Position(tp) + Position((ut.tileWidth() * BWAPI::TILEPOSITION_SCALE) / 2, (ut.tileHeight() * BWAPI::TILEPOSITION_SCALE) / 2));
};

//void scout(Unit* u) {
//	Broodwar->sendText("Scouting!");
//
//	BWTA::Region* region = BWTA::getRegion(u->getPosition());
//
//	//for (const auto& region : BWTA::getRegions()) {
//	// draw the polygon outline of it in green
//	BWTA::Polygon poly = region->getPolygon();
//	for (size_t j = 0; j < poly.size(); ++j) {
//		Position point1 = poly[j];
//		u->move(point1, true);
//	}
//	//}
//}
//
//void scoutOverlord(Unit* u) {
//	Broodwar->sendText("Overlord Scouting!");
//	
//	for (const auto& region : BWTA::getRegions()) {
//		//for (const auto& region : BWTA::getRegions()) {
//		// draw the polygon outline of it in green
//		BWTA::Polygon poly = region->getPolygon();
//		for (size_t j = 0; j < poly.size(); ++j) {
//			Position point1 = poly[j];
//			u->move(point1, true);
//		}
//	}
//}

std::map<Unit, std::pair<std::vector<TilePosition>, int>> movelists;


void setMoveTo(BWAPI::Unit* u, const BWAPI::Position &pos) {
	std::vector<TilePosition> movelist = BWTA::getShortestPath((*u)->getTilePosition(), (TilePosition)pos);
	movelists.erase(*u);
	movelists[*u] = std::make_pair(movelist, 0);
}

void move(BWAPI::Unit* u, const BWAPI::Position &pos) {
	if ((*u)->isMoving()) {
		return;
	}
	else {
		if (movelists.count(*u)) { // if we have a movelist for that unit
			auto it = movelists.find(*u);
			int i = it->second.second;
			std::vector<TilePosition> movelist = it->second.first;
			for (int j = 0; j < ((movelist.size() < 8) ? movelist.size() : 8); j++) {
				if (i >= movelist.size()){
					movelists.erase(*u);
					break;
				}
				(*u)->move(Position(movelist[i]), true);
				i++;
			}
		}
		else  { // we have no movelist for that unit
			setMoveTo(u, pos);
		}
	}
}

std::map<int, std::pair<char*, int>> msgList;

void sendText(int key, char* msg) {
	std::pair<char*, int> value(msg, Broodwar->getFrameCount());

	if (msgList.insert(std::map<int, std::pair<char*, int>>::value_type(key, value)).second == false) {
		msgList[key].first = msg;
	}
	//int lastChecked = msglist[key].second
	//msgList[key] == std::pair<msg, msgList[key].second
	//msgList.insert_or_assign(key, );
}

void PeregrineBot::onStart()
{
	// Hello World!
	Broodwar->sendText("PeregrineBot");

	// Print the map name.
	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	//Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Check if this is a replay
	if (Broodwar->isReplay())
	{
		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;

		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for (auto p : players)
		{
			// Only print the player if they are not an observer
			if (!p->isObserver())
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}
	}
	else // if this is not a replay
	{
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if (Broodwar->enemy()) // First make sure there is an enemy
			Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

		// Make our bot run thousands of games as fast as possible!
		Broodwar->setLocalSpeed(0);
		//Broodwar->setGUI(false);

		//if (Broodwar->mapFileName() == "(4)Andromeda.scx") {
		//	Broodwar->leaveGame();
		//}

		//BWTA::readMap();
		//analyzed = false;
		//analysis_just_finished = false;
		//analyzing = false;

		if (analysis){
			Broodwar << "Begin analyzing map." << std::endl;
			BWTA::readMap();
			BWTA::analyze();
			analyzed = true;
			analysis_just_finished = true;
		}

		bool islandFound = false;
		for (auto bl : BWTA::getBaseLocations()){
			if (bl->isIsland()) {
				islandFound = true;
				break;
			}
		}

		if (islandFound)	Broodwar << "Islands on map!" << std::endl;
	}
}

void PeregrineBot::onEnd(bool isWinner)
{
	// Called when the game ends

	if (!Broodwar->isReplay()) {
		struct Scores{
			std::string name;
			int matches;
			int score;
			int percent;
		};

		Scores score[3];

		boost::filesystem::path readDir ("./bwapi-data/read");
		boost::filesystem::path writeDir("./bwapi-data/write");
		boost::filesystem::path pathIn, pathOut;


		std::stringstream ss;
		ss << "data_" << Version << ".txt";
		std::string filename = ss.str();

		pathIn /= readDir /= filename;
		pathOut /= writeDir /= filename;

		boost::filesystem::ifstream input(pathIn, std::ios::in);

		if (input.fail()){
			score[0].name = "z";
			score[1].name = "t";
			score[2].name = "p";
			for (i = 0; i < 3; i++) {
				score[i].matches = 0;
				score[i].score = 0;
				score[i].percent = 0;
			}
		}
		else if (input.is_open()) {
			for (i = 0; i < 3; i++) {
				input >> score[i].name >> score[i].matches >> score[i].score >> score[i].percent;
			}
		}

		input.close();

		if (Broodwar->enemy()->getRace() == Races::Zerg)	score[0].matches++;
		if (Broodwar->enemy()->getRace() == Races::Terran)	score[1].matches++;
		if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].matches++;

		if (isWinner)
		{
			if (Broodwar->enemy()->getRace() == Races::Zerg)	score[0].score++;
			if (Broodwar->enemy()->getRace() == Races::Terran)	score[1].score++;
			if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].score++;
		}

		if (Broodwar->enemy()->getRace() == Races::Zerg)	score[0].percent = (int)(100 * score[0].score / score[0].matches);
		if (Broodwar->enemy()->getRace() == Races::Terran)	score[1].percent = (int)(100 * score[1].score / score[1].matches);
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
	// Called once every game frame

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	drawAdditionalInformation();

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	enemyRace = Broodwar->enemy()->getRace();

	static std::set<Position> startPositions;
	static std::set<Position> otherPositions;
	static std::set<Position> scoutedOtherPositions;
	static std::set<Position> unscoutedOtherPositions;

	static char msg[100];

	/*
	for (auto m : msgList) {
	int lastChecked = m.second.second;
	auto msg2 = m.second.first;
	if ((lastChecked + 400) < Broodwar->getFrameCount()) {
	Broodwar->sendText(msg2);
	m.second.second = Broodwar->getFrameCount();
	}
	}
	*/

	// do start location shuffle
	for (TilePosition p : Broodwar->getStartLocations()){
		startPositions.insert(getPos(p, UnitTypes::Special_Start_Location));
	}

	i = 0;
	for (Position startpos : startPositions) {
		if (startpos == getPos(Broodwar->self()->getStartLocation(), UnitTypes::Special_Start_Location)) {
			//sprintf_s(msg, sizeof(msg), "Player starting position, x: %i, y: %i", startpos.x, startpos.y);
		}
		else {
			//sprintf_s(msg, sizeof(msg), "Potential enemy starting position, x: %i, y: %i", startpos.x, startpos.y);
			std::pair<std::set<Position>::iterator, bool> ret = otherPositions.insert(startpos);

			if (ret.second) {
				unscoutedOtherPositions.insert(*ret.first);
			}
		}
		//Broodwar->sendText(msg);
		sendText(i, msg);
		i++;
	}

	int number_of_starts = Broodwar->getStartLocations().size();
	i = number_of_starts;


	if ((enemyBase.x != 0) && (enemyBase.y != 0)) {
		for (auto otherPos : unscoutedOtherPositions) {
			scoutedOtherPositions.insert(otherPos);
			unscoutedOtherPositions.erase(otherPos);
		}
	}

	for (auto otherPos : unscoutedOtherPositions) {
		if (Broodwar->isVisible(TilePosition(otherPos)))
		{
			if (Broodwar->getUnitsOnTile(TilePosition(otherPos), IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted).empty())
			{
				scoutedOtherPositions.insert(otherPos);
				unscoutedOtherPositions.erase(otherPos);
			}
			else
			{
				if (!((enemyBase.x != 0) && (enemyBase.y != 0))) {
					sprintf_s(msg, sizeof(msg), "Enemy base, x: %i, y: %i, frame: %i", otherPos.x, otherPos.y, Broodwar->getFrameCount());
					//Broodwar->sendText(msg);
					sendText(i, msg);
					Broodwar << msg << std::endl;
				}
				enemyBase = otherPos;
			}
		}
	}


	if (indx > (bo.size() * 2))	indx = bo.size() * 2;

	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;

		// Ignore the unit if it has one of the following status ailments
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;

		// Ignore the unit if it is in one of the following states
		//if (u->isLoaded() || !u->isPowered() || u->isStuck())
		//	continue;

		if (u->isLoaded() || !u->isPowered())
			continue;

		if (u->isStuck()) {
			//Broodwar << u->getType() << " at x: " << u->getPosition().x << ", y: " << u->getPosition().y << ", is stuck!" << std::endl;
			continue;
		}

		// Ignore the unit if it is incomplete or busy constructing
		/* if ( !u->isCompleted() || u->isConstructing() )
		   continue;*/


		// Finally make the unit do some stuff!


		// If the unit is a worker unit
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (u->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					// Harvest from the nearest mineral patch or gas refinery
					if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery)))
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
				else Broodwar << "is idle and has power up?" << std::endl;
			}
			if (bo[indx] == UnitTypes::Zerg_Spawning_Pool) {
				if ((!pool) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Spawning_Pool.mineralPrice())) {
					if ((poolLastChecked + 115) < Broodwar->getFrameCount()) {
						//find a location for spawning pool and construct it
						TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Spawning_Pool, u->getTilePosition());
						u->build(UnitTypes::Zerg_Spawning_Pool, buildPosition);
						poolLastChecked = Broodwar->getFrameCount();
					}
				}
			}

			if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Hatchery.mineralPrice()) && (indx >= bo.size())) {
				if ((lastChecked + 400) < Broodwar->getFrameCount()) {
					TilePosition buildPosition = Broodwar->getBuildLocation(UnitTypes::Zerg_Hatchery, u->getTilePosition());
					u->build(UnitTypes::Zerg_Hatchery, buildPosition);
					lastChecked = Broodwar->getFrameCount();
				}
			}

		}

		//if ((bo[indx] == UnitTypes::Zerg_Spawning_Pool) && (u->getType().isBuilding()) && (u->isConstructing())) {
		//	indx++;
		//	Broodwar << "pool isConstructing" << std::endl;
		//}

		if ((bo[indx] == UnitTypes::Zerg_Spawning_Pool) && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isBeingConstructed())){
			indx++;
			pool = true;
			Broodwar << "pool isBeingConstructed: " << Broodwar->getFrameCount() << std::endl;
		}

		if ((!poolready) && (u->getType() == UnitTypes::Zerg_Spawning_Pool) && (u->isCompleted())) {
			Broodwar << "pool ready: " << Broodwar->getFrameCount() << std::endl;
			poolready = true;
		}

		if (u->getType().isResourceDepot()) {
			/*UnitType type = bo[indx];
			std::string msg = std::to_string(type);
			Broodwar->sendText(msg.c_str());*/

			if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Drone.mineralPrice()) && (bo[indx] == UnitTypes::Zerg_Drone)) {
				if (!u->getLarva().empty()) {
					u->train(UnitTypes::Zerg_Drone);
					indx++;
				}
			}

			if ((Broodwar->self()->minerals() >= UnitTypes::Zerg_Overlord.mineralPrice()) &&
				((bo[indx] == UnitTypes::Zerg_Overlord) ||
				((indx >= bo.size()) &&
				(Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 1)))) {
				if (!u->getLarva().empty()) {
					u->train(UnitTypes::Zerg_Overlord);
					indx++;
				}
			}

			if ((poolready) && (Broodwar->self()->minerals() >= UnitTypes::Zerg_Zergling.mineralPrice()) && (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() > 0) && ((bo[indx] == UnitTypes::Zerg_Zergling) || (indx >= bo.size()))) {
				if (!u->getLarva().empty()) {
					u->train(UnitTypes::Zerg_Zergling);
					indx++;
				}

			}

		}
		if (u->getType() == UnitTypes::Zerg_Overlord) {
			if (u->isIdle()) {
				if (!((enemyBase.x != 0) && (enemyBase.y != 0))) {
					for (auto p : boost::adaptors::reverse(unscoutedOtherPositions)) { //https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop
						u->move(p, true);
					}
				}
				else {
					// Overlord scouting perimeter of all regions
					Broodwar << "Overlord Scouting!" << std::endl;
					static std::vector<Position> scoutLocations;
					if (scoutLocations.empty()) {
						BWTA::Region* enemyRegion = BWTA::getRegion(enemyBase);
						BWTA::Polygon poly = enemyRegion->getPolygon();
						for (size_t j = 0; j < poly.size(); ++j) {
							Position point1 = poly[j];
							scoutLocations.push_back(point1);
							//u->move(point1, true);
						}
						for (const auto& region : BWTA::getRegions()) {
							for (const auto& base : region->getBaseLocations()) {
								Position point1 = base->getPosition();
								scoutLocations.push_back(point1);
								//u->move(point1, true);
							}
						}
					}
					else {
						auto it = scoutLocations.begin();
						Position baseToScout = (*it);
						u->move(baseToScout, false);
						scoutLocations.erase(it);
					}
				}
			}
			else if (u->isUnderAttack()) {
				u->move(getPos(Broodwar->self()->getStartLocation(), UnitTypes::Special_Start_Location));
			}
		}

		if (u->getType() == UnitTypes::Zerg_Zergling) {
			if ((enemyBase.x != 0) && (enemyBase.y != 0)){
				if ((!reachEnemyBase) && (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(enemyBase))) {
					Broodwar << "reach enemy base: " << Broodwar->getFrameCount() << std::endl;
					reachEnemyBase = true;
				}
			}
			//Broodwar->setLocalSpeed(23);
			if (enemyRace == Races::Protoss) { // if protoss - enemy then pylon then worker
				Unit enemy = u->getClosestUnit(
					IsEnemy &&
					(GetType == UnitTypes::Zerg_Zergling ||
					GetType == UnitTypes::Terran_Marine ||
					GetType == UnitTypes::Protoss_Zealot ||
					GetType == UnitTypes::Zerg_Sunken_Colony ||
					GetType == UnitTypes::Terran_Bunker ||
					GetType == UnitTypes::Protoss_Photon_Cannon));
				Unit supply = u->getClosestUnit(
					IsEnemy &&
					(GetType == UnitTypes::Protoss_Pylon ||
					GetType == UnitTypes::Terran_Supply_Depot));
				Unit worker = u->getClosestUnit(IsEnemy && IsWorker);
				Unit enemy_atall = u->getClosestUnit(IsEnemy);
				if (enemy){
					//Broodwar->setLocalSpeed(1);
					u->attack(PositionOrUnit(enemy));
					//Broodwar->setLocalSpeed(1);
				}
				else if (supply) {

					u->attack(PositionOrUnit(supply));
				}
				else if (worker) u->attack(PositionOrUnit(worker));
				else if (enemy_atall) u->attack(PositionOrUnit(enemy_atall));
			}	// end if protoss
			else { // not protoss - enemy and worker then supply
				Unit enemy = u->getClosestUnit(
					IsEnemy &&
					(GetType == UnitTypes::Zerg_Zergling ||
					GetType == UnitTypes::Terran_Marine ||
					GetType == UnitTypes::Protoss_Zealot ||
					GetType == UnitTypes::Zerg_Sunken_Colony ||
					GetType == UnitTypes::Terran_Bunker ||
					GetType == UnitTypes::Protoss_Photon_Cannon ||
					GetType == UnitTypes::Terran_Firebat ||
					IsWorker));
				Unit supply = u->getClosestUnit(
					IsEnemy &&
					(GetType == UnitTypes::Protoss_Pylon ||
					GetType == UnitTypes::Terran_Supply_Depot));
				Unit enemy_atall = u->getClosestUnit(IsEnemy);
				if (enemy){
					//Broodwar->setLocalSpeed(1);
					u->attack(PositionOrUnit(enemy));
					//Broodwar->setLocalSpeed(1);
				}
				else if (supply) u->attack(PositionOrUnit(supply));
				else if (enemy_atall) u->attack(PositionOrUnit(enemy_atall));
			}

			//Unit neutral = u->getClosestUnit(IsNeutral && IsBuilding, 5);
			//if (neutral) {		// remove potential blockages
			//	u->attack(PositionOrUnit(neutral));
			//	Broodwar << "attacking neutral" << std::endl;
			//}

			if (u->isIdle()) {
				Unit enemy_atall = u->getClosestUnit(IsEnemy);
				if (enemy_atall) {
					u->attack(PositionOrUnit(enemy_atall));
				}
				else {
					if ((enemyBase.x != 0) && (enemyBase.y != 0)){
						if (!destroyEnemyBase) {
							if (!Broodwar->isVisible(TilePosition(enemyBase)))
								u->attack(PositionOrUnit(enemyBase));
							else if (!Broodwar->getUnitsOnTile(TilePosition(enemyBase), IsEnemy && IsVisible && Exists && IsBuilding && !IsLifted).empty())
								u->attack(PositionOrUnit(enemyBase));
							else {
								Broodwar << "Scouting!" << std::endl;
								static std::vector<Position> zerglingScoutLocations;
								if (zerglingScoutLocations.empty()) {
									//TODO: add scouting of known existing buildings

									for (const auto& region : BWTA::getRegions()) {
										// if region isn't reachable then skip
										if (!BWTA::getRegion(u->getPosition())->isReachable(region)) {
											continue;
										}
										BWTA::Polygon poly = region->getPolygon();
										auto it = zerglingScoutLocations.begin();
										for (size_t j = 0; j < poly.size(); ++j) {
											Position point1 = poly[j];
											if (region == BWTA::getRegion(u->getPosition())) {
												it = zerglingScoutLocations.insert(it, point1);
												//it++;
											}
											else {
												zerglingScoutLocations.push_back(point1);
											}
										}

									}
								}
								else {
									auto it = zerglingScoutLocations.begin();
									Position perimeterPoint = (*it);
									u->move(perimeterPoint, false);
									zerglingScoutLocations.erase(it);
								}
							}
						} // havent destroyed enemy base
						else {
							Broodwar << "Scouting!" << std::endl;
							static std::vector<Position> zerglingScoutLocations;
							if (zerglingScoutLocations.empty()) {
								//TODO: add scouting of known existing buildings

								for (const auto& region : BWTA::getRegions()) {
									// if region isn't reachable then skip
									if (!BWTA::getRegion(u->getPosition())->isReachable(region)) {
										continue;
									}
									BWTA::Polygon poly = region->getPolygon();
									auto it = zerglingScoutLocations.begin();
									for (size_t j = 0; j < poly.size(); ++j) {
										Position point1 = poly[j];
										if (region == BWTA::getRegion(u->getPosition())) {
											it = zerglingScoutLocations.insert(it, point1);
											//it++;
										}
										else {
											zerglingScoutLocations.push_back(point1);
										}
									}

								}
							}
							else {
								auto it = zerglingScoutLocations.begin();
								Position perimeterPoint = (*it);
								u->move(perimeterPoint, false);
								zerglingScoutLocations.erase(it);
							}

						}	// have destroyed enemy base
					}	// no enemy base
					else
					{
						u->move((*unscoutedOtherPositions.begin()), false);
						//move(u, (*unscoutedOtherPositions.begin()));
						/*for (auto p : unscoutedOtherPositions) {
							u->move(p, true);
							}*/
					}
				}
			} // end if idle
			else if (u->isMoving()) {
				// if (IsMoving)	TODO: look for ismoving and lastcommand to see if checking scouted 
				UnitCommand lastCmd = u->getLastCommand();
				if (lastCmd.getType() == UnitCommandTypes::Move) {
					Position targetPos = lastCmd.getTargetPosition();
					//if (unscoutedOtherPositions.find(targetPos) == unscoutedOtherPositions.end()) {
					if ((unscoutedOtherPositions.count(targetPos) == 0) && (!unscoutedOtherPositions.empty())) {
						Broodwar << "recalculate scouting" << std::endl;
						u->stop();
						u->move((*unscoutedOtherPositions.begin()), false);
						/*for (auto p : unscoutedOtherPositions) {
							u->move(p, true);
							}*/
					}
				}
			} // end if moving
		}
	} // closure: unit iterator

	if (Broodwar->getFrameCount() > 86400)	Broodwar->leaveGame();
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
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
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
	if (target)
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void PeregrineBot::onUnitDiscover(BWAPI::Unit unit)
{
	if ((IsEnemy)(unit)) {
		if ((IsBuilding)(unit)){
			enemyBuildings.insert(unit);
		}
	}
}

void PeregrineBot::onUnitEvade(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitShow(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitHide(BWAPI::Unit unit)
{
}

void PeregrineBot::onUnitCreate(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void PeregrineBot::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPosition() == enemyBase) {
		destroyEnemyBase = true;
		Broodwar << "destroyed enemy base: " << Broodwar->getFrameCount() << std::endl;
	}
	enemyBuildings.erase(unit);
}

void PeregrineBot::onUnitMorph(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void PeregrineBot::onUnitRenegade(BWAPI::Unit unit)
{
}

void PeregrineBot::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void PeregrineBot::onUnitComplete(BWAPI::Unit unit)
{
}
