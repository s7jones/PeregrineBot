#pragma once
#include <BWAPI.h>
#include <BWAPI/Game.h>

class DebugMessenger {
private:
	DebugMessenger();
	bool debug_enabled;
	std::ostringstream ss;

public:
	static DebugMessenger& Instance();
	~DebugMessenger();
	void SetDebugMode(bool debug);

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
