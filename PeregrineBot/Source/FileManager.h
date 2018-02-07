#pragma once
#include "Common.h"

class FileManager {
public:
	static FileManager& Instance();

	bool readJsonConfig();
	void writeStatisticsToFile(std::string botVersion, bool isWinner);

private:
	FileManager();
};
