#pragma once
#include "Common.h"
#include "InformationManager.h"
#include "OrderManager.h"

class ArmyManager {
	ArmyManager();

public:
	static ArmyManager& Instance();
	void ZerglingAttack(BWAPI::Unit u);
	void ZerglingScout(BWAPI::Unit u);
};