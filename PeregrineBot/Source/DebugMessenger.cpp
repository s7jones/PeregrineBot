#include "DebugMessenger.h"

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

void DebugMessenger::SetDebugMode(bool debug)
{
	DebugMessenger::my_debug = debug;
}

DebugMessenger& DebugMessenger::operator<<(BWAPI::GameWrapper::ostream_manipulator fn)
{
	// Pass manipulator into the stream
	ss << fn;

	// Flush to Broodwar's printf if we see endl or ends
	if (fn == &std::endl<char, std::char_traits<char>> || fn == &std::ends<char, std::char_traits<char>>) {
		this->flush();
	}
	return *this;
}

void DebugMessenger::flush()
{
	//if (!BroodwarPtr) return;
	if (ss.str().empty()) return;

	if (my_debug)
		BWAPI::Broodwar << ss.str().c_str();

	//BroodwarPtr->printf("%s", ss.str().c_str());
	ss.str("");
}