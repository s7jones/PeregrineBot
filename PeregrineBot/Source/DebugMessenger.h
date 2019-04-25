#pragma once
#include "BWAPI.h"
#include "BWAPI/Game.h"
#include <boost/filesystem.hpp>

class DebugMessenger
{
	//https://stackoverflow.com/a/1008289/5791272
private:
	DebugMessenger() = default;

public:
	static DebugMessenger& Instance()
	{
		static DebugMessenger instance;
		return instance;
	}
	void setup(bool debug_flag_define);

	template <class T>
	inline DebugMessenger& operator<<(const T& in)
	{
		// Pass whatever into the stream
		ss << in;
		return *this;
	};

	DebugMessenger& operator<<(BWAPI::GameWrapper::ostream_manipulator fn);
	void flush(BWAPI::GameWrapper::ostream_manipulator fn);

private:
	bool debug_enabled = false;
	std::ostringstream ss;
};
