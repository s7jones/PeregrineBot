#pragma once
#include "BWAPI.h"
#include "BWTA.h"

class BWTAManager {
private:
	BWTAManager() = default;

public:
	static BWTAManager& Instance()
	{
		static BWTAManager instance;
		return instance;
	}
	void analyze();

	bool analyzed               = false;
	bool analysis_just_finished = false;

private:
	bool analyzing      = false;
	const bool analysis = true;
};
