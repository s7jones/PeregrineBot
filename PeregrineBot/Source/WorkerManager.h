#pragma once
#include "Common.h"

class WorkerManager {
public:
	static WorkerManager& Instance();
	void DoAllWorkerTasks(BWAPI::Unit u);

private:
	WorkerManager();
};
