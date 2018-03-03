#include "GUIManager.h"

#include "ArmyManager.h"
#include "BWTAManager.h"
#include "BaseManager.h"
#include "BuildOrderManager.h"
#include "DebugMessenger.h"
#include "InformationManager.h"

using namespace BWAPI;
using namespace Filter;

void GUIManager::drawTextOnScreen(BWAPI::Unit u, std::string format, int frames)
{
	MessageAndFrames mnf = { format, frames };
	messageBuffer.insert_or_assign(u, mnf);
}

void GUIManager::drawLineOnScreen(BWAPI::Unit u, EnemyUnitInfo enemy, int frames)
{
	TargetAndFrames tnf = { enemy, frames };
	lineBuffer.insert_or_assign(u, tnf);
}

void GUIManager::drawTextOnUnit(BWAPI::Unit u, std::string format)
{
	if (!u->exists()) {
		return;
	}
	Broodwar->drawTextMap(u->getPosition(), format.c_str());
}

void GUIManager::draw()
{
	talk();

	//BWTA draw
	//if (analyzed)	drawTerrainData();
	//else if (!analyzing) {
	//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
	//	analyzing = true;
	//}

	if (BWTAManager::Instance().analyzed) {
		drawTerrainData();
	}

	drawTopLeftOverlay();

	for (auto hatch : BaseManager::Instance().hatcheries) {
		std::stringstream ss;
		ss << "border " << hatch.borderRadius;
		GUIManager::Instance().drawTextOnScreen(hatch.base, ss.str());
		Broodwar->drawCircleMap(hatch.base->getPosition(), hatch.borderRadius, Colors::White);
	}

	for (auto squad : ArmyManager::Instance().getSquads()) {
		Broodwar->drawCircleMap(squad.getPosition(), ArmyManager::Instance().SQUAD_RADIUS, Colors::Purple);
	}

	drawOnScreenMessages();

	drawOnScreenLines();

	drawExtendedInterface();

	// this seems redundant at the moment but is useful if threading is wanted later
	if (BWTAManager::Instance().analysis_just_finished) {
		DebugMessenger::Instance() << "Finished analyzing map." << std::endl;
		BWTAManager::Instance().analysis_just_finished = false;
	}
}

void GUIManager::drawOnScreenLines()
{
	auto it = lineBuffer.begin();
	while (it != lineBuffer.end()) {
		if (it->second.frames > 0) {
			Broodwar->drawLineMap(it->first->getPosition(),
			                      it->second.target.getPosition(), Colors::Orange);
		}

		it->second.frames--;

		if (it->second.frames <= 0) {
			it = lineBuffer.erase(it);
		} else {
			it++;
		}
	}
}

void GUIManager::drawOnScreenMessages()
{
	auto it = messageBuffer.begin();
	while (it != messageBuffer.end()) {
		if (it->second.frames > 0) {
			drawTextOnUnit(it->first, it->second.format);
		}

		it->second.frames--;

		if (it->second.frames <= 0) {
			it = messageBuffer.erase(it);
		} else {
			it++;
		}
	}
}

void GUIManager::drawTopLeftOverlay()
{
	calculateAverageFrameTime();

	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(1, 0, "Supply: %i/%i", Broodwar->self()->supplyUsed(), Broodwar->self()->supplyTotal());
	Broodwar->drawTextScreen(1, 10, "Frame Count: %iF", Broodwar->getFrameCount());
	Broodwar->drawTextScreen(1, 20, "Last Error: %i", Broodwar->getLastError());
	Broodwar->drawTextScreen(1, 30, "Enemy Buildings: %i", InformationManager::Instance().enemyBuildings.size());
	Broodwar->drawTextScreen(1, 40, "Enemy Army: %i", InformationManager::Instance().enemyArmy.size());
	Broodwar->drawTextScreen(1, 50, "Htchrs/Wrkrs: %i/%i", BaseManager::Instance().hatcheries.size(), BaseManager::Instance().workers.size());
	Broodwar->drawTextScreen(1, 60, "Frame Time: %.1fms", duration);
	Broodwar->drawTextScreen(1, 70, "APM: %i", Broodwar->getAPM());
	int invaders = (!BaseManager::Instance().hatcheries.empty()) ? BaseManager::Instance().hatcheries.begin()->checkForInvaders().size() : 0;
	Broodwar->drawTextScreen(1, 80, "Invaders: %i", invaders);
	Broodwar->drawTextScreen(1, 90, "Mnrs/Dfndrs: %i/%i", BaseManager::Instance().miners.size(), BaseManager::Instance().defenders.size());
	Broodwar->drawTextScreen(1, 100, "Squads: %i", ArmyManager::Instance().getSquads().size());

	Broodwar->drawTextScreen(100, 0, "BO index: %i", std::distance(BuildOrderManager::Instance().bo.cbegin(), BuildOrderManager::Instance().boIndex));
	Broodwar->drawTextScreen(100, 10, "Pool: %i", BuildOrderManager::Instance().pool);
	int screenVPos = 20;
	int count      = 1;

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
}

void GUIManager::calculateAverageFrameTime()
{
	if (frameCount > 23) {
		std::chrono::steady_clock::time_point end       = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::milli> fp_ms = end - start;
		duration                                        = fp_ms.count() / 24;
		frameCount                                      = 1;
	} else {
		if (frameCount == 1) {
			start = std::chrono::steady_clock::now();
		}
		++frameCount;
	}
}

void GUIManager::talk()
{
	int choice = -1;

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
}

void GUIManager::drawTerrainData()
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
		auto& p = region->getPolygon();
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

void GUIManager::drawExtendedInterface()
{
	int verticalOffset = -10;

	// draw enemy units
	auto enemy = Broodwar->enemy();
	if (enemy) {
		for (auto& unit : enemy->getUnits()) {
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
