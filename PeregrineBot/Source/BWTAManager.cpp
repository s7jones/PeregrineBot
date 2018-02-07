#include "BWTAManager.h"

using namespace BWAPI;
using namespace Filter;

BWTAManager& BWTAManager::Instance()
{
	static BWTAManager instance;
	return instance;
}

void BWTAManager::analyze()
{
	if (analysis) {
		DebugMessenger::Instance() << "Begin analyzing map." << std::endl;

		BWTA::readMap();
		BWTA::analyze();
		analyzed = true;
		analysis_just_finished = true;
	}
}

BWTAManager::BWTAManager()
{
}