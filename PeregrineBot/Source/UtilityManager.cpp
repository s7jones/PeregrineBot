#include "UtilityManager.h"

#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

bool UtilityManager::getBestActionForZergling(BWAPI::Unit zergling)
{
	if (options.empty())
	{
		constructOptions();
	}
	bool flag      = false;
	auto enemyMain = InformationManager::Instance().m_enemyMain;
	if (enemyMain.m_unit)
	{

		if (BWTA::getRegion(enemyMain.getPosition()) == BWTA::getRegion(zergling->getPosition()))
		{
			flag = performBestActionForZerglingInEnemyBase(zergling);
			if (flag) return true;
		}
	}

	if (!flag)
	{
		// putting this here for now to reconstruct old behaviour
		flag = performBestActionForZerglingInEnemyBase(zergling);
		if (flag) return true;
	}

	return flag;
}

void UtilityManager::constructOptions()
{
	const auto enemyRace = InformationManager::Instance().m_enemyRace;

	if (enemyRace != Races::Unknown)
	{
		switch (enemyRace)
		{
		case Races::Enum::Protoss:
		{
			auto genericInjr = [& scores = scores](Unit u, UnitType enemyType, double scoreGiven) -> UtilResult {
				auto weapon = u->getType().groundWeapon();
				if (weapon.damageAmount() == 0) return std::make_pair(0, nullptr);
				const auto enemies = u->getUnitsInRadius(
				    weapon.maxRange() * 2,
				    IsEnemy
				        && (GetType == enemyType));
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies)
				{
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int shields         = enemy->getShields();
					int hits            = lround(ceil(hp / effectiveDamage + shields / weapon.damageAmount()));
					if (hits < bestHits)
					{
						bestHits = hits;
					}
					if (hits > worstHits)
					{
						worstHits = hits;
					}
					int maxHits  = lround(ceil(enemyType.maxHitPoints() / effectiveDamage
                                              + enemyType.maxShields() / weapon.damageAmount()));
					double score = scoreGiven
					    + (maxHits - hits) / maxHits;
					if (bestScore < score)
					{
						bestScore  = score;
						bestChoice = enemy;
					}
				}
				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};

			auto utilityInjrZeal = [& scores = scores, genericInjr](Unit u) -> UtilResult {
				return genericInjr(u, UnitTypes::Protoss_Zealot, scores.p.injrZeal);
			};
			Option injrZeal = Option(utilityInjrZeal, "attack injured zealot");
			options.push_back(injrZeal);

			auto utilityInjrWork = [& scores = scores, genericInjr](Unit u) -> UtilResult {
				return genericInjr(u, UnitTypes::Protoss_Probe, scores.p.injrWork);
			};
			Option injrWork = Option(utilityInjrWork, "attack injured worker");
			options.push_back(injrWork);

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

			auto utilityArtosis = [& scores = scores](Unit u) -> UtilResult {
				const auto enemyBuildings = InformationManager::Instance().m_enemyBuildings;
				std::set<EnemyUnitInfo> pylons;
				for (const auto enemyBuilding : enemyBuildings)
				{
					if (enemyBuilding.getType() == UnitTypes::Protoss_Pylon)
					{
						pylons.insert(enemyBuilding);
					}
				}
				std::map<EnemyUnitInfo, std::vector<EnemyUnitInfo>> artosisPylonsAndBuildings;
				for (auto enemyBuilding : enemyBuildings)
				{
					if ((enemyBuilding.getType() == UnitTypes::Protoss_Pylon)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Nexus)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Assimilator))
					{
						continue;
					}
					std::set<EnemyUnitInfo> suppliers;
					for (const auto pylon : pylons)
					{
						auto rel = pylon.getPosition() - enemyBuilding.getPosition();
						if (isInPylonRange(rel.x, rel.y))
						{
							suppliers.insert(pylon);
						}
					}
					if (suppliers.size() == 1)
					{
						auto iterAndBool = artosisPylonsAndBuildings.insert({ *suppliers.begin(), { enemyBuilding } });
						if (!iterAndBool.second)
						{
							iterAndBool.first->second.push_back(enemyBuilding);
						}
					}
				}

				auto unitTypeScoreLambda = [& scores = scores](const BWAPI::UnitType& unitType) -> double {
					return unitType == UnitTypes::Protoss_Gateway ? scores.p.addArtosisGtwy
					                                              : unitType == UnitTypes::Protoss_Photon_Cannon ? scores.p.addArtosisCann
					                                                                                             : scores.p.addArtosisAll;
				};

				double bestScore    = 0;
				EnemyUnitInfo pylon = { nullptr };
				for (auto artosisPylonBuildings : artosisPylonsAndBuildings)
				{
					double score = scores.p.artosisPyln;
					for (auto building : artosisPylonBuildings.second)
					{
						score += unitTypeScoreLambda(building.getType());
					}

					if (bestScore < score)
					{
						bestScore = score;
						pylon     = artosisPylonBuildings.first;
					}
				}

				auto p = std::make_pair(bestScore, pylon);
				return p;
			};
			Option enemyArtosisPylon = Option(utilityArtosis, "attack artosis pylon");
			options.push_back(enemyArtosisPylon);

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
		case Races::Enum::Zerg:
		{
			// stateful lambdas - https://youtu.be/_1X9D8Z5huA
			auto utilityInjrZerg = [& scores = scores](Unit u) -> UtilResult {
				auto weapon        = u->getType().groundWeapon();
				const auto enemies = u->getUnitsInRadius(
				    weapon.maxRange() * 2,
				    IsEnemy
				        && (GetType == UnitTypes::Zerg_Zergling));
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies)
				{
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int hits            = lround(ceil(hp / effectiveDamage));
					if (hits < bestHits)
					{
						bestHits = hits;
					}
					if (hits > worstHits)
					{
						worstHits = hits;
					}
					int maxHits  = lround(ceil(UnitTypes::Zerg_Zergling.maxHitPoints() / effectiveDamage));
					double score = scores.z.injrZerg + (maxHits - hits) / maxHits;
					if (bestScore < score)
					{
						bestScore  = score;
						bestChoice = enemy;
					}
				}
				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};
			Option injrZerg = Option(utilityInjrZerg, "attack injured zerg");
			options.push_back(injrZerg);

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
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying
                                             && (GetType != UnitTypes::Zerg_Larva
                                                 || GetType != UnitTypes::Zerg_Egg));
				double score = any ? scores.z.closestAllNotLarvEgg : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyEnemy = Option(utilityEnemy, "attack closest ground enemy(!larva/egg)");
			options.push_back(enemyEnemy);

			auto utilityAtAll = [& scores = scores](Unit u) -> UtilResult {
				Unit any     = u->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.z.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			Option enemyAtAll = Option(utilityAtAll, "attack closest ground enemy");
			options.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Terran:
		{
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

			auto utilityEnemyBase = [& scores = scores](Unit u) -> UtilResult {
				auto enemyMain = InformationManager::Instance().m_enemyMain;
				double score   = scores.t.enemyBase;
				UtilResult p   = std::make_pair(0, nullptr);
				if (!enemyMain.m_unit)
				{
					score = 0;
					p     = std::make_pair(score, enemyMain.m_unit);
				}
				else
				{
					auto pos = enemyMain.getPosition();
					if (Broodwar->isVisible(TilePosition(pos)))
					{
						auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(pos),
						                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
						if (unitsOnBaseTile.empty())
						{
							score = 0;
						}
					}
					p = std::make_pair(score, enemyMain);
				}
				return p;
			};
			Option enemyBase = Option(utilityEnemyBase, "attack known main base");
			options.push_back(enemyBase);

			break;
		}
		}
	}
}

bool UtilityManager::performBestActionForZerglingInEnemyBase(
    BWAPI::Unit zergling)
{
	std::string bestOptionDescription;
	UtilResult bestOptionResult = { 0, nullptr };
	for (auto option : options)
	{
		auto result = option.util(zergling);
		if (result.first > bestOptionResult.first)
		{
			bestOptionResult      = result;
			bestOptionDescription = option.description;
		}
	}

	if (bestOptionResult.first != 0 && bestOptionResult.second.m_unit != nullptr)
	{
		OrderManager::Instance().Attack(zergling, bestOptionResult.second);
		GUIManager::Instance().drawTextOnScreen(zergling, bestOptionDescription, 48);
		GUIManager::Instance().drawLineOnScreen(zergling, bestOptionResult.second, 48);
		return true;
	}

	return false;
}
