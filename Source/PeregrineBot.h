#pragma once
#include <BWAPI.h>
#include <string>
#include <iostream>
#include <boost/range/adaptor/reversed.hpp> // for reverse for each loop
#include <fstream>
#include <BWTA.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
//#include <sqlite3.h>

// Remember not to use "Broodwar" in any global class constructor!

class PeregrineBot : public BWAPI::AIModule
{
public:
  // Virtual functions for callbacks, leave these as they are.
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit unit);
  virtual void onUnitEvade(BWAPI::Unit unit);
  virtual void onUnitShow(BWAPI::Unit unit);
  virtual void onUnitHide(BWAPI::Unit unit);
  virtual void onUnitCreate(BWAPI::Unit unit);
  virtual void onUnitDestroy(BWAPI::Unit unit);
  virtual void onUnitMorph(BWAPI::Unit unit);
  virtual void onUnitRenegade(BWAPI::Unit unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit unit);
  // Everything below this line is safe to modify.

  void drawTerrainData();
  void drawExtendedInterface();
  //void scout(BWAPI::Unit* u);
  //void scoutOverlord(BWAPI::Unit* u);
  void move(BWAPI::Unit* u, const BWAPI::Position &pos);
  void setMoveTo(BWAPI::Unit* u, const BWAPI::Position &pos);

};
