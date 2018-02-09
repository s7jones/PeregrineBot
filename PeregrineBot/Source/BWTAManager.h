#pragma once
#include "Common.h"

class BWTAManager {
public:
	static BWTAManager& Instance();
	void analyze();

	bool analyzed               = false;
	bool analysis_just_finished = false;

private:
	BWTAManager();
	bool analyzing      = false;
	const bool analysis = true;
};
#pragma once
