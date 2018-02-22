#include "DebugMessenger.h"

#include "FileManager.h"

void DebugMessenger::setup(bool debug_flag_define)
{
	bool debug_flag_json = FileManager::Instance().readJsonConfig();

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
