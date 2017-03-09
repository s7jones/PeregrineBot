#pragma once
#include <BWAPI.h>
#include <BWAPI/Game.h>

class DebugMessenger {
public:
	DebugMessenger(bool debug);
	~DebugMessenger();

	template <class T>
	inline DebugMessenger& operator<<(const T& in)
	{
		// Pass whatever into the stream
		ss << in;
		return *this;
	};

	DebugMessenger& operator<<(BWAPI::GameWrapper::ostream_manipulator fn);

	void DebugMessenger::flush();

private:
	bool my_debug;
	std::ostringstream ss;
};