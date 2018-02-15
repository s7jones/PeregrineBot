#pragma once
#include "Common.h"

class Base;

class BaseManager {
private:
	BaseManager() {}

public:
	static BaseManager& BaseManager::Instance()
	{
		static BaseManager instance;
		return instance;
	}
	void ManageBases(BWAPI::Unit u);
	void DoAllWorkerTasks(BWAPI::Unit u);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);

	const Base* main = nullptr;
	std::set<Base> hatcheries;
	std::set<BWAPI::Unit> workers;
	std::set<BWAPI::Unit> miners;
	std::set<BWAPI::Unit> defenders;

	using invaderAndDefender = std::pair<BWAPI::Unit, BWAPI::Unit>;
	std::set<invaderAndDefender> targetsAndAssignedDefenders;

private:
	std::set<BWAPI::Unit> workersTraining;
};

class Base {
public:
	Base(BWAPI::Unit u);
	BWAPI::Unitset checkForInvaders() const;
	void calculateBorder() const;
	bool operator<(const Base& other) const
	{
		return base < other.base;
	}
	friend bool operator<(const Base& lhs, BWAPI::Unit rhs)
	{
		return lhs.base < rhs;
	}
	friend bool operator<(BWAPI::Unit lhs, const Base& rhs)
	{
		return lhs < rhs.base;
	}
	using is_transparent = void;

	mutable double borderRadius = 256;
	BWAPI::Unit base;

private:
};
