#pragma once
#include "Common.h"

double distanceAir(const BWAPI::Position p1, const BWAPI::Position p2);
double distanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
BWAPI::Position getPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut);
BWAPI::Position getBasePos(const BWAPI::TilePosition tp);
double distanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double distanceGround(const BWAPI::Position start, const BWAPI::Position end);
double timeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double timeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
bool isReachable(BWTA::Region* region1, BWTA::Region* region2);

void errorMessage(std::string message);
