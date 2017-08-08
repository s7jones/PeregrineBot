#pragma once
#include "Common.h"

class WorkerManager {
	WorkerManager();

public:
	static WorkerManager& Instance();
};