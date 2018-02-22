#pragma once
#include "BWAPI.h"

class FileManager {
private:
	FileManager() = default;

public:
	static FileManager& Instance()
	{
		static FileManager instance;
		return instance;
	}

	bool readJsonConfig();
	void writeStatisticsToFile(std::string botVersion, bool isWinner);

private:
};
