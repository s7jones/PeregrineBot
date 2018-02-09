#pragma once
#include "Common.h"

class Option {
public:
	using funcUtil = std::function<std::pair<double, BWAPI::Unit>(BWAPI::Unit)>;
	Option(funcUtil util, std::string description);

	funcUtil util;
	std::string description;

private:
};

class UtilityManager {
public:
	static UtilityManager& Instance();
	bool getBestActionForZergling(BWAPI::Unit zergling);

private:
	UtilityManager();
	void constructOptions();
	bool performBestActionForZerglingInEnemyBase(BWAPI::Unit zergling);

	std::vector<Option> options;
};
