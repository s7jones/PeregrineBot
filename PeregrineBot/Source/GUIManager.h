#pragma once
#include "Common.h"

struct UnitAndMessage
{
	BWAPI::Unit u;
	std::string format;
	bool operator< (const UnitAndMessage& um) const {
		return u < um.u;
	}
};

class GUIManager {
public:
	static GUIManager& Instance();
	void drawTextOnScreen(BWAPI::Unit u, std::string format, int frames);
	void draw();

private:
	GUIManager();
	void drawTextOnUnit(BWAPI::Unit u, std::string format);
	void drawOnScreenMessages();
	void drawTopLeftOverlay();
	void calculateAverageFrameTime();
	void talk();
	void drawTerrainData();
	void drawExtendedInterface();

	double duration = 0;
	std::chrono::steady_clock::time_point start;
	int frameCount = 0;
	std::map<UnitAndMessage, int> messageBuffer;
};
