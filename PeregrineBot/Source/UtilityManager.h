#pragma once
#include "Common.h"

using UtilResult = std::pair<double, BWAPI::Unit>;

class Option;

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
			double injrZeal        = 2;
			double closestZealCann = 1;
			double closestPyln     = 0.9;
			double closestWork     = 0.8;
			double closestAll      = 0.7;
		} p;
		struct tutilities {
			double bust                    = 3;
			double closestMrneFireBunkWork = 1;
			double closestWork             = 0.9;
			double closestSdpt             = 0.8;
			double enemyBase               = 0.7;
			double closestAll              = 0.6;
		} t;
		struct zutilities {
			double injrZerg             = 2;
			double closestLingSunkWork  = 1;
			double closestWork          = 0.9;
			double closestAllNotLarvEgg = 0.8;
			double closestAll           = 0.7;
		} z;
	};

	utilities scores;
};

class Option {
public:
	using funcUtil = std::function<UtilResult(BWAPI::Unit)>;
	Option(funcUtil util, std::string description);

	funcUtil util;
	std::string description;

private:
};
