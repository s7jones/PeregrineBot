#pragma once
#include "Common.h"

class BaseManager {
	BaseManager();

public:
	static BaseManager& Instance();
};