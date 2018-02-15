#pragma once
#include "Common.h"

struct MessageAndFrames {
	std::string format;
	int frames;
};

class GUIManager {
private:
	GUIManager() {}

public:
	static GUIManager& GUIManager::Instance()
	{
		static GUIManager instance;
		return instance;
	}
	void drawTextOnScreen(BWAPI::Unit u, std::string format, int frames = 480);
	void draw();

private:
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
	std::map<BWAPI::Unit, MessageAndFrames> messageBuffer;
};
