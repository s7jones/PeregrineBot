#pragma once
#include "Common.h"

class FileManager {
private:
	FileManager() {}

public:
	static FileManager& FileManager::Instance()
	{
		static FileManager instance;
		return instance;
	}

	bool readJsonConfig();
	void writeStatisticsToFile(std::string botVersion, bool isWinner);

private:
};
