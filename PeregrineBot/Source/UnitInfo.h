#pragma once
#include "Common.h"

class UnitInfo {
public:
	UnitInfo(BWAPI::Unit unitToWrap);
	void update() const;
	bool exists() const;
	BWAPI::Position getPosition() const { return pos; }
	int x() { return getPosition().x; }
	int y() { return getPosition().y; }

	bool operator<(const UnitInfo& other) const
	{
		return u < other.u;
	}
	friend bool operator<(const UnitInfo& lhs, BWAPI::Unit rhs)
	{
		return lhs.u < rhs;
	}
	friend bool operator<(BWAPI::Unit lhs, const UnitInfo& rhs)
	{
		return lhs < rhs.u;
	}
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
