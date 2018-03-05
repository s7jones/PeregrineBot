#pragma once
#include "BWAPI.h"

class UnitInfo {
public:
	UnitInfo(BWAPI::Unit unitToWrap);
	virtual void update() const;
	bool exists() const;
	BWAPI::Position getPosition() const { return pos; }
	int x() { return getPosition().x; }
	int y() { return getPosition().y; }
	BWAPI::UnitType getType() const { return type; }

	virtual bool operator<(const UnitInfo& other) const
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

	BWAPI::Unit u = nullptr;

private:
	mutable int lastFrameSeen    = 0;
	mutable BWAPI::Position pos  = { 0, 0 };
	mutable BWAPI::UnitType type = BWAPI::UnitTypes::Unknown;
};

class FriendlyUnitInfo : public UnitInfo {
public:
	FriendlyUnitInfo(BWAPI::Unit unitToWrap)
	    : UnitInfo(unitToWrap)
	{
		FriendlyUnitInfo::update();
	}
	void update() const override;

private:
	mutable int lastFrameAttacking = 0;
};

class EnemyUnitInfo : public UnitInfo {
public:
	EnemyUnitInfo(BWAPI::Unit unitToWrap)
	    : UnitInfo(unitToWrap)
	{
		EnemyUnitInfo::update();
	}
	void update() const override;

private:
	mutable int shields = 0;
	mutable int hp      = 0;
	mutable int energy  = 0;
	mutable std::pair<double, double> velocity;
};

class ResourceUnitInfo : public UnitInfo {
public:
	ResourceUnitInfo(BWAPI::Unit unitToWrap)
	    : UnitInfo(unitToWrap)
	{
		ResourceUnitInfo::update();
	}
	void update() const override;
	int getResources() const;

private:
	mutable int resources = 0;
};
