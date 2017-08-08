#include "WorkerManager.h"

using namespace BWAPI;

WorkerManager::WorkerManager()
{
}

WorkerManager& WorkerManager::Instance()
{
	static WorkerManager instance;
	return instance;
}