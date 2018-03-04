#include "FileManager.h"

#include "DebugMessenger.h"
#include "InformationManager.h"
#include <cpptoml.h>
#include <rapidjson\document.h>
#include <rapidjson\filereadstream.h>
#include <rapidjson\rapidjson.h>
#include <rapidjson\reader.h>

using namespace BWAPI;
using namespace Filter;
using namespace rapidjson;

bool FileManager::readJsonConfig()
{
	// TOML snippet for later?
	boost::filesystem::path toml_config_path("./bwapi-data/read/PeregrineConfig.toml");

	bool debug_flag_toml = false;

	try {
		auto config = cpptoml::parse_file(toml_config_path.string());

		debug_flag_toml = config->get_as<bool>("debug_msgs_enabled").value_or(false); // val is bool value in toml or false
	} catch (const cpptoml::parse_exception& e) {
		BWAPI::Broodwar << "Config file not read, " << e.what() << std::endl;
	}

	bool debug_flag_json = false;

	boost::filesystem::path json_config_path("./bwapi-data/read/PeregrineConfig.json");

	//FILE* fp = fopen_s(config_file_path.string(), "rb"); // rapidjson recommended fopen, but compiler recommends "safe" variant
	FILE* fp;
	const wchar_t* mode = L"rb";
	errno_t err         = _wfopen_s(&fp, json_config_path.c_str(), mode);

	if (!err) {

		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		Document d;
		d.ParseStream(is);

		fclose(fp);

		if (d.HasMember("debug_msgs_enabled")) {
			if (d["debug_msgs_enabled"].IsBool()) {
				debug_flag_json = d["debug_msgs_enabled"].GetBool();
			}
		}
	}

	return debug_flag_json || debug_flag_toml;
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
			score[i].score   = 0;
			score[i].percent = 0;
		}
	} else if (input.is_open()) {
		for (int i = 0; i < 3; i++) {
			input >> score[i].name >> score[i].matches >> score[i].score >> score[i].percent;
		}
	}

	input.close();

	const auto enemyRace = InformationManager::Instance().enemyRace;

	if (enemyRace == Races::Zerg) score[0].matches++;
	if (enemyRace == Races::Terran) score[1].matches++;
	if (enemyRace == Races::Protoss) score[2].matches++;

	if (isWinner) {
		if (enemyRace == Races::Zerg) score[0].score++;
		if (enemyRace == Races::Terran) score[1].score++;
		if (enemyRace == Races::Protoss) score[2].score++;
	}

	if (enemyRace == Races::Zerg) score[0].percent = (int)(100 * score[0].score / score[0].matches);
	if (enemyRace == Races::Terran) score[1].percent = (int)(100 * score[1].score / score[1].matches);
	if (enemyRace == Races::Protoss) score[2].percent = (int)(100 * score[2].score / score[2].matches);

	boost::filesystem::ofstream output(pathOut, std::ios::trunc);

	for (int i = 0; i < 3; i++) {
		output << score[i].name << "\t" << score[i].matches << "\t" << score[i].score << "\t" << score[i].percent << "\n";
	}

	output.close();
}
