#include "UtilityManager.h"

#include "InformationManager.h"

using namespace BWAPI;
using namespace Filter;

UtilityManager& UtilityManager::Instance()
{
	static UtilityManager instance;
	return instance;
}

BWAPI::UnitCommand UtilityManager::getBestActionForZergling(BWAPI::Unit zergling)
{
	UnitCommand command;
	auto isEnemyBaseFound = InformationManager::Instance().isEnemyBaseFound;
	if (isEnemyBaseFound) {
		auto enemyBase = InformationManager::Instance().enemyBase;
		if (BWTA::getRegion(enemyBase) == BWTA::getRegion(zergling->getPosition())) {
			command = getBestActionForZerglingInEnemyBase(zergling);
		}
	}

	if (command.getType() == UnitCommandTypes::None) {
		command.type = UnitCommandTypes::Stop;
		Broodwar << "Err: Utility Manager didn't know what to do with zergling, stopping" << std::endl;
	}
	return command;
}

BWAPI::UnitCommand UtilityManager::getBestActionForZerglingInEnemyBase(BWAPI::Unit zergling)
{
	UnitCommand command;
	auto enemyRace = InformationManager::Instance().enemyRace;

	std::unordered_map<Option, double> UtilityMap;

	switch (enemyRace) {
	case Races::Enum::Protoss: {
		std::function<Unit(Unit)> lambdaFilter = [](Unit u) -> Unit {
			return u->getClosestUnit(
				IsEnemy && (GetType == UnitTypes::Protoss_Zealot || GetType == UnitTypes::Protoss_Photon_Cannon));
		};

		std::function<double> lambdaUtility = []() -> double {
			return 1;
		};
		
		Option enemyClosest = Option(lambdaFilter, lambdaUtility);
		break;
	}
	case Races::Enum::Zerg:
		break;
	case Races::Enum::Terran:
		break;
	otherwise:
		Broodwar << "Err: don't know enemy race but in his base??" << std::endl;
	}

	return command;
}

UtilityManager::UtilityManager()
{
}

Option::Option(std::function<BWAPI::Unit(BWAPI::Unit)> filterFunction, std::function<double> utilityFunction)
    : filterFunction(filterFunction)
    , utilityFunction(utilityFunction)
{
}
