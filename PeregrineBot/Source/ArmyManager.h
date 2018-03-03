#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"
#include <deque>
#include <set>

class Squad;

class ArmyManager {
private:
	ArmyManager() = default;

public:
	static ArmyManager& Instance()
	{
		static ArmyManager instance;
		return instance;
	}
	void update();
	void ZerglingAttack(BWAPI::Unit u);
	std::set<EnemyUnitInfo> GetZerglingAccessibleBuildings(BWAPI::Unit u);
	void ZerglingAttackKnownBuildings(BWAPI::Unit u);
	void ZerglingScoutingBeforeBaseFound(BWAPI::Unit u);
	void ZerglingScoutSpreadOut(BWAPI::Unit u);

private:
	void putUnassignedInSquads();
	void attackWithSquad(Squad& squad);
	void handleIdleUnits();
	void incrementScoutLocationZerglingIndex();

	std::set<Squad> squads;
	const int SQUAD_RADIUS = 128;

	std::deque<BWAPI::Position> scoutLocationsZergling;
	std::deque<BWAPI::Position>::iterator scoutLocationIndex;
};

class SquadCommand {
public:
	enum SquadCommandTypes { NONE,
		                     ATTACK_MOVE,
		                     ATTACK_UNIT,
		                     MOVE };

	SquadCommand(SquadCommandTypes type, BWAPI::Unit target)
	    : type(type)
	    , targetUnit(target)
	{
	}
	SquadCommand(SquadCommandTypes type, BWAPI::Position target)
	    : type(type)
	    , targetPosition(target)
	{
	}
	SquadCommand(SquadCommandTypes type, EnemyUnitInfo enemy)
	    : type(type)
	    , targetEnemy(enemy)
	{
	}

	SquadCommandTypes type         = SquadCommand::NONE;
	BWAPI::Unit targetUnit         = nullptr;
	BWAPI::Position targetPosition = BWAPI::Positions::None;
	EnemyUnitInfo targetEnemy      = { nullptr };
};

class Squad : public BWAPI::Unitset {
public:
	Squad() = default;

	enum SquadIdleTypes { ALLIDLE,
		                  SOMEIDLE,
		                  NONEIDLE };
	SquadIdleTypes isIdle() const
	{
		bool allIdle = true, someIdle = false;

		for (auto unit : *this) {
			if (!unit->isIdle()) {
				allIdle = false;
				break;
			} else {
				if (!someIdle) {
					someIdle = true;
				}
			}
		}

		if (allIdle) {
			return ALLIDLE;
		} else {
			if (someIdle) {
				return SOMEIDLE;
			} else {
				return NONEIDLE;
			}
		}
	}

	enum SquadMovingTypes { ALLMOVING,
		                    SOMEMOVING,
		                    NONEMOVING };
	SquadMovingTypes isMoving() const
	{
		bool allMoving = true, someMoving = false;

		for (auto unit : *this) {
			if (!unit->isIdle()) {
				allMoving = false;
				break;
			} else {
				if (!someMoving) {
					someMoving = true;
				}
			}
		}

		if (allMoving) {
			return ALLMOVING;
		} else {
			if (someMoving) {
				return SOMEMOVING;
			} else {
				return NONEMOVING;
			}
		}
	}

	SquadCommand getLastCommand() const
	{
		return lastCommand;
	}

	void setLastCommand(SquadCommand command)
	{
		lastCommand = command;
	}

private:
	SquadCommand lastCommand;
};
