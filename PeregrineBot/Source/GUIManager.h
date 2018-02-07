#pragma once
#include "Common.h"

class GUIManager {
public:
	static GUIManager& Instance();
	void draw();

private:
	GUIManager();
	void drawAdditionalInformation();
	void drawTerrainData();
	void drawExtendedInterface();
};
