#include "UtilityManager.h"

#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

UtilityManager& UtilityManager::Instance()
{
	static UtilityManager instance;
	return instance;
}

bool UtilityManager::getBestActionForZergling(BWAPI::Unit zergling)
{
	if (options.size() == 0) {
		constructOptions();
	}
	bool flag      = false;
	auto enemyMain = InformationManager::Instance().enemyMain;
	if (enemyMain) {
		if (BWTA::getRegion(enemyMain->getPosition()) == BWTA::getRegion(zergling->getPosition())) {
			flag = performBestActionForZerglingInEnemyBase(zergling);
			if (flag) return true;
		}
	}

	if (!flag) {
		// putting this here for now to reconstruct old behaviour
		flag = performBestActionForZerglingInEnemyBase(zergling);
		if (flag) return true;
	}

	return flag;
}

void UtilityManager::constructOptions()
{
	auto enemyRace = InformationManager::Instance().enemyRace;

	if (enemyRace != Races::Unknown) {
		switch (enemyRace) {
		case Races::Enum::Protoss: {
			// couldn't take unit filter out of getClosestUnit without a runtime crash
			auto utilityClosest = [& scores = scores](Unit u) -> UtilResult {
				Unit other = u->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Protoss_Zealot
				        || GetType == UnitTypes::Protoss_Photon_Cannon));
				double score = other ? scores.p.closestZealCann : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(utilityClosest, "attack closest zealot/cannon");
			options.push_back(enemyClosest);

			auto utilitySupply = [& scores = scores](Unit u) -> UtilResult {
				Unit supply = u->getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Protoss_Pylon);
				double score = supply ? scores.p.closestPyln : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};

			Option enemySupply = Option(utilitySupply, "attack closest pylon");
			options.push_back(enemySupply);

			auto utilityWorker = [& scores = scores](Unit u) -> UtilResult {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.p.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilityAtAll = [& scores = scores](Unit u) -> UtilResult {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.p.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Zerg: {
			// stateful lambdas - https://youtu.be/_1X9D8Z5huA
			auto utilityClosest = [& scores = scores](Unit u) -> UtilResult {
				Unit other = u->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Zerg_Zergling
				        || GetType == UnitTypes::Zerg_Sunken_Colony
				        || IsWorker));
				double score = other ? scores.z.closestLingSunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(utilityClosest, "attack closest zergling/sunken/worker");
			options.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Unit u) -> UtilResult {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.z.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilityEnemy = [& scores = scores](Unit u) -> UtilResult {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.z.closestAllNotLarvEgg : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyEnemy = Option(utilityEnemy, "attack closest ground enemy(!larva/egg)");
			options.push_back(enemyEnemy);

			auto utilityAtAll = [& scores = scores](Unit u) -> UtilResult {
				Unit any = u->getClosestUnit(
				    IsEnemy && !IsFlying
				    && (GetType != UnitTypes::Zerg_Larva
				        || GetType != UnitTypes::Zerg_Egg));
				double score = any ? scores.z.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Terran: {
			auto utilityClosest = [& scores = scores](Unit u) -> UtilResult {
				Unit other = u->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Terran_Marine
				        || GetType == UnitTypes::Terran_Firebat
				        || GetType == UnitTypes::Terran_Bunker
				        || IsWorker));
				double score = other ? scores.t.closestMrneFireBunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(
			    utilityClosest, "attack closest marine/firebat/bunker/worker");
			options.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Unit u) -> UtilResult {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.t.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilitySupply = [& scores = scores](Unit u) -> UtilResult {
				Unit supply = u->getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Terran_Supply_Depot);
				double score = supply ? scores.t.closestSdpt : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			Option enemySupply = Option(utilitySupply, "attack closest depot");
			options.push_back(enemySupply);

			auto utilityAtAll = [& scores = scores](Unit u) -> UtilResult {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.t.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;

			auto utilityEnemyBase = [& scores = scores](Unit u) -> UtilResult {
				auto enemyMain = InformationManager::Instance().enemyMain;
				double score   = scores.t.enemyBase;
				UtilResult p;
				if (!enemyMain) {
					score = 0;
					p     = std::make_pair(score, enemyMain->u);
				} else {
					auto pos = enemyMain->getPosition();
					if (Broodwar->isVisible(TilePosition(pos))) {
						auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(pos),
						                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
						if (unitsOnBaseTile.size() == 0) {
							score = 0;
						}
					}
					p = std::make_pair(score, enemyMain->u);
				}
				return p;
			};
			Option enemyBase = Option(utilityEnemyBase, "attack known main base");
			options.push_back(enemyBase);
		}
		}
	}
}

bool UtilityManager::performBestActionForZerglingInEnemyBase(
    BWAPI::Unit zergling)
{
	std::string bestOptionDescription;
	UtilResult bestOptionResult = { 0, NULL };
	for (auto option : options) {
		auto result = option.util(zergling);
		if (result.first > bestOptionResult.first) {
			bestOptionResult      = result;
			bestOptionDescription = option.description;
		}
	}

	if (bestOptionResult.first == 0 || bestOptionResult.second == NULL) {
		return false;
	} else {
		OrderManager::Instance().Attack(zergling, bestOptionResult.second);
		GUIManager::Instance().drawTextOnScreen(zergling, bestOptionDescription, 48);
		return true;
	}
}

UtilityManager::UtilityManager() {}

Option::Option(funcUtil util, std::string description)
    : util(util)
    , description(description)
{
}
