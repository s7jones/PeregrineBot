#pragma once
#include "Common.h"

class FileManager {
public:
	static FileManager& Instance();

	void writeStatisticsToFile(std::string botVersion, bool isWinner);

private:
	FileManager();
};
