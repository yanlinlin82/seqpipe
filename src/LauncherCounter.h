#ifndef LAUNCHER_COUNTER_H__
#define LAUNCHER_COUNTER_H__

#include <mutex>

class LauncherCounter
{
public:
	unsigned int FetchId()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return ++counter_;
	}
private:
	std::mutex mutex_;
	unsigned int counter_ = 0;
};

#endif
