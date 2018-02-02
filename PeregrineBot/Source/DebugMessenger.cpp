#include "DebugMessenger.h"
//#include <cpptoml.h>
#include <rapidjson\document.h>
#include <rapidjson\filereadstream.h>
#include <rapidjson\rapidjson.h>
#include <rapidjson\reader.h>

using namespace rapidjson;

DebugMessenger::DebugMessenger()
{
}

DebugMessenger& DebugMessenger::Instance()
{
	static DebugMessenger instance;
	return instance;
}

DebugMessenger::~DebugMessenger()
{
}

void DebugMessenger::Setup(bool debug_flag_define)
{
	/*
	// TOML snippet for later?
	boost::filesystem::path config_file_path("./bwapi-data/read/config.toml");

	bool debug_flag_toml = false;

	try {
		auto config = cpptoml::parse_file(config_file_path.string());

		debug_flag_toml = config->get_as<bool>("debug").value_or(false); // val is bool value in toml or false
	}
	catch (const cpptoml::parse_exception& e) {
		BWAPI::Broodwar << "Config file not read" << std::endl;
	}

	if (debug_flag_toml || debug_flag_define) {
		debug_enabled = true;
	}
	*/

	boost::filesystem::path config_file_path("./bwapi-data/read/PeregrineConfig.json");

	//FILE* fp = fopen_s(config_file_path.string(), "rb"); // rapidjson recommended fopen, but compiler recommends "safe" variant
	FILE* fp;
	const wchar_t* mode = L"rb";
	errno_t err = _wfopen_s(&fp, config_file_path.c_str(), mode);

	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	Document d;
	d.ParseStream(is);

	fclose(fp);

	bool debug_flag_json = false;

	if (d.HasMember("debug")) {
		if (d["debug"].IsBool()) {
			debug_flag_json = d["debug"].GetBool();
		}
	}

	if (debug_flag_json || debug_flag_define) {
		debug_enabled = true;
	}
}

DebugMessenger& DebugMessenger::operator<<(BWAPI::GameWrapper::ostream_manipulator fn)
{
	// DO NOT pass manipulator naively into the stream like GameWrapper does.

	// Flush to Broodwar's printf if we see endl or ends
	if (fn == &std::endl<char, std::char_traits<char>> || fn == &std::ends<char, std::char_traits<char>>) {
		// Pass endl or ends manipulator to be applied by operator<< onto Broodwar.
		this->flush(fn);
	} else {
		// Pass other ostream_manipulators into the stream.
		ss << fn;
	}
	return *this;
}

void DebugMessenger::flush(BWAPI::GameWrapper::ostream_manipulator fn)
{
	if (ss.str().empty()) return;

	if (debug_enabled) {
		BWAPI::Broodwar << ss.str().c_str() << fn;
	}
	ss.str("");
}
