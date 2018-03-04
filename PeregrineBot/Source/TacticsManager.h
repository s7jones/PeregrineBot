#pragma once
#include "BWAPI.h"
#include "Squad.h"

class TacticsManager {
private:
	TacticsManager() = default;

public:
	static TacticsManager& Instance()
	{
		static TacticsManager instance;
		return instance;
	}

	void recommendReinforcement(SquadID squad, unsigned int count)
	{
		reinforcementMap.insert({ squad, count });
	}

private:
	std::map<SquadID, unsigned int> reinforcementMap;
};
