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

	struct utilities {
		struct putilities {
			double closestZealCann = 1;
			double closestPyln     = 0.9;
			double closestWork     = 0.8;
			double closestAll      = 0.7;
		} p;
		struct tutilities {
			double closestMrneFireBunkWork = 1;
			double closestWork             = 0.9;
			double closestSdpt             = 0.8;
			double closestAll              = 0.7;
		} t;
		struct zutilities {
			double closestLingSunkWork  = 1;
			double closestWork          = 0.9;
			double closestAllNotLarvEgg = 0.8;
			double closestAll           = 0.7;
		} z;
	};

	utilities scores;
};
