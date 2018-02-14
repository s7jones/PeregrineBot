#pragma once
#include "Common.h"

double DistanceAir(const BWAPI::Position p1, const BWAPI::Position p2);
double DistanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
BWAPI::Position GetPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut);
BWAPI::Position GetBasePos(const BWAPI::TilePosition tp);
double DistanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double DistanceGround(const BWAPI::Position start, const BWAPI::Position end);
double TimeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double TimeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
bool isReachable(BWTA::Region* region1, BWTA::Region* region2);

void errorMessage(std::string message);
