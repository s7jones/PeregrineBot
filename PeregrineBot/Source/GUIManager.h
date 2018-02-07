#pragma once
#include "Common.h"

class GUIManager {
public:
	static GUIManager& Instance();
	void draw();

private:
	GUIManager();
	void drawAndTalk();
	void drawAdditionalInformation();
	void drawTerrainData();
	void drawExtendedInterface();

	double duration = 0;
	std::chrono::steady_clock::time_point start;
	int frameCount = 0;
};
