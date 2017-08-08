#include "BaseManager.h"

using namespace BWAPI;

BaseManager::BaseManager()
{
}

BaseManager& BaseManager::Instance()
{
	static BaseManager instance;
	return instance;
}