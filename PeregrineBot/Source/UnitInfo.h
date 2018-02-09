#pragma once
#include "Common.h"

class UnitInfo {
public:
	UnitInfo(BWAPI::Unit unitToWrap);
	void update();
	bool exists();
	BWAPI::Position getPosition();
	bool operator<(const UnitInfo& other) const;
	BWAPI::Unit u = NULL;

private:
	int lastFrameSeen;
	BWAPI::Position pos  = { 0, 0 };
	BWAPI::UnitType type = BWAPI::UnitTypes::Unknown;
	int shields;
	int hp;
	int energy;
	std::pair<double, double> velocity;
};
