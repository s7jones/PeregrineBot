#pragma once
#include "Common.h"

class UnitInfo {
public:
	UnitInfo(BWAPI::Unit unitToWrap);
	void update() const;
	bool exists() const;
	BWAPI::Position getPosition() const;
	int x()
	{
		return getPosition().x;
	}
	int y()
	{
		return getPosition().y;
	}

	bool operator<(const UnitInfo& other) const;
	friend bool operator<(const UnitInfo& lhs, BWAPI::Unit rhs);
	friend bool operator<(BWAPI::Unit lhs, const UnitInfo& rhs);
	// transparent comparators
	// https://www.youtube.com/watch?v=BBUacofxOP8
	using is_transparent = void;

	BWAPI::Unit u = NULL;

private:
	mutable int lastFrameSeen;
	mutable BWAPI::Position pos  = { 0, 0 };
	mutable BWAPI::UnitType type = BWAPI::UnitTypes::Unknown;
	mutable int shields;
	mutable int hp;
	mutable int energy;
	mutable std::pair<double, double> velocity;
};
