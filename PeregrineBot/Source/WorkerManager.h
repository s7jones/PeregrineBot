#pragma once
#include "Common.h"

class WorkerManager {
public:
	static WorkerManager& Instance();
	void DoAllWorkerTasks(const BWAPI::Unit u);

private:
	WorkerManager();

	int lastChecked     = 0;
	int poolLastChecked = 0;
};
