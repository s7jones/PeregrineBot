#pragma once
#include <BWAPI.h>

float DistanceAir(const BWAPI::Position p1, const BWAPI::Position p2);
float DistanceAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
BWAPI::Position GetPos(const BWAPI::TilePosition tp, const BWAPI::UnitType ut);
BWAPI::Position GetBasePos(const BWAPI::TilePosition tp);
double DistanceGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double TimeGround(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
double TimeAir(const BWAPI::TilePosition start, const BWAPI::TilePosition end);
