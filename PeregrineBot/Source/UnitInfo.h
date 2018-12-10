#pragma once
#include "BWAPI.h"

class UnitInfo
{
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
		return m_unit < other.m_unit;
	}
	friend bool operator<(const UnitInfo& lhs, BWAPI::Unit rhs)
	{
		return lhs.m_unit < rhs;
	}
	friend bool operator<(BWAPI::Unit lhs, const UnitInfo& rhs)
	{
		return lhs < rhs.m_unit;
	}
	// transparent comparators
	// https://www.youtube.com/watch?v=BBUacofxOP8
	using is_transparent = void;

	BWAPI::Unit m_unit = nullptr;

private:
	mutable int lastFrameSeen    = 0;
	mutable BWAPI::Position pos  = { 0, 0 };
	mutable BWAPI::UnitType type = BWAPI::UnitTypes::Unknown;
};

class FriendlyUnitInfo : public UnitInfo
{
public:
	FriendlyUnitInfo(BWAPI::Unit unitToWrap);
	void update() const override;

private:
	mutable int lastFrameAttacking = 0;
};

class EnemyUnitInfo : public UnitInfo
{
public:
	EnemyUnitInfo(BWAPI::Unit unitToWrap);
	void update() const override;

private:
	mutable int shields = 0;
	mutable int hp      = 0;
	mutable int energy  = 0;
	mutable std::pair<double, double> velocity;
};

class ResourceUnitInfo : public UnitInfo
{
public:
	ResourceUnitInfo(BWAPI::Unit unitToWrap);
	void update() const override;
	int getResources() const;

private:
	mutable int resources;
};
