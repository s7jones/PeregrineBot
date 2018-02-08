#pragma once
#include "Common.h"

class UtilityManager {
public:
	static UtilityManager& Instance();
	BWAPI::UnitCommand getBestActionForZergling(BWAPI::Unit zergling);

private:
	UtilityManager();
	BWAPI::UnitCommand getBestActionForZerglingInEnemyBase(BWAPI::Unit zergling);

};

class Option {
public:
	Option();
	Option(std::function<BWAPI::Unit(BWAPI::Unit)> filterFunction, std::function<double()> utilityFunction);

	std::function<BWAPI::Unit(BWAPI::Unit)> filterFunction;
	std::function<double()> utilityFunction;

private:
	//BWAPI::UnitFilter unitfilter;
};