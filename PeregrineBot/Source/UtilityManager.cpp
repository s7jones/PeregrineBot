#include "UtilityManager.h"

#include "GUIManager.h"
#include "InformationManager.h"
#include "OrderManager.h"
#include "TacticsManager.h"
#include "UnitInfo.h"

using namespace BWAPI;
using namespace Filter;

bool UtilityManager::getBestActionForZergling(BWAPI::Unit zergling)
{
	if (options_individual.empty()) {
		constructOptionsIndividual();
	}
	bool flag            = false;
	const auto enemyMain = InformationManager::Instance().enemyMain;
	if (enemyMain.unit) {
		if (BWTA::getRegion(enemyMain.getPosition()) == BWTA::getRegion(zergling->getPosition())) {
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

bool UtilityManager::getBestActionForSquad(Squad& squad)
{
	if (options_squad.empty()) {
		constructOptionsSquad();
	}
	bool flag            = false;
	const auto enemyMain = InformationManager::Instance().enemyMain;
	if (enemyMain.unit) {
		if (BWTA::getRegion(enemyMain.getPosition()) == BWTA::getRegion(squad.getPosition())) {
			flag = performBestActionForSquadInEnemyBase(squad);
			if (flag) return true;
		}
	}

	if (!flag) {
		// putting this here for now to reconstruct old behaviour
		flag = performBestActionForSquadInEnemyBase(squad);
		if (flag) return true;
	}

	return flag;
}

void UtilityManager::constructOptionsIndividual()
{
	const auto enemyRace = InformationManager::Instance().enemyRace;

	if (enemyRace != Races::Unknown) {
		switch (enemyRace) {
		case Races::Enum::Protoss: {
			auto genericInjr = [& scores = scores](Unit unit, UnitType enemyType, double scoreGiven) -> UtilResult {
				auto weapon = unit->getType().groundWeapon();
				if (weapon.damageAmount() == 0) return std::make_pair(0, nullptr);
				const auto enemies = unit->getUnitsInRadius(
				    weapon.maxRange() * 2,
				    IsEnemy
				        && (GetType == enemyType));
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies) {
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int shields         = enemy->getShields();
					int hits            = ceil(hp / effectiveDamage + shields / weapon.damageAmount());
					if (hits < bestHits) {
						bestHits = hits;
					}
					if (hits > worstHits) {
						worstHits = hits;
					}
					int maxHits  = ceil(enemyType.maxHitPoints() / effectiveDamage
                                       + enemyType.maxShields() / weapon.damageAmount());
					double score = scoreGiven
					    + (maxHits - hits) / maxHits;
					if (bestScore < score) {
						bestScore  = score;
						bestChoice = enemy;
					}
				}
				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};

			auto utilityInjrZeal = [& scores = scores, genericInjr](Unit unit) -> UtilResult {
				return genericInjr(unit, UnitTypes::Protoss_Zealot, scores.p.injrZeal);
			};
			OptionIndividual injrZeal = { utilityInjrZeal, "attack injured zealot" };
			options_individual.push_back(injrZeal);

			auto utilityInjrWork = [& scores = scores, genericInjr](Unit unit) -> UtilResult {
				return genericInjr(unit, UnitTypes::Protoss_Probe, scores.p.injrWork);
			};
			OptionIndividual injrWork = { utilityInjrWork, "attack injured worker" };
			options_individual.push_back(injrWork);

			// couldn't take unit filter out of getClosestUnit without a runtime crash
			auto utilityClosest = [& scores = scores](Unit unit) -> UtilResult {
				Unit other = unit->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Protoss_Zealot
				        || GetType == UnitTypes::Protoss_Photon_Cannon));
				double score = other ? scores.p.closestZealCann : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionIndividual enemyClosest = { utilityClosest, "attack closest zealot/cannon" };
			options_individual.push_back(enemyClosest);

			auto utilityArtosis = [& scores = scores](Unit unit) -> UtilResult {
				const auto enemyBuildings = InformationManager::Instance().enemyBuildings;
				std::set<EnemyUnitInfo> pylons;
				for (const auto& enemyBuilding : enemyBuildings) {
					if (enemyBuilding.getType() == UnitTypes::Protoss_Pylon) {
						pylons.insert(enemyBuilding);
					}
				}
				std::map<EnemyUnitInfo, std::vector<EnemyUnitInfo>> artosisPylonsAndBuildings;
				for (const auto& enemyBuilding : enemyBuildings) {
					if ((enemyBuilding.getType() == UnitTypes::Protoss_Pylon)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Nexus)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Assimilator)) {
						continue;
					}
					std::set<EnemyUnitInfo> suppliers;
					for (const auto& pylon : pylons) {
						auto rel = pylon.getPosition() - enemyBuilding.getPosition();
						if (isInPylonRange(rel.x, rel.y)) {
							suppliers.insert(pylon);
						}
					}
					if (suppliers.size() == 1) {
						auto iterAndBool = artosisPylonsAndBuildings.insert({ *suppliers.begin(), { enemyBuilding } });
						if (!iterAndBool.second) {
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
				for (auto artosisPylonBuildings : artosisPylonsAndBuildings) {
					double score = scores.p.artosisPyln;
					for (const auto& building : artosisPylonBuildings.second) {
						score += unitTypeScoreLambda(building.getType());
					}

					if (bestScore < score) {
						bestScore = score;
						pylon     = artosisPylonBuildings.first;
					}
				}

				auto p = std::make_pair(bestScore, pylon);
				return p;
			};
			OptionIndividual enemyArtosisPylon = { utilityArtosis, "attack artosis pylon" };
			options_individual.push_back(enemyArtosisPylon);

			auto utilitySupply = [& scores = scores](Unit unit) -> UtilResult {
				Unit supply = unit->getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Protoss_Pylon);
				double score = supply ? scores.p.closestPyln : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			OptionIndividual enemySupply = { utilitySupply, "attack closest pylon" };
			options_individual.push_back(enemySupply);

			auto utilityWorker = [& scores = scores](Unit unit) -> UtilResult {
				Unit worker  = unit->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.p.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionIndividual enemyWorker = { utilityWorker, "attack closest worker" };
			options_individual.push_back(enemyWorker);

			auto utilityAtAll = [& scores = scores](Unit unit) -> UtilResult {
				Unit any     = unit->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.p.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionIndividual enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_individual.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Zerg: {
			// stateful lambdas - https://youtu.be/_1X9D8Z5huA
			auto utilityInjrZerg = [& scores = scores](Unit unit) -> UtilResult {
				auto weapon        = unit->getType().groundWeapon();
				const auto enemies = unit->getUnitsInRadius(
				    weapon.maxRange() * 2,
				    IsEnemy
				        && (GetType == UnitTypes::Zerg_Zergling));
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies) {
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int hits            = ceil(hp / effectiveDamage);
					if (hits < bestHits) {
						bestHits = hits;
					}
					if (hits > worstHits) {
						worstHits = hits;
					}
					int maxHits  = ceil(UnitTypes::Zerg_Zergling.maxHitPoints() / effectiveDamage);
					double score = scores.z.injrZerg + (maxHits - hits) / maxHits;
					if (bestScore < score) {
						bestScore  = score;
						bestChoice = enemy;
					}
				}
				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};
			OptionIndividual injrZerg = { utilityInjrZerg, "attack injured zerg" };
			options_individual.push_back(injrZerg);

			auto utilityClosest = [& scores = scores](Unit unit) -> UtilResult {
				Unit other = unit->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Zerg_Zergling
				        || GetType == UnitTypes::Zerg_Sunken_Colony
				        || IsWorker));
				double score = other ? scores.z.closestLingSunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionIndividual enemyClosest = { utilityClosest, "attack closest zergling/sunken/worker" };
			options_individual.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Unit unit) -> UtilResult {
				Unit worker  = unit->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.z.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionIndividual enemyWorker = { utilityWorker, "attack closest worker" };
			options_individual.push_back(enemyWorker);

			auto utilityEnemy = [& scores = scores](Unit unit) -> UtilResult {
				Unit any     = unit->getClosestUnit(IsEnemy && !IsFlying
                                                && (GetType != UnitTypes::Zerg_Larva
                                                    || GetType != UnitTypes::Zerg_Egg));
				double score = any ? scores.z.closestAllNotLarvEgg : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionIndividual enemyEnemy = { utilityEnemy, "attack closest ground enemy(!larva/egg)" };
			options_individual.push_back(enemyEnemy);

			auto utilityAtAll = [& scores = scores](Unit unit) -> UtilResult {
				Unit any     = unit->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.z.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionIndividual enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_individual.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Terran: {
			auto utilityClosest = [& scores = scores](Unit unit) -> UtilResult {
				Unit other = unit->getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Terran_Marine
				        || GetType == UnitTypes::Terran_Firebat
				        || GetType == UnitTypes::Terran_Bunker
				        || IsWorker));
				double score = other ? scores.t.closestMrneFireBunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionIndividual enemyClosest = OptionIndividual(
			    utilityClosest, "attack closest marine/firebat/bunker/worker");
			options_individual.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Unit unit) -> UtilResult {
				Unit worker  = unit->getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.t.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionIndividual enemyWorker = { utilityWorker, "attack closest worker" };
			options_individual.push_back(enemyWorker);

			auto utilitySupply = [& scores = scores](Unit unit) -> UtilResult {
				Unit supply = unit->getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Terran_Supply_Depot);
				double score = supply ? scores.t.closestSdpt : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			OptionIndividual enemySupply = { utilitySupply, "attack closest depot" };
			options_individual.push_back(enemySupply);

			auto utilityAtAll = [& scores = scores](Unit unit) -> UtilResult {
				Unit any     = unit->getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.t.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionIndividual enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_individual.push_back(enemyAtAll);

			auto utilityEnemyBase = [& scores = scores](Unit unit) -> UtilResult {
				const auto enemyMain            = InformationManager::Instance().enemyMain;
				const auto isEnemyBaseDestroyed = InformationManager::Instance().isEnemyBaseDestroyed;
				double score                    = scores.t.enemyBase;
				UtilResult p                    = std::make_pair(0, nullptr);
				if (isEnemyBaseDestroyed) {
					score = 0;
				} else {
					if (enemyMain.unit == nullptr) {
						score = 0;
						p     = std::make_pair(score, enemyMain.unit);
					} else {
						auto pos = enemyMain.getPosition();
						if (Broodwar->isVisible(TilePosition(pos))) {
							auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(pos),
							                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
							if (unitsOnBaseTile.empty()) {
								score = 0;
							}
						}
						p = std::make_pair(score, enemyMain);
					}
				}
				return p;
			};
			OptionIndividual enemyBase = { utilityEnemyBase, "attack known main base" };
			options_individual.push_back(enemyBase);

			break;
		}
		}
	}
}

void UtilityManager::constructOptionsSquad()
{
	const auto enemyRace = InformationManager::Instance().enemyRace;

	if (enemyRace != Races::Unknown) {
		// race agnostic options
		auto reinforce = [& scores = scores](Squad squad) -> UtilResult {
			if (false) {
			}
		};

		// race specific options
		switch (enemyRace) {
		case Races::Enum::Protoss: {
			auto genericInjr = [](Squad squad, UnitType enemyType, double scoreGiven) -> UtilResult {
				auto weapon = (*squad.begin())->getType().groundWeapon(); // assume mono-unit squads
				if (weapon.damageAmount() == 0) return std::make_pair(0, nullptr);
				Unitset enemies;
				for (auto unit : squad) {
					auto enemiesLocal = unit->getUnitsInRadius(
					    weapon.maxRange() * 2,
					    IsEnemy
					        && (GetType == enemyType));
					enemies.insert(enemiesLocal.begin(), enemiesLocal.end()); // would set_union be more efficient?
				}
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies) {
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int shields         = enemy->getShields();
					int hits            = ceil(hp / effectiveDamage + shields / weapon.damageAmount());
					if (hits < bestHits) {
						bestHits = hits;
					}
					if (hits > worstHits) {
						worstHits = hits;
					}
					int maxHits  = ceil(enemyType.maxHitPoints() / effectiveDamage
                                       + enemyType.maxShields() / weapon.damageAmount());
					double score = scoreGiven
					    + (maxHits - hits) / maxHits;
					if (bestScore < score) {
						bestScore  = score;
						bestChoice = enemy;
					}
				}

				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};

			auto utilityInjrZeal = [& scores = scores, genericInjr](Squad squad) -> UtilResult {
				return genericInjr(squad, UnitTypes::Protoss_Zealot, scores.p.injrZeal);
			};
			OptionSquad injrZeal = { utilityInjrZeal, "attack injured zealot" };
			options_squad.push_back(injrZeal);

			auto utilityInjrWork = [& scores = scores, genericInjr](Squad squad) -> UtilResult {
				return genericInjr(squad, UnitTypes::Protoss_Probe, scores.p.injrWork);
			};
			OptionSquad injrWork = { utilityInjrWork, "attack injured worker" };
			options_squad.push_back(injrWork);

			// couldn't take unit filter out of getClosestUnit without a runtime crash
			auto utilityClosest = [& scores = scores](Squad squad) -> UtilResult {
				Unit other = squad.getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Protoss_Zealot
				        || GetType == UnitTypes::Protoss_Photon_Cannon));
				double score = other ? scores.p.closestZealCann : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionSquad enemyClosest = { utilityClosest, "attack closest zealot/cannon" };
			options_squad.push_back(enemyClosest);

			auto utilityArtosis = [& scores = scores](Squad squad) -> UtilResult {
				const auto enemyBuildings = InformationManager::Instance().enemyBuildings;
				std::set<EnemyUnitInfo> pylons;
				for (const auto& enemyBuilding : enemyBuildings) {
					if (enemyBuilding.getType() == UnitTypes::Protoss_Pylon) {
						pylons.insert(enemyBuilding);
					}
				}
				std::map<EnemyUnitInfo, std::vector<EnemyUnitInfo>> artosisPylonsAndBuildings;
				for (const auto& enemyBuilding : enemyBuildings) {
					if ((enemyBuilding.getType() == UnitTypes::Protoss_Pylon)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Nexus)
					    || (enemyBuilding.getType() == UnitTypes::Protoss_Assimilator)) {
						continue;
					}
					std::set<EnemyUnitInfo> suppliers;
					for (const auto& pylon : pylons) {
						auto rel = pylon.getPosition() - enemyBuilding.getPosition();
						if (isInPylonRange(rel.x, rel.y)) {
							suppliers.insert(pylon);
						}
					}
					if (suppliers.size() == 1) {
						auto iterAndBool = artosisPylonsAndBuildings.insert({ *suppliers.begin(), { enemyBuilding } });
						if (!iterAndBool.second) {
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
				for (auto artosisPylonBuildings : artosisPylonsAndBuildings) {
					double score = scores.p.artosisPyln;
					for (const auto& building : artosisPylonBuildings.second) {
						score += unitTypeScoreLambda(building.getType());
					}

					if (bestScore < score) {
						bestScore = score;
						pylon     = artosisPylonBuildings.first;
					}
				}

				auto p = std::make_pair(bestScore, pylon);
				return p;
			};
			OptionSquad enemyArtosisPylon = { utilityArtosis, "attack artosis pylon" };
			options_squad.push_back(enemyArtosisPylon);

			auto utilitySupply = [& scores = scores](Squad squad) -> UtilResult {
				Unit supply = squad.getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Protoss_Pylon);
				double score = supply ? scores.p.closestPyln : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			OptionSquad enemySupply = { utilitySupply, "attack closest pylon" };
			options_squad.push_back(enemySupply);

			auto utilityWorker = [& scores = scores](Squad squad) -> UtilResult {
				Unit worker  = squad.getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.p.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionSquad enemyWorker = { utilityWorker, "attack closest worker" };
			options_squad.push_back(enemyWorker);

			auto utilityAtAll = [& scores = scores](Squad squad) -> UtilResult {
				Unit any     = squad.getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.p.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionSquad enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_squad.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Zerg: {
			// stateful lambdas - https://youtu.be/_1X9D8Z5huA
			auto utilityInjrZerg = [& scores = scores](Squad squad) -> UtilResult {
				auto weapon        = (*squad.begin())->getType().groundWeapon();
				const auto enemies = squad.getUnitsInRadius(
				    weapon.maxRange() * 2,
				    IsEnemy
				        && (GetType == UnitTypes::Zerg_Zergling));
				double bestScore = 0;
				int bestHits     = std::numeric_limits<int>::infinity();
				int worstHits    = 0;
				Unit bestChoice  = nullptr;
				for (const auto enemy : enemies) {
					int effectiveDamage = weapon.damageAmount() - enemy->getType().armor();
					int hp              = enemy->getHitPoints();
					int hits            = ceil(hp / effectiveDamage);
					if (hits < bestHits) {
						bestHits = hits;
					}
					if (hits > worstHits) {
						worstHits = hits;
					}
					int maxHits  = ceil(UnitTypes::Zerg_Zergling.maxHitPoints() / effectiveDamage);
					double score = scores.z.injrZerg + (maxHits - hits) / maxHits;
					if (bestScore < score) {
						bestScore  = score;
						bestChoice = enemy;
					}
				}
				auto p = std::make_pair(bestScore, bestChoice);
				return p;
			};
			OptionSquad injrZerg = { utilityInjrZerg, "attack injured zerg" };
			options_squad.push_back(injrZerg);

			auto utilityClosest = [& scores = scores](Squad squad) -> UtilResult {
				Unit other = squad.getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Zerg_Zergling
				        || GetType == UnitTypes::Zerg_Sunken_Colony
				        || IsWorker));
				double score = other ? scores.z.closestLingSunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionSquad enemyClosest = { utilityClosest, "attack closest zergling/sunken/worker" };
			options_squad.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Squad squad) -> UtilResult {
				Unit worker  = squad.getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.z.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionSquad enemyWorker = { utilityWorker, "attack closest worker" };
			options_squad.push_back(enemyWorker);

			auto utilityEnemy = [& scores = scores](Squad squad) -> UtilResult {
				Unit any     = squad.getClosestUnit(IsEnemy && !IsFlying
                                                && (GetType != UnitTypes::Zerg_Larva
                                                    || GetType != UnitTypes::Zerg_Egg));
				double score = any ? scores.z.closestAllNotLarvEgg : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionSquad enemyEnemy = { utilityEnemy, "attack closest ground enemy(!larva/egg)" };
			options_squad.push_back(enemyEnemy);

			auto utilityAtAll = [& scores = scores](Squad squad) -> UtilResult {
				Unit any     = squad.getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.z.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionSquad enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_squad.push_back(enemyAtAll);

			break;
		}
		case Races::Enum::Terran: {
			auto utilityClosest = [& scores = scores](Squad squad) -> UtilResult {
				Unit other = squad.getClosestUnit(
				    IsEnemy
				    && (GetType == UnitTypes::Terran_Marine
				        || GetType == UnitTypes::Terran_Firebat
				        || GetType == UnitTypes::Terran_Bunker
				        || IsWorker));
				double score = other ? scores.t.closestMrneFireBunkWork : 0;
				auto p       = std::make_pair(score, other);
				return p;
			};
			OptionSquad enemyClosest = OptionSquad(
			    utilityClosest, "attack closest marine/firebat/bunker/worker");
			options_squad.push_back(enemyClosest);

			auto utilityWorker = [& scores = scores](Squad squad) -> UtilResult {
				Unit worker  = squad.getClosestUnit(IsEnemy && IsWorker);
				double score = worker ? scores.t.closestWork : 0;
				auto p       = std::make_pair(score, worker);
				return p;
			};
			OptionSquad enemyWorker = { utilityWorker, "attack closest worker" };
			options_squad.push_back(enemyWorker);

			auto utilitySupply = [& scores = scores](Squad squad) -> UtilResult {
				Unit supply = squad.getClosestUnit(
				    IsEnemy
				    && GetType == UnitTypes::Terran_Supply_Depot);
				double score = supply ? scores.t.closestSdpt : 0;
				auto p       = std::make_pair(score, supply);
				return p;
			};
			OptionSquad enemySupply = { utilitySupply, "attack closest depot" };
			options_squad.push_back(enemySupply);

			auto utilityAtAll = [& scores = scores](Squad squad) -> UtilResult {
				Unit any     = squad.getClosestUnit(IsEnemy && !IsFlying);
				double score = any ? scores.t.closestAll : 0;
				auto p       = std::make_pair(score, any);
				return p;
			};
			OptionSquad enemyAtAll = { utilityAtAll, "attack closest ground enemy" };
			options_squad.push_back(enemyAtAll);

			auto utilityEnemyBase = [& scores = scores](Squad squad) -> UtilResult {
				const auto enemyMain            = InformationManager::Instance().enemyMain;
				const auto isEnemyBaseDestroyed = InformationManager::Instance().isEnemyBaseDestroyed;
				double score                    = scores.t.enemyBase;
				UtilResult p                    = std::make_pair(0, nullptr);
				if (isEnemyBaseDestroyed) {
					score = 0;
				} else {
					if (enemyMain.unit == nullptr) {
						score = 0;
						p     = std::make_pair(score, enemyMain.unit);
					} else {
						auto pos = enemyMain.getPosition();
						if (Broodwar->isVisible(TilePosition(pos))) {
							auto unitsOnBaseTile = Broodwar->getUnitsOnTile(TilePosition(pos),
							                                                IsEnemy && IsVisible && Exists && IsResourceDepot && !IsLifted);
							if (unitsOnBaseTile.empty()) {
								score = 0;
							}
						}
						p = std::make_pair(score, enemyMain);
					}
				}
				return p;
			};
			OptionSquad enemyBase = { utilityEnemyBase, "attack known main base" };
			options_squad.push_back(enemyBase);

			break;
		}
		}
	}
}

bool UtilityManager::performBestActionForZerglingInEnemyBase(BWAPI::Unit zergling)
{
	std::string bestOptionDescription;
	UtilResult bestOptionResult = { 0, nullptr };
	for (auto option : options_individual) {
		auto result = option.util(zergling);
		if (result.first > bestOptionResult.first) {
			bestOptionResult      = result;
			bestOptionDescription = option.description;
		}
	}

	if (bestOptionResult.first != 0 && bestOptionResult.second.unit != nullptr) {
		OrderManager::Instance().attack(zergling, bestOptionResult.second);
		GUIManager::Instance().drawTextOnScreen(zergling, bestOptionDescription, 48);
		GUIManager::Instance().drawLineOnScreen(zergling, bestOptionResult.second, 48);
		return true;
	}

	return false;
}

bool UtilityManager::performBestActionForSquadInEnemyBase(Squad& squad)
{
	std::string bestOptionDescription;
	UtilResult bestOptionResult = { 0, nullptr };
	for (auto option : options_squad) {
		auto result = option.util(squad);
		if (result.first > bestOptionResult.first) {
			bestOptionResult      = result;
			bestOptionDescription = option.description;
		}
	}

	if (bestOptionResult.first != 0 && bestOptionResult.second.unit != nullptr) {
		OrderManager::Instance().attack(squad, bestOptionResult.second);
		GUIManager::Instance().drawTextOnScreen(*squad.begin(), bestOptionDescription, 48);
		GUIManager::Instance().drawLineOnScreen(*squad.begin(), bestOptionResult.second, 48);
		return true;
	}

	return false;
}
