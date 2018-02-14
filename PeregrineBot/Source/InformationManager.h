#pragma once
#include "Common.h"
#include "UnitInfo.h"
#include "Utility.h"

class InformationManager {
	InformationManager();

public:
	static InformationManager& Instance();
	void onUnitShow(BWAPI::Unit unit);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);

	void Setup();
	void SetupScouting();
	void Update();
	void UpdateScouting();
	void OverlordScouting(BWAPI::Unit overlord);
	void OverlordScoutingAtGameStart(BWAPI::Unit overlord);
	void OverlordScoutingAfterBaseFound(BWAPI::Unit overlord);
	void OverlordRetreatToHome(BWAPI::Unit overlord);

	ResourceUnitInfo* getClosestMineral(BWAPI::Unit u);

	bool isEnemyBaseDeduced   = false;
	bool isEnemyBaseReached   = false;
	bool isEnemyBaseDestroyed = false;
	BWAPI::Race enemyRace     = BWAPI::Races::Unknown;
	bool isEnemyRaceRandom    = false;
	bool isIslandsOnMap       = false;

	std::map<BWAPI::TilePosition, std::array<double, 6>> scoutingInfo;
	std::set<BWAPI::TilePosition> allStarts;
	std::set<BWAPI::TilePosition> otherStarts; // would be a good idea to make this const
	std::set<BWAPI::Position> unscoutedPositions;
	std::set<BWAPI::Position> scoutedPositions;
	std::set<ScoutingOptionFor4, sortByMeanTime> scoutingOptions;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> zerglingNetwork;
	std::map<std::set<BWAPI::TilePosition, sortByMostTopThenLeft>, distAndTime> overlordNetwork;
	bool isEnemyBaseFromOverlordSpotting   = false;
	BWAPI::Position enemyBaseSpottingGuess = { 0, 0 };

	std::set<EnemyUnitInfo> enemyBuildings;
	std::set<EnemyUnitInfo> enemyArmy;
	std::shared_ptr<EnemyUnitInfo> enemyMain = nullptr;

	std::set<ResourceUnitInfo> minerals;
	std::set<ResourceUnitInfo> geysers;

private:
	void addToEnemyBuildings(BWAPI::Unit unit);
	void addToEnemyArmy(BWAPI::Unit unit);
	void validateEnemyUnits();
	void addToMinerals(BWAPI::Unit mineral);
	void addToGeysers(BWAPI::Unit geyser);
	void validateResources();

	double maxBaseToBaseDistance;
	bool isPastSpottingTime = false;
};
