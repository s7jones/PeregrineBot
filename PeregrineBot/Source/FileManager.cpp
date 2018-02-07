#include "FileManager.h"

using namespace BWAPI;
using namespace Filter;

FileManager& FileManager::Instance()
{
	static FileManager instance;
	return instance;
}

void FileManager::writeStatisticsToFile(std::string botVersion, bool isWinner)
{
	struct Scores {
		std::string name;
		int matches;
		int score;
		int percent;
	};

	Scores score[3];

	boost::filesystem::path readDir("./bwapi-data/read");
	boost::filesystem::path writeDir("./bwapi-data/write");
	boost::filesystem::path pathIn, pathOut;

	std::stringstream ss;
	ss << "data_" << botVersion << ".txt";
	std::string filename = ss.str();

	pathIn /= readDir /= filename;
	pathOut /= writeDir /= filename;

	boost::filesystem::ifstream input(pathIn, std::ios::in);

	if (input.fail()) {
		score[0].name = "z";
		score[1].name = "t";
		score[2].name = "p";
		for (int i = 0; i < 3; i++) {
			score[i].matches = 0;
			score[i].score = 0;
			score[i].percent = 0;
		}
	}
	else if (input.is_open()) {
		for (int i = 0; i < 3; i++) {
			input >> score[i].name >> score[i].matches >> score[i].score >> score[i].percent;
		}
	}

	input.close();

	if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].matches++;
	if (Broodwar->enemy()->getRace() == Races::Terran) score[1].matches++;
	if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].matches++;

	if (isWinner) {
		if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].score++;
		if (Broodwar->enemy()->getRace() == Races::Terran) score[1].score++;
		if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].score++;
	}

	if (Broodwar->enemy()->getRace() == Races::Zerg) score[0].percent = (int)(100 * score[0].score / score[0].matches);
	if (Broodwar->enemy()->getRace() == Races::Terran) score[1].percent = (int)(100 * score[1].score / score[1].matches);
	if (Broodwar->enemy()->getRace() == Races::Protoss) score[2].percent = (int)(100 * score[2].score / score[2].matches);

	boost::filesystem::ofstream output(pathOut, std::ios::trunc);

	for (int i = 0; i < 3; i++) {
		output << score[i].name << "\t" << score[i].matches << "\t" << score[i].score << "\t" << score[i].percent << "\n";
	}

	output.close();
}

FileManager::FileManager()
{
}
