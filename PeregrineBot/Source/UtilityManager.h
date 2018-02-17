#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"

using UtilResult = std::pair<double, EnemyUnitInfo>;

class Option;

class UtilityManager {
private:
	UtilityManager() {}

public:
	static UtilityManager& UtilityManager::Instance()
	{
		static UtilityManager instance;
		return instance;
	}
	bool getBestActionForZergling(BWAPI::Unit zergling);

private:
	void constructOptions();
	bool performBestActionForZerglingInEnemyBase(BWAPI::Unit zergling);

	std::vector<Option> options;

	const struct utilities {
		const struct putilities {
			double injrZeal        = 5;
			double injrWork        = 4;
			double closestZealCann = 1.1;
			double artosisPyln     = 1;
			double closestPyln     = 0.9;
			double closestWork     = 0.8;
			double closestAll      = 0.7;

			double addArtosisGtwy = 1;
			double addArtosisCann = 0.5;
			double addArtosisAll  = 0.01;
		} p;
		const struct tutilities {
			double closestMrneFireBunkWork = 1;
			double closestWork             = 0.9;
			double closestSdpt             = 0.8;
			double enemyBase               = 0.7;
			double closestAll              = 0.6;
		} t;
		const struct zutilities {
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

	Option::Option(funcUtil util, std::string description)
	    : util(util)
	    , description(description)
	{
	}

	funcUtil util;
	std::string description;

private:
};
