#include "UtilityManager.h"

#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"

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
	bool flag             = false;
	auto isEnemyBaseFound = InformationManager::Instance().isEnemyBaseFound;
	if (isEnemyBaseFound) {
		auto enemyBase = InformationManager::Instance().enemyBase;
		if (BWTA::getRegion(enemyBase) == BWTA::getRegion(zergling->getPosition())) {
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
			auto utilityClosest = [](Unit u) -> std::pair<double, Unit> {
				Unit other = u->getClosestUnit(
				    IsEnemy && (GetType == UnitTypes::Protoss_Zealot || GetType == UnitTypes::Protoss_Photon_Cannon));
				double score = other ? 1 : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(utilityClosest, "attack closest zealot/cannon");
			options.push_back(enemyClosest);

			auto utilitySupply = [](Unit u) -> std::pair<double, Unit> {
				Unit supply = u->getClosestUnit(
				    IsEnemy && GetType == UnitTypes::Protoss_Pylon);
				double score = supply ? 0.9 : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};

			Option enemySupply = Option(utilitySupply, "attack closest pylon");
			options.push_back(enemySupply);

			auto utilityWorker = [](Unit u) -> std::pair<double, Unit> {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? 0.8 : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilityAtAll = [](Unit u) -> std::pair<double, Unit> {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? 0.7 : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Zerg: {
			auto utilityClosest = [](Unit u) -> std::pair<double, Unit> {
				Unit other = u->getClosestUnit(
				    IsEnemy && (GetType == UnitTypes::Zerg_Zergling || GetType == UnitTypes::Zerg_Sunken_Colony || IsWorker));
				double score = other ? 1 : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(utilityClosest, "attack closest zergling/sunken/worker");
			options.push_back(enemyClosest);

			auto utilityWorker = [](Unit u) -> std::pair<double, Unit> {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? 0.9 : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilityAtAll = [](Unit u) -> std::pair<double, Unit> {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? 0.8 : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Terran: {
			auto utilityClosest = [](Unit u) -> std::pair<double, Unit> {
				Unit other = u->getClosestUnit(
				    IsEnemy && (GetType == UnitTypes::Terran_Marine || GetType == UnitTypes::Terran_Firebat || GetType == UnitTypes::Terran_Bunker || IsWorker));
				double score = other ? 1 : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			Option enemyClosest = Option(
			    utilityClosest, "attack closest marine/firebat/bunker/worker");
			options.push_back(enemyClosest);

			auto utilityWorker = [](Unit u) -> std::pair<double, Unit> {
				Unit worker  = u->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? 0.9 : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			Option enemyWorker = Option(utilityWorker, "attack closest worker");
			options.push_back(enemyWorker);

			auto utilitySupply = [](Unit u) -> std::pair<double, Unit> {
				Unit supply = u->getClosestUnit(
				    IsEnemy && GetType == UnitTypes::Terran_Supply_Depot);
				double score = supply ? 0.8 : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			Option enemySupply = Option(utilitySupply, "attack closest depot");
			options.push_back(enemySupply);

			auto utilityAtAll = [](Unit u) -> std::pair<double, Unit> {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? 0.7 : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		}
	}
}

bool UtilityManager::performBestActionForZerglingInEnemyBase(
    BWAPI::Unit zergling)
{
	std::string bestOptionDescription;
	std::pair<double, Unit> bestOptionResult = { 0, NULL };
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
