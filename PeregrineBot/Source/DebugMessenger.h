#pragma once
#include <BWAPI.h>
#include <BWAPI/Game.h>
#include <boost/filesystem.hpp>

class DebugMessenger {
private:
	DebugMessenger();
	bool debug_enabled;
	std::ostringstream ss;

public:
	static DebugMessenger& Instance();
	~DebugMessenger();
	void Setup(bool debug_flag_define);

	template <class T>
	inline DebugMessenger& operator<<(const T& in)
	{
		// Pass whatever into the stream
		ss << in;
		return *this;
	};

	DebugMessenger& operator<<(BWAPI::GameWrapper::ostream_manipulator fn);
	void flush(BWAPI::GameWrapper::ostream_manipulator fn);
};
