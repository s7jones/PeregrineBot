#pragma once
#include "Common.h"

class UnitInfo {
public:
	UnitInfo(BWAPI::Unit unitToWrap);
	void Update();
	bool Exists();
private:
	int lastFrameSeen;
	BWAPI::Unit u = NULL;
	BWAPI::Position pos = { 0,0 };
	BWAPI::UnitType type = BWAPI::UnitTypes::Unknown;
	int shields;
	int hp;
	int energy;
	std::pair<double, double> velocity;
};
