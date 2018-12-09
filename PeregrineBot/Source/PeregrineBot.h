#pragma once
#include "BWAPI.h"

// Remember not to use "Broodwar" in any global class constructor!

class PeregrineBot : public BWAPI::AIModule
{
public:
	// Virtual functions for callbacks, leave these as they are.
	void onStart() override;
	void onEnd(bool isWinner) override;
	void onFrame() override;
	void onSendText(std::string text) override;
	void onReceiveText(BWAPI::Player player, std::string text) override;
	void onPlayerLeft(BWAPI::Player player) override;
	void onNukeDetect(BWAPI::Position target) override;
	void onUnitDiscover(BWAPI::Unit unit) override;
	void onUnitEvade(BWAPI::Unit unit) override;
	void onUnitShow(BWAPI::Unit unit) override;
	void onUnitHide(BWAPI::Unit unit) override;
	void onUnitCreate(BWAPI::Unit unit) override;
	void onUnitDestroy(BWAPI::Unit unit) override;
	void onUnitMorph(BWAPI::Unit unit) override;
	void onUnitRenegade(BWAPI::Unit unit) override;
	void onSaveGame(std::string gameName) override;
	void onUnitComplete(BWAPI::Unit unit) override;
	// Everything below this line is safe to modify.

	const std::string version = "v4";
};
