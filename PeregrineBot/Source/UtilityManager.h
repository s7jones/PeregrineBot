#pragma once
#include "ArmyManager.h"
#include "BWAPI.h"
#include "UnitInfo.h"

using UtilResult = std::pair<double, EnemyUnitInfo>;

class OptionIndividual;
class OptionSquad;

class UtilityManager {
private:
	UtilityManager() = default;

public:
	static UtilityManager& Instance()
	{
		static UtilityManager instance;
		return instance;
	}
	bool getBestActionForZergling(BWAPI::Unit zergling);
	bool getBestActionForSquad(Squad& squad);

private:
	void constructOptionsIndividual();
	void constructOptionsSquad();
	bool performBestActionForZerglingInEnemyBase(BWAPI::Unit zergling);
	bool performBestActionForSquadInEnemyBase(Squad& squad);

	std::vector<OptionIndividual> options_individual;
	std::vector<OptionSquad> options_squad;

	struct utilities {
		struct putilities {
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
		} const p;
		struct tutilities {
			double closestMrneFireBunkWork = 1;
			double closestWork             = 0.9;
			double closestSdpt             = 0.8;
			double enemyBase               = 0.7;
			double closestAll              = 0.6;
		} const t;
		struct zutilities {
			double injrZerg             = 2;
			double closestLingSunkWork  = 1;
			double closestWork          = 0.9;
			double closestAllNotLarvEgg = 0.8;
			double closestAll           = 0.7;
		} const z;
	} const scores{};
};

class OptionIndividual {
public:
	using funcUtil = std::function<UtilResult(BWAPI::Unit)>;

	OptionIndividual(const funcUtil& util, const std::string& description)
	    : util(util)
	    , description(description)
	{
	}

	funcUtil util;
	std::string description;

private:
};

class OptionSquad {
public:
	using funcUtil = std::function<UtilResult(const Squad&)>;

	OptionSquad(const funcUtil& util, const std::string& description)
	    : util(util)
	    , description(description)
	{
	}

	funcUtil util;
	std::string description;

private:
};
