#pragma once
#include "Common.h"

class Base;

class BaseManager {
public:
	static BaseManager& Instance();
	void ManageBases(const BWAPI::Unit& u);
	void onUnitShow(const BWAPI::Unit& unit);
	void onUnitCreate(const BWAPI::Unit& unit);
	void onUnitDestroy(const BWAPI::Unit& unit);
	void onUnitMorph(const BWAPI::Unit& unit);
	void onUnitRenegade(const BWAPI::Unit& unit);

	std::set<Base> hatcheries;
	std::set<BWAPI::Unit> workers;

private:
	BaseManager();

	std::set<BWAPI::Unit> workersTraining;
};

class Base {
public:
	Base(const BWAPI::Unit& u);
	BWAPI::Unitset checkForInvaders() const;
	void calculateBorder() const;
	bool operator<(const Base& other) const;

	mutable double borderRadius = 0;

private:
	const BWAPI::Unit& base;
};
