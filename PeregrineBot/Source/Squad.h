#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"

enum SquadCommandTypes {
	NONE,
	ATTACK_MOVE,
	ATTACK_UNIT,
	MOVE
};

class SquadCommand {
public:
	SquadCommand() = default;

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

	SquadCommandTypes type         = SquadCommandTypes::NONE;
	BWAPI::Unit targetUnit         = nullptr;
	BWAPI::Position targetPosition = BWAPI::Positions::None;
	EnemyUnitInfo targetEnemy      = { nullptr };
};

class Squad : public BWAPI::Unitset {
public:
	Squad()
	{
		id = numberSquadsMade++;
	}

	unsigned int id;
	static unsigned int numberSquadsMade;

	enum SquadIdleTypes {
		ALLIDLE,
		SOMEIDLE,
		NONEIDLE
	};
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

	enum SquadMovingTypes {
		ALLMOVING,
		SOMEMOVING,
		NONEMOVING
	};
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
