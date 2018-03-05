#pragma once
#include "BWAPI.h"
#include "UnitInfo.h"
#include <chrono>

class GUIManager {
private:
	GUIManager() = default;

public:
	static GUIManager& Instance()
	{
		static GUIManager instance;
		return instance;
	}
	void drawTextOnScreen(BWAPI::Unit unit, const std::string& format, int frames = 480);
	void drawLineOnScreen(BWAPI::Unit unit, EnemyUnitInfo enemy, int frames = 480);
	void draw();

private:
	void drawTextOnUnit(BWAPI::Unit unit, const std::string& format);
	void drawOnScreenLines();
	void drawOnScreenMessages();
	void drawTopLeftOverlay();
	void calculateAverageFrameTime();
	void talk();
	void drawTerrainData();
	void drawExtendedInterface();

	double duration = 0;
	std::chrono::steady_clock::time_point start;
	int frameCount = 0;

	struct MessageAndFrames {
		std::string format;
		int frames;
	};

	struct TargetAndFrames {
		EnemyUnitInfo target;
		int frames;
	};

	std::map<BWAPI::Unit, MessageAndFrames> messageBuffer;
	std::map<BWAPI::Unit, TargetAndFrames> lineBuffer;
};
